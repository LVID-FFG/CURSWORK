#!/bin/bash
echo "КУЛИНАРНАЯ КНИГА"
echo "================="

# Проверка Docker
if ! docker ps &> /dev/null; then
    echo "Ошибка: Docker не запущен!"
    exit 1
fi

# Очистка старых контейнеров
docker stop cookbook-app 2>/dev/null || true
docker rm cookbook-app 2>/dev/null || true

# Разрешаем доступ к X11
xhost +local:docker > /dev/null 2>&1

echo "Запуск контейнера..."
echo ""

# Запуск контейнера
docker run -it --rm \
    --name cookbook-app \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
    -v cookbook-db-data:/var/lib/postgresql/14/main \
    cookbook-app