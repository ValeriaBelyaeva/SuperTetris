FROM python:3.11-slim

WORKDIR /app
COPY ./src/python_tools/ /app/python_tools/

RUN pip install --no-cache-dir -r /app/python_tools/requirements.txt

ENV PYTHONPATH=/app

CMD ["python", "-m", "python_tools.main"] 