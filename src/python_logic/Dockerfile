# --------- stage 0: build physics ----------
FROM ubuntu:20.04 AS builder

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

ARG HTTP_LIB_VER=v0.15.4            # при желании зафиксируйте версию
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential g++ cmake make curl ca-certificates && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /build

# ① кладём cpp-physics в контекст (уже работает)
COPY cpp-physics ./cpp-physics

# ② тащим header-only cpp-httplib туда, где его найдёт CMake
RUN mkdir -p /usr/local/include/httplib && \
    curl -L https://raw.githubusercontent.com/yhirose/cpp-httplib/${HTTP_LIB_VER}/httplib.h \
         -o /usr/local/include/httplib/httplib.h

# ③ собираем .so
WORKDIR /build/cpp-physics
RUN cmake -DCMAKE_BUILD_TYPE=Release . && \
    make -j$(nproc)

# --------- stage 1: runtime ----------
FROM python:3.11-slim

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /app
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# ④ берём собранную библиотеку
COPY --from=builder /build/cpp-physics/libphysics_engine.so ./libphysics.so

# ⑤ копируем исходники
COPY src ./src

CMD ["python", "-m", "uvicorn", "src.main:app", "--host", "0.0.0.0", "--port", "8000"]
 