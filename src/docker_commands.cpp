#include "docker_commands.h"
#include <array>
#include <cstdio>
#include <sstream>
#include <algorithm>

bool DockerCommands::IsValidDockerIdentifier(const std::string& str) {
    if (str.empty() || str.size() > 256) return false;
    return std::all_of(str.begin(), str.end(), [](char c) {
        return std::isalnum(c) || c == '_' || c == '-' || c == '.' || c == ':' || c == '/';
    });
}

CommandResult DockerCommands::ExecuteCommand(const std::string& command) {
    CommandResult result;
    result.exit_code = -1;
    std::array<char, 256> buffer;

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return result;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result.output += buffer.data();
    }

    int status = pclose(pipe);
    result.exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    return result;
}

std::vector<ContainerInfo> DockerCommands::GetRunningContainers() {
    std::vector<ContainerInfo> containers;
    CommandResult res = ExecuteCommand(
        "docker ps --format '{{.ID}}|{{.Names}}|{{.Status}}|{{.Image}}' 2>/dev/null");

    if (res.exit_code != 0) return containers;

    std::istringstream stream(res.output);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        ContainerInfo info;
        std::istringstream lineStream(line);

        std::getline(lineStream, info.id, '|');
        std::getline(lineStream, info.name, '|');
        std::getline(lineStream, info.status, '|');
        std::getline(lineStream, info.image, '|');

        containers.push_back(info);
    }

    return containers;
}

std::vector<ContainerInfo> DockerCommands::GetStoppedContainers() {
    std::vector<ContainerInfo> containers;
    CommandResult res = ExecuteCommand(
        "docker ps -a --filter 'status=exited' --filter 'status=created' "
        "--filter 'status=dead' "
        "--format '{{.ID}}|{{.Names}}|{{.Status}}|{{.Image}}' 2>/dev/null");

    if (res.exit_code != 0) return containers;

    std::istringstream stream(res.output);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        ContainerInfo info;
        std::istringstream lineStream(line);

        std::getline(lineStream, info.id, '|');
        std::getline(lineStream, info.name, '|');
        std::getline(lineStream, info.status, '|');
        std::getline(lineStream, info.image, '|');

        containers.push_back(info);
    }

    return containers;
}

std::vector<ImageInfo> DockerCommands::GetUnusedImages() {
    std::vector<ImageInfo> images;
    CommandResult res = ExecuteCommand(
        "docker images -f 'dangling=true' "
        "--format '{{.ID}}|{{.Repository}}|{{.Tag}}|{{.Size}}' 2>/dev/null");

    if (res.exit_code != 0) return images;

    std::istringstream stream(res.output);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        ImageInfo info;
        std::istringstream lineStream(line);

        std::getline(lineStream, info.id, '|');
        std::getline(lineStream, info.repository, '|');
        std::getline(lineStream, info.tag, '|');
        std::getline(lineStream, info.size, '|');

        images.push_back(info);
    }

    return images;
}

std::vector<VolumeInfo> DockerCommands::GetUnusedVolumes() {
    std::vector<VolumeInfo> volumes;
    CommandResult res = ExecuteCommand(
        "docker volume ls -f 'dangling=true' "
        "--format '{{.Name}}|{{.Driver}}' 2>/dev/null");

    if (res.exit_code != 0) return volumes;

    std::istringstream stream(res.output);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        VolumeInfo info;
        std::istringstream lineStream(line);

        std::getline(lineStream, info.name, '|');
        std::getline(lineStream, info.driver, '|');

        volumes.push_back(info);
    }

    return volumes;
}

SystemInfo DockerCommands::GetSystemInfo() {
    SystemInfo info;
    info.cpu_usage = 0.0;
    info.mem_usage = "0 MiB";
    info.container_count = 0;

    CommandResult res = ExecuteCommand(
        "docker stats --no-stream --format '{{.CPUPerc}}|{{.MemUsage}}' 2>/dev/null");

    if (res.exit_code != 0) return info;

    std::istringstream stream(res.output);
    std::string line;
    double totalCpu = 0.0;
    double totalMemMiB = 0.0;
    int count = 0;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        std::istringstream lineStream(line);
        std::string cpu, mem;

        std::getline(lineStream, cpu, '|');
        std::getline(lineStream, mem, '|');

        if (!cpu.empty() && cpu.back() == '%') {
            cpu.pop_back();
            try {
                totalCpu += std::stod(cpu);
            } catch (...) {}
        }

        if (!mem.empty()) {
            try {
                double val = std::stod(mem);
                if (mem.find("GiB") != std::string::npos) {
                    totalMemMiB += val * 1024.0;
                } else {
                    totalMemMiB += val;
                }
            } catch (...) {}
        }

        count++;
    }

    info.cpu_usage = totalCpu;
    info.container_count = count;

    if (totalMemMiB >= 1024.0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.2f GiB", totalMemMiB / 1024.0);
        info.mem_usage = buf;
    } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.1f MiB", totalMemMiB);
        info.mem_usage = buf;
    }

    return info;
}

bool DockerCommands::StopContainer(const std::string& id) {
    if (!IsValidDockerIdentifier(id)) return false;
    CommandResult res = ExecuteCommand("docker stop " + id + " 2>/dev/null");
    return res.exit_code == 0;
}

bool DockerCommands::StopAllContainers() {
    CommandResult ids = ExecuteCommand("docker ps -q 2>/dev/null");
    if (ids.exit_code != 0 || ids.output.empty()) return false;

    std::string trimmed = ids.output;
    while (!trimmed.empty() && (trimmed.back() == '\n' || trimmed.back() == ' ')) {
        trimmed.pop_back();
    }
    if (trimmed.empty()) return false;

    CommandResult res = ExecuteCommand("docker stop " + trimmed + " 2>/dev/null");
    return res.exit_code == 0;
}

bool DockerCommands::RemoveContainer(const std::string& id) {
    if (!IsValidDockerIdentifier(id)) return false;
    CommandResult res = ExecuteCommand("docker rm " + id + " 2>/dev/null");
    return res.exit_code == 0;
}

bool DockerCommands::RemoveImage(const std::string& id) {
    if (!IsValidDockerIdentifier(id)) return false;
    CommandResult res = ExecuteCommand("docker rmi " + id + " 2>/dev/null");
    return res.exit_code == 0;
}

bool DockerCommands::RemoveVolume(const std::string& name) {
    if (!IsValidDockerIdentifier(name)) return false;
    CommandResult res = ExecuteCommand("docker volume rm " + name + " 2>/dev/null");
    return res.exit_code == 0;
}

bool DockerCommands::PruneAll() {
    CommandResult res = ExecuteCommand("docker system prune -af --volumes 2>/dev/null");
    return res.exit_code == 0;
}
