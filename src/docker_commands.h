#pragma once

#include <string>
#include <vector>

struct ContainerInfo {
    std::string id;
    std::string name;
    std::string status;
    std::string image;
};

struct ImageInfo {
    std::string id;
    std::string repository;
    std::string tag;
    std::string size;
};

struct VolumeInfo {
    std::string name;
    std::string driver;
};

struct SystemInfo {
    double cpu_usage;
    std::string mem_usage;
    int container_count;
};

struct CommandResult {
    std::string output;
    int exit_code;
};

class DockerCommands {
public:
    static CommandResult ExecuteCommand(const std::string& command);
    static std::vector<ContainerInfo> GetRunningContainers();
    static std::vector<ContainerInfo> GetStoppedContainers();
    static std::vector<ImageInfo> GetUnusedImages();
    static std::vector<VolumeInfo> GetUnusedVolumes();
    static SystemInfo GetSystemInfo();
    static bool StopContainer(const std::string& id);
    static bool StopAllContainers();
    static bool RemoveContainer(const std::string& id);
    static bool RemoveImage(const std::string& id);
    static bool RemoveVolume(const std::string& name);
    static bool PruneAll();
    static bool IsDockerAvailable();
    static std::string GetDockerError();

private:
    static bool IsValidDockerIdentifier(const std::string& str);
    static std::string FindDockerBinary();
};
