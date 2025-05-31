FROM gcc:13 as builder

WORKDIR /build

# Install build dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    curl \
    git \
    && rm -rf /var/lib/apt/lists/*

# Download and install httplib.h
RUN mkdir -p /usr/local/include/httplib && \
    curl -L https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h \
    -o /usr/local/include/httplib/httplib.h

# Install nlohmann_json
RUN git clone https://github.com/nlohmann/json.git && \
    cd json && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make install

# Copy source code
COPY cpp-physics ./cpp-physics

# Build
RUN mkdir cpp-physics/build && \
    cd cpp-physics/build && \
    cmake .. && \
    make

# Create final image
FROM gcc:13

WORKDIR /app

# Copy built library
COPY --from=builder /build/cpp-physics/build/libphysics_engine.so /app/

# Set environment variables
ENV LD_LIBRARY_PATH=/app

# Run the server
CMD ["./physics_server"] 