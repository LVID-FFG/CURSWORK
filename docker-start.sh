#!/bin/bash
set -e

echo "=== ЗАПУСК КУЛИНАРНОЙ КНИГИ ==="

# Просто запускаем PostgreSQL
echo "Запуск PostgreSQL..."
sudo service postgresql start

# Ждем пока PostgreSQL полностью запустится
sleep 5

# ВСЕГДА проверяем и создаем базу данных cookbook и пользователя
echo "Проверка базы данных cookbook..."
if ! sudo -u postgres psql -tAc "SELECT 1 FROM pg_database WHERE datname='cookbook'" | grep -q 1; then
    echo "Создание пользователя и базы данных..."
    sudo -u postgres psql -c "CREATE USER cookbookuser WITH PASSWORD 'cookbook123';"
    sudo -u postgres psql -c "CREATE DATABASE cookbook OWNER cookbookuser;"
    sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE cookbook TO cookbookuser;"
    echo "✓ База данных создана"
else
    echo "✓ База данных cookbook уже существует"
fi

# ВСЕГДА проверяем пользователя
echo "Проверка пользователя cookbookuser..."
if ! sudo -u postgres psql -tAc "SELECT 1 FROM pg_roles WHERE rolname='cookbookuser'" | grep -q 1; then
    echo "Создание пользователя cookbookuser..."
    sudo -u postgres psql -c "CREATE USER cookbookuser WITH PASSWORD 'cookbook123';"
    sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE cookbook TO cookbookuser;"
    echo "✓ Пользователь создан"
fi

# ВСЕГДА устанавливаем пароль
echo "Установка пароля пользователя..."
sudo -u postgres psql -c "ALTER USER cookbookuser WITH PASSWORD 'cookbook123';" 2>/dev/null || true

# Настройка X11
export QT_QPA_PLATFORM=xcb
export DISPLAY=${DISPLAY:-:0}

echo "=== ЗАПУСК ПРИЛОЖЕНИЯ ==="
cd /app/build
exec ./CookBook