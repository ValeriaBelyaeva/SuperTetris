# Use Python 3.11 slim image as base
FROM python:3.11-slim

# Системные зависимости (при необходимости, добавить дополнительные)
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential \
      gcc \
      g++ \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
ENV PYTHONUNBUFFERED=1
ENV PYTHONPATH=/app

# Копируем только requirements.txt для кэширующих слоёв
COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

# Копируем остальной исходный код
COPY . .

# Expose port
EXPOSE 8000

# Run the FastAPI application
CMD ["uvicorn", "src.main:app", "--host", "0.0.0.0", "--port", "8000"] 