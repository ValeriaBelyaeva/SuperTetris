# Используем Python 3.11 slim в качестве базового образа
FROM python:3.11-slim

# Устанавливаем системные зависимости
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential \
      gcc \
      g++ \
      libsnappy-dev \
      libpq-dev \
      postgresql-client \
    && rm -rf /var/lib/apt/lists/*

# Устанавливаем рабочую директорию
WORKDIR /app

# Устанавливаем переменные окружения
ENV PYTHONUNBUFFERED=1
ENV PYTHONPATH=/app

# Копируем requirements.txt для кэширования слоев
COPY src/python_analytics/requirements.txt ./

# Устанавливаем зависимости Python
RUN pip install --no-cache-dir -r requirements.txt

# Копируем исходный код приложения
COPY src/python_analytics/ /app/python_analytics/

# Открываем порт
EXPOSE 8001

# Запускаем FastAPI приложение
CMD ["uvicorn", "python_analytics.src.main:app", "--host", "0.0.0.0", "--port", "8001", "--reload"]
