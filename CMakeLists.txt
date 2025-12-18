cmake_minimum_required(VERSION 3.10)
project(CookBook VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Поиск Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Поиск PostgreSQL (обязательно)
find_path(POSTGRESQL_INCLUDE_DIR
    NAMES libpq-fe.h
    PATHS /usr/include/postgresql
    REQUIRED
)

find_library(POSTGRESQL_LIBRARY
    NAMES pq
    PATHS /usr/lib/x86_64-linux-gnu
    REQUIRED
)

message(STATUS "Found PostgreSQL: ${POSTGRESQL_INCLUDE_DIR}")

# Основное приложение
add_executable(CookBook
    src/main.cpp
    src/mainwindow.cpp
    src/recipe.cpp
    src/cookbookdatabase.cpp
    src/recipedialog.cpp
)

target_include_directories(CookBook PRIVATE 
    src
    ${POSTGRESQL_INCLUDE_DIR}
)

target_link_libraries(CookBook
    Qt6::Core
    Qt6::Widgets
    ${POSTGRESQL_LIBRARY}
)