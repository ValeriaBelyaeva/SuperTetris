# syntax=docker/dockerfile:1

# Stage 1: Build cpp-physics
FROM gcc:13 as cpp-physics-builder
WORKDIR /build

# Install build dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Download and install httplib.h
RUN mkdir -p /usr/local/include/httplib && \
    curl -L https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h \
    -o /usr/local/include/httplib/httplib.h

# Copy and build cpp-physics
COPY cpp-physics ./cpp-physics
RUN mkdir cpp-physics/build && cd cpp-physics/build && cmake .. && make

# Stage 2: Python service
FROM python:3.11-slim
WORKDIR /app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    libsnappy-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy requirements first to leverage Docker cache
COPY src/python_ai/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy the rest of the application
COPY src/python_ai/ .

# Copy the physics library from the builder stage
COPY --from=cpp-physics-builder /build/cpp-physics/build/libphysics_engine.so /app/

# Set environment variables
ENV PYTHONPATH=/app
ENV LD_LIBRARY_PATH=/app

# Run the application
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8001"]
