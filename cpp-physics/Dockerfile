# Build stage
FROM gcc:13 AS builder

WORKDIR /app

# Install build dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    make \
    g++ \
    git \
    curl \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Install vcpkg
RUN git clone https://github.com/Microsoft/vcpkg.git /vcpkg && \
    /vcpkg/bootstrap-vcpkg.sh && \
    /vcpkg/vcpkg integrate install

# Set vcpkg environment
ENV VCPKG_ROOT=/vcpkg

# Copy source code
COPY . .

# Build project
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Runtime stage
FROM debian:bookworm-slim

WORKDIR /app

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Copy binaries from builder
COPY --from=builder /app/build/physics_server /usr/local/bin/
COPY --from=builder /app/build/libphysics_engine.so /usr/local/lib/

# Register shared library
RUN ldconfig

# Set environment variables
ENV LD_LIBRARY_PATH=/usr/local/lib

# Expose port
EXPOSE 9000

# Health check
HEALTHCHECK --interval=10s --timeout=3s \
    CMD curl -fs http://localhost:9000/health || exit 1

# Run server
CMD ["/usr/local/bin/physics_server"]
