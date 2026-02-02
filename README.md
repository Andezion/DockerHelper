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

### Для разработчиков (сборка из исходников):
- CMake 3.10+
- C++11 compiler
- wxWidgets 3.0+ с dev пакетами
- Docker

## Возможности

- Мониторинг контейнеров в реальном времени
- Остановка контейнеров (один или все)
- Удаление контейнеров, образов и томов
- Очистка неиспользуемых ресурсов
- Отображение системной информации Docker
- Автоматическое обновление каждые 5 секунд

## Структура проекта

```
Docker/
├── src/                      # Исходный код
│   ├── docker_manager.cpp    # Главный файл с GUI
│   ├── docker_manager.h
│   ├── docker_commands.cpp   # Docker команды
│   └── docker_commands.h
├── scripts/
│   └── docker_info.sh        # Вспомогательный скрипт
├── build_static.sh           # Сборка оптимизированного бинарника
├── create_portable_package.sh # Создание портативного пакета
├── install_system_wide.sh    # Установка в систему
├── CMakeLists.txt
└── README.md
```

## Права доступа к Docker

Пользователь должен иметь права на использование Docker:

```bash
# Добавить пользователя в группу docker
sudo usermod -aG docker $USER

# Перелогиниться или выполнить
newgrp docker
```

## Разработка

### Пересборка

```bash
cd build
make
```

### Очистка

```bash
rm -rf build
```
