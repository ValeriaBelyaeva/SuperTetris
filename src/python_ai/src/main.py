"""
Main module for Tetris AI Service
"""

from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from loguru import logger
import uvicorn
import os
from typing import Dict, Any

# Импорты из локальных модулей
from src.api import router as api_router
from src.ai_system import AISystem
from src.models import AIConfig

# Получаем настройки из переменных окружения
SERVER_HOST = os.getenv("SERVER_HOST", "0.0.0.0")
SERVER_PORT = int(os.getenv("SERVER_PORT", "8001"))

# Инициализация FastAPI приложения
app = FastAPI(
    title="Tetris AI",
    description="AI service for Tetris game",
    version="1.0.0"
)

# Настройка CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Инициализация AI системы
ai_system = AISystem()

@app.on_event("startup")
async def startup_event():
    """Инициализация сервиса при запуске"""
    try:
        logger.info("Starting AI service...")
        await ai_system.start()
        logger.info("AI service started successfully")
    except Exception as e:
        logger.error(f"Failed to start AI service: {e}")
        raise

@app.on_event("shutdown")
async def shutdown_event():
    """Очистка ресурсов при остановке"""
    try:
        logger.info("Stopping AI service...")
        await ai_system.stop()
        logger.info("AI service stopped successfully")
    except Exception as e:
        logger.error(f"Error during shutdown: {e}")

# Подключаем роутер API
app.include_router(api_router, prefix="/api/v1")

@app.get("/health")
async def health_check() -> Dict[str, str]:
    """Проверка работоспособности сервиса"""
    return {"status": "ok", "service": "ai"}

if __name__ == "__main__":
    uvicorn.run(
        "src.main:app",
        host=SERVER_HOST,
        port=SERVER_PORT,
        reload=True
    ) 