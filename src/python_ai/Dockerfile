# syntax=docker/dockerfile:1.4
FROM python:3.11-slim

WORKDIR /app/ai
# 1) Сначала скопировать только requirements.txt
COPY src/python_ai/requirements.txt .

# 2) Установить зависимости
RUN pip install --no-cache-dir -r requirements.txt

# 3) После этого уже копировать весь остальной код
COPY src/python_ai/ .

CMD ["python", "main.py"]
