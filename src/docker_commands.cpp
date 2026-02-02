#include "docker_commands.h"
#include <array>
#include <cstdio>
#include <sstream>

std::string DockerCommands::ExecuteCommand(const std::string& command) {
    std::array<char, 256> buffer;
    std::string result;
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    pclose(pipe);
    return result;
}

std::vector<ContainerInfo> DockerCommands::GetRunningContainers() {
    std::vector<ContainerInfo> containers;
    std::string output = ExecuteCommand("docker ps --format '{{.ID}}|{{.Names}}|{{.Status}}|{{.Image}}' 2>/dev/null");
    
    std::istringstream stream(output);
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
    std::string output = ExecuteCommand("docker ps -a --filter 'status=exited' --format '{{.ID}}|{{.Names}}|{{.Status}}|{{.Image}}' 2>/dev/null");
    
    std::istringstream stream(output);
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
    std::string output = ExecuteCommand("docker images -f 'dangling=true' --format '{{.ID}}|{{.Repository}}|{{.Tag}}|{{.Size}}' 2>/dev/null");
    
    std::istringstream stream(output);
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
    std::string output = ExecuteCommand("docker volume ls -f 'dangling=true' --format '{{.Name}}|{{.Driver}}' 2>/dev/null");
    
    std::istringstream stream(output);
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
    
    std::string output = ExecuteCommand("docker stats --no-stream --format '{{.CPUPerc}}|{{.MemUsage}}' 2>/dev/null | head -10");
    
    std::istringstream stream(output);
    std::string line;
    double totalCpu = 0.0;
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
        
        if (count == 0 && !mem.empty()) {
            info.mem_usage = mem;
        }
        
        count++;
    }
    
    info.cpu_usage = totalCpu;
    info.container_count = count;
    
    return info;
}

bool DockerCommands::StopContainer(const std::string& id) {
    std::string command = "docker stop " + id + " 2>/dev/null";
    ExecuteCommand(command);
    return true;
}

bool DockerCommands::StopAllContainers() {
    std::string command = "docker stop $(docker ps -q) 2>/dev/null";
    ExecuteCommand(command);
    return true;
}

bool DockerCommands::RemoveContainer(const std::string& id) {
    std::string command = "docker rm " + id + " 2>/dev/null";
    ExecuteCommand(command);
    return true;
}

bool DockerCommands::RemoveImage(const std::string& id) {
    std::string command = "docker rmi " + id + " 2>/dev/null";
    ExecuteCommand(command);
    return true;
}

bool DockerCommands::RemoveVolume(const std::string& name) {
    std::string command = "docker volume rm " + name + " 2>/dev/null";
    ExecuteCommand(command);
    return true;
}

bool DockerCommands::PruneAll() {
    std::string command = "docker system prune -af --volumes 2>/dev/null";
    ExecuteCommand(command);
    return true;
}
