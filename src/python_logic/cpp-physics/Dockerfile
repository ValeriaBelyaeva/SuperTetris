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
FROM gcc:13

WORKDIR /app

# Нам достаточно curl только для healthcheck, libstdc++ уже есть в gcc:13
RUN apt-get update && apt-get install -y \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Копируем собранный сервер и библиотеку в /usr/local
COPY --from=builder /app/build/physics_server /usr/local/bin/
COPY --from=builder /app/build/libphysics_engine.so /usr/local/lib/

# Обновляем кэш динамических библиотек (libstdc++.so.6 уже подходящей версии)
RUN ldconfig

# Учтём, что libphysics_engine.so лежит в /usr/local/lib, чтобы программa её нашла
ENV LD_LIBRARY_PATH=/usr/local/lib

# Открываем порт того самого HTTP-сервера
EXPOSE 9000

# Healthcheck: проверяем «/health» каждый 10 секунд, таймаут 3 секунды
HEALTHCHECK --interval=10s --timeout=3s \
    CMD curl -fs http://localhost:9000/health || exit 1

# Запускаем наш physics_server (он уже подрос до /usr/local/bin/)
CMD ["/usr/local/bin/physics_server"]
