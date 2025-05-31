cmake_minimum_required(VERSION 3.10)
project(TetrisTowersPhysics)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включаем vcpkg (если переменная VCPKG_ROOT установлена)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

# Ищем header-only httplib.h (cpp-httplib)
find_path(HTTPLIB_INCLUDE_DIR
  NAMES httplib.h
  PATHS /usr/local/include/httplib /usr/local/include /usr/include
)
if (NOT HTTPLIB_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find httplib.h (cpp-httplib). Make sure `/usr/local/include/httplib/httplib.h` exists.")
endif()
include_directories(${HTTPLIB_INCLUDE_DIR})

# Находим nlohmann_json
find_package(nlohmann_json CONFIG REQUIRED)

# Опции сборки
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTS "Build tests" ON)

# Экспорт базы команд
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Флаги компиляции
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Исходники и заголовки для physics_engine
set(SOURCES
    PhysicsEngine.cpp
    FFIInterface.cpp
)
set(HEADERS
    include/PhysicsEngine.h
    include/FFIInterface.h
)

# Библиотека physics_engine
add_library(physics_engine ${SOURCES} ${HEADERS})
target_include_directories(physics_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Тестовый исполняемый
if(BUILD_TESTS)
    add_executable(physics_engine_test main.cpp)
    target_link_libraries(physics_engine_test PRIVATE physics_engine)
endif()

# HTTP-серверная часть
add_executable(physics_server server.cpp)
target_link_libraries(physics_server PRIVATE physics_engine)
target_link_libraries(physics_server PRIVATE nlohmann_json::nlohmann_json)

# Установка (install)
install(TARGETS physics_engine
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)
install(FILES ${HEADERS} DESTINATION include)

# Экспорт целей
export(TARGETS physics_engine FILE physics_engine-config.cmake)
