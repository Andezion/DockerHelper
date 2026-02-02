# Docker Manager

Графический менеджер для управления Docker контейнерами, образами и томами.

## Быстрый старт для разработчиков

### Сборка проекта

```bash
mkdir build && cd build
cmake ..
make
```

### Запуск

```bash
./build/docker_manager
```

## Создание портативного пакета для распространения

Если ты хочешь поделиться программой с другими, чтобы они могли **просто скачать и запустить** без компиляции:

### 1. Создай портативный пакет

```bash
chmod +x create_portable_package.sh
./create_portable_package.sh
```

Это создаст архив `dist/docker-manager-X.X.X-linux-x86_64.tar.gz`, который можно распространять.

### 2. Пользователи просто распаковывают и запускают

```bash
tar -xzf docker-manager-1.0.0-linux-x86_64.tar.gz
cd docker-manager-1.0.0-linux-x86_64
./docker-manager.sh
```

**Вот и всё!** Никакой компиляции, никаких сложных зависимостей.

## Установка в систему (опционально)

Если хочешь установить в систему для всех пользователей:

```bash
chmod +x install_system_wide.sh
sudo ./install_system_wide.sh
```

После этого можно запускать командой `docker-manager` из любого места.

## Требования

### Для конечных пользователей (запуск готового пакета):
- Docker установлен и запущен
- Linux x86_64 (64-bit)
- wxWidgets 3.0+ (обычно уже есть в системе)

Если wxWidgets отсутствует:
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


