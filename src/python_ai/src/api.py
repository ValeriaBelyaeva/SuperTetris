"""
FastAPI service for AI system.
"""

import logging
from typing import Any, Dict
from fastapi import FastAPI, HTTPException, APIRouter
from src.ai_system import AISystem
from src.models import GameState, Action

# Настройка логирования
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Создаем роутер
router = APIRouter()

# Инициализация AI системы
ai_system = AISystem()

@router.get("/health", response_model=None)
async def health_check():
    """Health check endpoint."""
    return {"status": "ok"}

@router.post("/players/{player_type}", response_model=None)
async def create_player(player_type: str, difficulty: int, name: str = None):
    """Create a new AI player."""
    try:
        player = ai_system.create_player(player_type, difficulty, name)
        return {"status": "success", "player_name": player.name}
    except Exception as e:
        logger.error(f"Failed to create player: {e}")
        raise HTTPException(status_code=400, detail=str(e))

@router.post("/players/{player_name}/action", response_model=None)
async def get_action(player_name: str, state: Dict[str, Any]):
    """Get the next action for a player."""
    try:
        game_state = GameState(**state)  # Convert dict to GameState
        action = ai_system.get_action(player_name, game_state)
        return {"status": "success", "action": action.__dict__}
    except Exception as e:
        logger.error(f"Failed to get action: {e}")
        raise HTTPException(status_code=400, detail=str(e))

@router.post("/players/{player_name}/update", response_model=None)
async def update_player(
    player_name: str,
    state: Dict[str, Any],
    action: Dict[str, Any],
    reward: float,
    next_state: Dict[str, Any]
):
    """Update a player's knowledge."""
    try:
        game_state = GameState(**state)
        game_action = Action(**action)
        next_game_state = GameState(**next_state)
        ai_system.update(player_name, game_state, game_action, reward, next_game_state)
        return {"status": "success"}
    except Exception as e:
        logger.error(f"Failed to update player: {e}")
        raise HTTPException(status_code=400, detail=str(e))

@router.post("/players/{player_name}/train", response_model=None)
async def train_player(player_name: str, epochs: int = 100, batch_size: int = 32):
    """Train a player using collected training data."""
    try:
        ai_system.train_player(player_name, epochs, batch_size)
        return {"status": "success"}
    except Exception as e:
        logger.error(f"Failed to train player: {e}")
        raise HTTPException(status_code=400, detail=str(e))

@router.get("/players/{player_name}", response_model=None)
async def get_player(player_name: str):
    """Get player information."""
    player = ai_system.get_player(player_name)
    if not player:
        raise HTTPException(status_code=404, detail=f"Player not found: {player_name}")
    return {"status": "success", "player": player.name}

@router.delete("/players/{player_name}", response_model=None)
async def remove_player(player_name: str):
    """Remove a player."""
    try:
        ai_system.remove_player(player_name)
        return {"status": "success"}
    except Exception as e:
        logger.error(f"Failed to remove player: {e}")
        raise HTTPException(status_code=400, detail=str(e)) 