FROM ubuntu:22.04

ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8
ENV DEBIAN_FRONTEND=noninteractive

# Устанавливаем что нужно под root
RUN apt-get update && apt-get install -y \
    postgresql-14 postgresql-client-14 postgresql-contrib-14 libpq-dev \
    cmake make g++ pkg-config \
    qt6-base-dev qt6-base-dev-tools \
    libx11-6 libxcb1 libgl1-mesa-dev \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Настраиваем PostgreSQL
RUN mkdir -p /var/run/postgresql && \
    chown -R postgres:postgres /var/run/postgresql && \
    chmod 2777 /var/run/postgresql

# Инициализируем базу данных PostgreSQL (она создастся в /var/lib/postgresql/14/main)
RUN service postgresql start && \
    sleep 3 && \
    sudo -u postgres psql -c "CREATE USER cookbookuser WITH PASSWORD 'cookbook123';" && \
    sudo -u postgres psql -c "CREATE DATABASE cookbook OWNER cookbookuser;" && \
    sudo -u postgres psql -c "ALTER USER cookbookuser WITH PASSWORD 'cookbook123';" && \
    service postgresql stop

WORKDIR /app
COPY . /app/

# Собираем приложение под root
RUN mkdir -p /app/build && \
    cd /app/build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DPOSTGRESQL_INCLUDE_DIR=/usr/include/postgresql \
        -DPOSTGRESQL_LIBRARY=/usr/lib/x86_64-linux-gnu/libpq.so \
    && make -j$(nproc)

# Создаем пользователя приложения после сборки
RUN useradd -m -u 1000 appuser && \
    echo "appuser ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers && \
    chown -R appuser:appuser /app

# Скрипт запуска
COPY docker-start.sh /docker-start.sh
RUN chmod +x /docker-start.sh && \
    sed -i 's/\r$//' /docker-start.sh  # Удаляем возможные CR

# Переключаемся на appuser
USER appuser

CMD ["/docker-start.sh"]