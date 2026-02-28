#include "docker_commands.h"
#include <array>
#include <cstdio>
#include <sstream>
#include <algorithm>
#include <unistd.h>

bool DockerCommands::IsValidDockerIdentifier(const std::string& str) {
    if (str.empty() || str.size() > 256) return false;
    return std::all_of(str.begin(), str.end(), [](char c) {
        return std::isalnum(c) || c == '_' || c == '-' || c == '.' || c == ':' || c == '/';
    });
}

std::string DockerCommands::FindDockerBinary() {
    static std::string cached;
    if (!cached.empty()) return cached;

    const char* candidates[] = {
        "/usr/bin/docker",
        "/usr/local/bin/docker",
        "/snap/bin/docker",
        "/opt/homebrew/bin/docker",
        nullptr
    };
    for (int i = 0; candidates[i]; ++i) {
        if (access(candidates[i], X_OK) == 0) {
            cached = candidates[i];
            return cached;
        }
    }
    cached = "docker";
    return cached;
}

bool DockerCommands::IsDockerAvailable() {
    CommandResult res = ExecuteCommand(
        FindDockerBinary() + " info 2>/dev/null");
    return res.exit_code == 0;
}

std::string DockerCommands::GetDockerError() {
    CommandResult res = ExecuteCommand(
        FindDockerBinary() + " info 2>&1");
    if (res.exit_code == 0) return "";
    return res.output;
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
        FindDockerBinary() + " ps --format '{{.ID}}|{{.Names}}|{{.State}}|{{.Status}}|{{.Image}}' 2>/dev/null");

    if (res.exit_code != 0) return containers;

    std::istringstream stream(res.output);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        ContainerInfo info;
        std::istringstream lineStream(line);

        std::getline(lineStream, info.id, '|');
        std::getline(lineStream, info.name, '|');
        std::getline(lineStream, info.state, '|');
        std::getline(lineStream, info.status, '|');
        std::getline(lineStream, info.image, '|');

        containers.push_back(info);
    }

    return containers;
}

std::vector<ContainerInfo> DockerCommands::GetStoppedContainers() {
    std::vector<ContainerInfo> containers;
    CommandResult res = ExecuteCommand(
        FindDockerBinary() + " ps -a --filter 'status=exited' --filter 'status=created' "
        "--filter 'status=dead' "
        "--format '{{.ID}}|{{.Names}}|{{.State}}|{{.Status}}|{{.Image}}' 2>/dev/null");

    if (res.exit_code != 0) return containers;

    std::istringstream stream(res.output);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        ContainerInfo info;
        std::istringstream lineStream(line);

        std::getline(lineStream, info.id, '|');
        std::getline(lineStream, info.name, '|');
        std::getline(lineStream, info.state, '|');
        std::getline(lineStream, info.status, '|');
        std::getline(lineStream, info.image, '|');

        containers.push_back(info);
    }

    return containers;
}

std::vector<ContainerInfo> DockerCommands::GetAllContainers() {
    std::vector<ContainerInfo> containers;
    CommandResult res = ExecuteCommand(
        FindDockerBinary() + " ps -a --format '{{.ID}}|{{.Names}}|{{.State}}|{{.Status}}|{{.Image}}' 2>/dev/null");

    if (res.exit_code != 0) return containers;

    std::istringstream stream(res.output);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        ContainerInfo info;
        std::istringstream lineStream(line);

        std::getline(lineStream, info.id, '|');
        std::getline(lineStream, info.name, '|');
        std::getline(lineStream, info.state, '|');
        std::getline(lineStream, info.status, '|');
        std::getline(lineStream, info.image, '|');

        containers.push_back(info);
    }

    return containers;
}

std::vector<ImageInfo> DockerCommands::GetUnusedImages() {
    std::vector<ImageInfo> images;
    CommandResult res = ExecuteCommand(
        FindDockerBinary() + " images -f 'dangling=true' "
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

std::vector<ImageInfo> DockerCommands::GetAllImages() {
    std::vector<ImageInfo> images;
    CommandResult res = ExecuteCommand(
        FindDockerBinary() + " images -a "
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
        FindDockerBinary() + " volume ls -f 'dangling=true' "
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

std::vector<VolumeInfo> DockerCommands::GetAllVolumes() {
    std::vector<VolumeInfo> volumes;
    CommandResult res = ExecuteCommand(
        FindDockerBinary() + " volume ls "
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
        FindDockerBinary() + " stats --no-stream --format '{{.CPUPerc}}|{{.MemUsage}}' 2>/dev/null");

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
    CommandResult res = ExecuteCommand(FindDockerBinary() + " stop " + id + " 2>/dev/null");
    return res.exit_code == 0;
}

bool DockerCommands::StopAllContainers() {
    CommandResult ids = ExecuteCommand(FindDockerBinary() + " ps -q 2>/dev/null");
    if (ids.exit_code != 0 || ids.output.empty()) return false;

    std::string idList = ids.output;
    for (char& c : idList) {
        if (c == '\n') c = ' ';
    }
    while (!idList.empty() && idList.back() == ' ') {
        idList.pop_back();
    }
    if (idList.empty()) return false;

    CommandResult res = ExecuteCommand(FindDockerBinary() + " stop " + idList + " 2>/dev/null");
    return res.exit_code == 0;
}

bool DockerCommands::RemoveContainer(const std::string& id) {
    if (!IsValidDockerIdentifier(id)) return false;
    CommandResult res = ExecuteCommand(FindDockerBinary() + " rm " + id + " 2>/dev/null");
    return res.exit_code == 0;
}

bool DockerCommands::RemoveImage(const std::string& id) {
    if (!IsValidDockerIdentifier(id)) return false;
    CommandResult res = ExecuteCommand(FindDockerBinary() + " rmi " + id + " 2>/dev/null");
    return res.exit_code == 0;
}

bool DockerCommands::RemoveVolume(const std::string& name) {
    if (!IsValidDockerIdentifier(name)) return false;
    CommandResult res = ExecuteCommand(FindDockerBinary() + " volume rm " + name + " 2>/dev/null");
    return res.exit_code == 0;
}

bool DockerCommands::PruneAll() {
    CommandResult res = ExecuteCommand(FindDockerBinary() + " system prune -af --volumes 2>/dev/null");
    return res.exit_code == 0;
}
