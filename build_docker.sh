#!/bin/bash
echo "=== Сборка Docker образа ==="

# Останавливаем старые контейнеры
docker stop cookbook-app 2>/dev/null || true
docker rm cookbook-app 2>/dev/null || true

# Удаляем старый образ
docker rmi cookbook-app 2>/dev/null || true

# Получаем UID и GID текущего пользователя
MY_UID=$(id -u)
MY_GID=$(id -g)

echo "Сборка образа с UID=$MY_UID, GID=$MY_GID..."
echo "Используется базовый образ postgres:14"
echo "Это может занять несколько минут..."

docker build --no-cache \
    --build-arg USER_ID=$MY_UID \
    --build-arg GROUP_ID=$MY_GID \
    -t cookbook-app .

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Образ успешно собран!"
    echo "✅ Используется PostgreSQL 14 из официального образа"
    echo "✅ Данные БД сохраняются в Docker volume: cookbook-db-data"
    echo ""
    echo "Для запуска приложения выполните:"
    echo "  ./run.sh"
    echo ""
    echo "Для просмотра логов базы данных:"
    echo "  docker volume inspect cookbook-db-data"
else
    echo "❌ Ошибка сборки!"
    exit 1
fi