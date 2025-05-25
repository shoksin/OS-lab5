cmake_minimum_required(VERSION 3.14)
project(NamedPipesIPC LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем папку со исходниками
add_subdirectory(src)

# Тесты
enable_testing()
find_package(GTest REQUIRED)
add_subdirectory(tests)