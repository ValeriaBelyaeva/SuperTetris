FROM python:3.11-slim

WORKDIR /app/src
COPY ./src/ /app/

RUN pip install --no-cache-dir -r /app/python_server/requirements.txt

CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000", "--reload"] 