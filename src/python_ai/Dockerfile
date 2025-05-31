# syntax=docker/dockerfile:1.4
FROM python:3.11-slim

# Install system dependencies and build tools
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        gcc \
        g++ \
        libsnappy-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app/ai
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
COPY . .
CMD ["python", "main.py"]
