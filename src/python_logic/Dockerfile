# Dockerfile для Python игровой логики
FROM python:3.11-slim

WORKDIR /app/logic
COPY ./src/python_logic/ /app/logic/

RUN pip install --no-cache-dir -r requirements.txt

CMD ["python", "main.py"]
 