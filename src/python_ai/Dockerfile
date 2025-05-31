# syntax=docker/dockerfile:1

FROM python:3.11-slim

# Install system dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy requirements and install dependencies
COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

# Copy the entire project structure
COPY . .

# Set PYTHONPATH to include the app directory
ENV PYTHONPATH=/app

# Run the FastAPI service with uvicorn
CMD ["uvicorn", "src.api:app", "--host", "0.0.0.0", "--port", "8002"]
