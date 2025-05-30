FROM python:3.11-slim

# Установить системные библиотеки и компилятор
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        gcc \
        g++ \
        libsnappy-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN pip install --no-cache-dir -r requirements.txt
CMD ["python", "-m", "python_analytics.main"] # Assuming main.py or similar entry point 