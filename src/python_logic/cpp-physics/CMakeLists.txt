cmake_minimum_required(VERSION 3.10)
project(physics_engine)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем путь к заголовочным файлам
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Включаем vcpkg (если переменная VCPKG_ROOT установлена)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

# Ищем header-only httplib.h (cpp-httplib)
find_path(HTTPLIB_INCLUDE_DIR
  NAMES httplib.h
  PATHS 
    /usr/local/include/httplib
    /usr/include/httplib
    /usr/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  NO_DEFAULT_PATH
)
if (NOT HTTPLIB_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find httplib.h (cpp-httplib). Make sure it is installed.")
endif()
include_directories(${HTTPLIB_INCLUDE_DIR})

# Находим nlohmann_json
find_package(nlohmann_json REQUIRED)

# Находим pthreads
find_package(Threads REQUIRED)

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
add_library(physics_engine SHARED ${SOURCES} ${HEADERS})
target_include_directories(physics_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(physics_engine PRIVATE nlohmann_json::nlohmann_json)

# Тестовый исполняемый
if(BUILD_TESTS)
    add_executable(physics_engine_test main.cpp)
    target_link_libraries(physics_engine_test PRIVATE physics_engine)
endif()

# HTTP-серверная часть
add_executable(physics_server server.cpp)
target_link_libraries(physics_server PRIVATE physics_engine)
target_link_libraries(physics_server PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(physics_server PRIVATE Threads::Threads)

# Установка (install)
install(TARGETS physics_engine
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)
install(FILES ${HEADERS} DESTINATION include)

# Экспорт целей
export(TARGETS physics_engine FILE physics_engine-config.cmake)
