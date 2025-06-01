# --------- stage 0: build physics ----------
FROM ubuntu:20.04 AS builder

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

ARG HTTP_LIB_VER=v0.15.4            # при желании зафиксируйте версию
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential g++ cmake make curl ca-certificates \
        nlohmann-json3-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /build

# ① кладём cpp-physics в контекст (уже работает)
COPY src/python_logic/cpp-physics ./cpp-physics

# ② тащим header-only cpp-httplib туда, где его найдёт CMake
RUN mkdir -p /usr/local/include/httplib && \
    curl -L -o /usr/local/include/httplib/httplib.h \
         https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h && \
    if [ ! -s /usr/local/include/httplib/httplib.h ]; then \
        echo "Failed to download httplib.h" && exit 1; \
    fi

# ③ собираем .so
WORKDIR /build/cpp-physics
RUN cmake -DCMAKE_BUILD_TYPE=Release . && \
    make -j$(nproc)

# --------- stage 1: runtime ----------
FROM python:3.11-slim

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /app
COPY src/python_logic/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# ④ берём собранную библиотеку
COPY --from=builder /build/cpp-physics/libphysics_engine.so ./libphysics.so

# ⑤ копируем исходники и конфигурацию
COPY src/python_logic/src ./src
COPY src/python_logic/config.py ./config.py

ENV PYTHONPATH=/app

CMD ["uvicorn", "src.main:app", "--host", "0.0.0.0", "--port", "8000"]
 