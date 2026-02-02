#!/bin/bash

set -e
echo "Сборка Docker Manager для распространения"

rm -rf build_release
mkdir -p build_release
cd build_release

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-O3 -march=x86-64 -mtune=generic"

make -j$(nproc)

echo "Сборка завершена!"
echo "Исполняемый файл: $(pwd)/docker_manager"
echo "Размер файла: $(du -h docker_manager | cut -f1)"

echo "Динамические зависимости:"
ldd docker_manager | grep -v "linux-vdso\|ld-linux" || true
echo ""

strip docker_manager
echo "Размер после strip: $(du -h docker_manager | cut -f1)"
echo ""

cd ..
