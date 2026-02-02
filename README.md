# Docker Manager

A graphical manager for managing Docker containers, images, and volumes.

## Quick start for developers

### Building the project

```bash
mkdir build && cd build
cmake ..
make
```

### Running

```bash
./build/docker_manager
```

## Creating a portable package for distribution

If you want to share the program with others so they can **simply download and run** it without compiling:

### 1. Create a portable package

```bash
chmod +x create_portable_package.sh
./create_portable_package.sh
```

This will create an archive `dist/docker-manager-X.X.X-linux-x86_64.tar.gz` that can be distributed.

### 2. Users simply unpack and run

```bash
tar -xzf docker-manager-1.0.0-linux-x86_64.tar.gz
cd docker-manager-1.0.0-linux-x86_64
./docker-manager.sh
```

**That's it!** No compilation, no complex dependencies.

## System installation (optional)

If you want to install it system-wide for all users:

```bash
chmod +x install_system_wide.sh
sudo ./install_system_wide.sh
```

After that, you can run it from anywhere with the `docker-manager` command.

## Requirements

### For end users (running the ready-made package):
- Docker installed and running
- Linux x86_64 (64-bit)
- wxWidgets 3.0+ (usually already installed on the system)

If wxWidgets is not present:
```bash
# Ubuntu/Debian
sudo apt-get install libwxgtk3.0-gtk3-0v5

# Fedora/RHEL
sudo dnf install wxGTK3

# Arch Linux
sudo pacman -S wxgtk3
```


### For developers (building from source):
- CMake 3.10+
- C++11 compiler
- wxWidgets 3.0+ with dev packages
- Docker

## Features

- Real-time container monitoring
- Stop containers (one or all)
- Delete containers, images, and volumes
- Clean up unused resources
- Display Docker system information
- Automatic refresh every 5 seconds

## Project structure

```
Docker/
├── src/                      # Source code
│   ├── docker_manager.cpp    # Main file with GUI
│   ├── docker_manager.h
│   ├── docker_commands.cpp   # Docker commands
│   └── docker_commands.h
├── scripts/
│   └── docker_info.sh        # Auxiliary script
├── build_static.sh           # Build optimized binary
├── create_portable_package.sh # Create portable package
├── install_system_wide.sh    # Install to system
├── CMakeLists.txt
└── README.md
```

## Docker access rights

The user must have rights to use Docker:

```bash
# Add user to the docker group
sudo usermod -aG docker $USER

# Log out and log back in or execute
newgrp docker
```

## Development

### Rebuild

```bash
cd build
make
```

### Cleanup

```bash
rm -rf build
```


