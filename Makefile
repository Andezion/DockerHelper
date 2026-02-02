CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
WX_CONFIG = wx-config

WX_CXXFLAGS = $(shell $(WX_CONFIG) --cxxflags)
WX_LIBS = $(shell $(WX_CONFIG) --libs)

SRC_DIR = src
BUILD_DIR = build
SCRIPT_DIR = scripts

SOURCES = $(SRC_DIR)/docker_manager.cpp
OBJECTS = $(BUILD_DIR)/docker_manager.o

TARGET = docker_manager

all: $(BUILD_DIR) $(TARGET) make_executable

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(SRC_DIR)/docker_manager.h
	$(CXX) $(CXXFLAGS) $(WX_CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(WX_LIBS)

make_executable:
	chmod +x $(SCRIPT_DIR)/docker_info.sh

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

run: all
	./$(TARGET)

install-deps:
	@echo "Установка зависимостей для Ubuntu/Debian..."
	sudo apt-get update
	sudo apt-get install -y build-essential libwxgtk3.0-gtk3-dev docker.io bc
	@echo "Зависимости установлены!"

install-deps-fedora:
	@echo "Установка зависимостей для Fedora..."
	sudo dnf install -y gcc-c++ wxGTK-devel docker bc
	@echo "Зависимости установлены!"

install-deps-arch:
	@echo "Установка зависимостей для Arch Linux..."
	sudo pacman -S --needed base-devel wxwidgets-gtk3 docker bc
	@echo "Зависимости установлены!"

.PHONY: all clean run make_executable install-deps install-deps-fedora install-deps-arch
