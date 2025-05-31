# Use Python 3.11 slim image as base
FROM python:3.11-slim

# Установить системные зависимости (в том числе libsnappy-dev)
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential \
      gcc \
      g++ \
      libsnappy-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
ENV PYTHONUNBUFFERED=1
ENV PYTHONPATH=/app/src

# Сначала копируем только requirements.txt, чтобы кэш слоёв работал эффективнее
COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

# Теперь копируем весь исходный код
COPY . .

# Expose port
EXPOSE 8001

# Run the FastAPI application
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8001"] 