"""
Main module for Tetris Server
"""

from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from loguru import logger
import uvicorn
from typing import Dict, Any

# Импорты из локальных модулей
from .config import Settings
from .game.server import GameServer
from .network.websocket import WebSocketManager
from .session.session_manager import SessionManager

# Инициализация FastAPI приложения
app = FastAPI(
    title="Tetris Server",
    description="Server for Tetris game",
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
game_server = GameServer(settings)
websocket_manager = WebSocketManager(game_server)
session_manager = SessionManager(settings)

@app.on_event("startup")
async def startup_event():
    """Инициализация сервиса при запуске"""
    try:
        logger.info("Starting server...")
        await game_server.start()
        await websocket_manager.start()
        await session_manager.start()
        logger.info("Server started successfully")
    except Exception as e:
        logger.error(f"Failed to start server: {e}")
        raise

@app.on_event("shutdown")
async def shutdown_event():
    """Очистка ресурсов при остановке"""
    try:
        logger.info("Stopping server...")
        await game_server.stop()
        await websocket_manager.stop()
        await session_manager.stop()
        logger.info("Server stopped successfully")
    except Exception as e:
        logger.error(f"Error during shutdown: {e}")

@app.get("/health")
async def health_check() -> Dict[str, str]:
    """Проверка работоспособности сервиса"""
    return {"status": "ok", "service": "server"}

if __name__ == "__main__":
    uvicorn.run(
        "main:app",
        host="0.0.0.0",
        port=8000,
        reload=True
    ) 