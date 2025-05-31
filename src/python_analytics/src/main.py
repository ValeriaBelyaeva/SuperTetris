"""
Main module for Tetris Analytics Service
"""

from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from loguru import logger
import uvicorn
from typing import Dict, Any

# Импорты из локальных модулей
from .config import Settings
from .data_collectors import DataCollector
from .analyzers import GameAnalyzer
from .reporters import ReportGenerator
from .models import AnalyticsData

# Инициализация FastAPI приложения
app = FastAPI(
    title="Tetris Analytics",
    description="Analytics service for Tetris game",
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

# Инициализация компонентов
settings = Settings()
data_collector = DataCollector(settings)
game_analyzer = GameAnalyzer(settings)
report_generator = ReportGenerator(settings)

@app.on_event("startup")
async def startup_event():
    """Инициализация сервиса при запуске"""
    try:
        logger.info("Starting analytics service...")
        await data_collector.start()
        await game_analyzer.start()
        await report_generator.start()
        logger.info("Analytics service started successfully")
    except Exception as e:
        logger.error(f"Failed to start analytics service: {e}")
        raise

@app.on_event("shutdown")
async def shutdown_event():
    """Очистка ресурсов при остановке"""
    try:
        logger.info("Stopping analytics service...")
        await report_generator.stop()
        await game_analyzer.stop()
        await data_collector.stop()
        logger.info("Analytics service stopped successfully")
    except Exception as e:
        logger.error(f"Error during shutdown: {e}")

@app.get("/health")
async def health_check() -> Dict[str, str]:
    """Проверка работоспособности сервиса"""
    return {"status": "ok", "service": "analytics"}

@app.get("/api/v1/analytics/status")
async def get_status() -> Dict[str, Any]:
    """Получение статуса аналитики"""
    try:
        return {
            "status": "running",
            "version": "1.0.0",
            "collectors": await data_collector.get_status(),
            "analyzers": await game_analyzer.get_status(),
            "reporters": await report_generator.get_status()
        }
    except Exception as e:
        logger.error(f"Error getting status: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/api/v1/analytics/data")
async def get_analytics_data() -> AnalyticsData:
    """Получение аналитических данных"""
    try:
        return await game_analyzer.get_current_data()
    except Exception as e:
        logger.error(f"Error getting analytics data: {e}")
        raise HTTPException(status_code=500, detail=str(e))

if __name__ == "__main__":
    uvicorn.run(
        "src.main:app",
        host=settings.server_host,
        port=settings.server_port,
        reload=True
    ) 