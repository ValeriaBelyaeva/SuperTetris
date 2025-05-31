#
# 1) Этап сборки (builder)
#
FROM gcc:13 AS builder
WORKDIR /app

# Устанавливаем зависимости для сборки (cmake, make, git/curl для vcpkg)
RUN apt-get update && apt-get install -y \
      cmake \
      make \
      g++ \
      git \
      curl \
      pkg-config \
      zip \
    && rm -rf /var/lib/apt/lists/*

# Клонируем и устанавливаем vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git /vcpkg && \
    /vcpkg/bootstrap-vcpkg.sh

# Указываем путь к vcpkg
ENV VCPKG_ROOT=/vcpkg

# Сразу подтягиваем пакеты (чтобы закэшировать)
RUN /vcpkg/vcpkg install cpp-httplib nlohmann-json --triplet x64-linux

# Копируем в контейнер исходники
COPY . .

# Собираем проект
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_TOOLCHAIN_FILE=/vcpkg/scripts/buildsystems/vcpkg.cmake && \
    make -j$(nproc)

#
# 2) Этап исполнения (runtime)
#
FROM debian:bookworm-slim
WORKDIR /app

# Устанавливаем только рантайм-библиотеки
RUN apt-get update && apt-get install -y \
      libstdc++6 \
      curl \
    && rm -rf /var/lib/apt/lists/*

# Копируем скомпилированные бинарники и shared-библиотеку
COPY --from=builder /app/build/physics_server     /usr/local/bin/physics_server
COPY --from=builder /app/build/libphysics_engine.so /usr/local/lib/

# Обновляем кэш shared-библиотек
RUN ldconfig

# Переменная для поиска .so (если потребуется)
ENV LD_LIBRARY_PATH=/usr/local/lib

# Открываем порт сервера
EXPOSE 9000

# Healthcheck — проверка на "/health"
HEALTHCHECK --interval=10s --timeout=3s \
  CMD curl -fs http://localhost:9000/health || exit 1

# Запуск сервера
CMD ["/usr/local/bin/physics_server"]
