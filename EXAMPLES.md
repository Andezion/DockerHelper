# Как собрать

## Для меня

**Простой способ:**
```bash
./create_portable_package.sh
```

**Что будет:**
1. Создается оптимизированная сборка в `build_release/`
2. Упаковывается портативный пакет в `dist/`
3. Готовый архив: `dist/docker-manager-1.0.0-linux-x86_64.tar.gz`

### Проверка

```bash
./test_package.sh
```

Распакует пакет во временную директорию и покажет содержимое.

### Информация о сборках

```bash
./info.sh
```

Показывает:
- Доступные команды
- Статус сборок
- Системную информацию
- Список документации

### Подготовка к GitHub Release

```bash
./prepare_release.sh 1.0.0
```

Автоматически:
- Проверит Git репозиторий
- Закоммитит изменения (если нужно)
- Создаст портативный пакет
- Покажет инструкции для публикации

### Установка в систему

```bash
sudo ./install_system_wide.sh
```

После этого можно запускать `docker-manager` из любого места.

---

## Для пользователей 

### Скачивание и запуск

```bash
# 1. Распаковать
tar -xzf docker-manager-1.0.0-linux-x86_64.tar.gz

# 2. Зайти
cd docker-manager-1.0.0-linux-x86_64

# 3. Запустить
./docker-manager.sh
```

### Если нет прав на Docker

```bash
# Добавить себя в группу docker
sudo usermod -aG docker $USER

# Применить изменения (перелогиниться или)
newgrp docker

# Проверить
docker ps
```

### Если не хватает библиотек

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install libwxgtk3.0-gtk3-0v5
```

**Fedora:**
```bash
sudo dnf install wxGTK3
```

**Arch Linux:**
```bash
sudo pacman -S wxgtk3
```

**OpenSUSE:**
```bash
sudo zypper install wxWidgets-3_2-devel
```

---


