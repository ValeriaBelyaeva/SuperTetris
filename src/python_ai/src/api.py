"""
FastAPI service for AI system.
"""

import logging
from fastapi import FastAPI, HTTPException
from .ai_system import AISystem
from .models import GameState, Action

# Настройка логирования
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

app = FastAPI(title="Tetris AI Service")
ai_system = AISystem()

@app.get("/health")
async def health_check():
    """Health check endpoint."""
    return {"status": "ok"}

@app.post("/players/{player_type}")
async def create_player(player_type: str, difficulty: int, name: str = None):
    """Create a new AI player."""
    try:
        player = ai_system.create_player(player_type, difficulty, name)
        return {"status": "success", "player_name": player.name}
    except Exception as e:
        logger.error(f"Failed to create player: {e}")
        raise HTTPException(status_code=400, detail=str(e))

@app.post("/players/{player_name}/action")
async def get_action(player_name: str, state: GameState):
    """Get the next action for a player."""
    try:
        action = ai_system.get_action(player_name, state)
        return {"status": "success", "action": action}
    except Exception as e:
        logger.error(f"Failed to get action: {e}")
        raise HTTPException(status_code=400, detail=str(e))

@app.post("/players/{player_name}/update")
async def update_player(player_name: str, state: GameState, action: Action, reward: float, next_state: GameState):
    """Update a player's knowledge."""
    try:
        ai_system.update(player_name, state, action, reward, next_state)
        return {"status": "success"}
    except Exception as e:
        logger.error(f"Failed to update player: {e}")
        raise HTTPException(status_code=400, detail=str(e))

@app.post("/players/{player_name}/train")
async def train_player(player_name: str, epochs: int = 100, batch_size: int = 32):
    """Train a player using collected training data."""
    try:
        ai_system.train_player(player_name, epochs, batch_size)
        return {"status": "success"}
    except Exception as e:
        logger.error(f"Failed to train player: {e}")
        raise HTTPException(status_code=400, detail=str(e))

@app.get("/players/{player_name}")
async def get_player(player_name: str):
    """Get player information."""
    player = ai_system.get_player(player_name)
    if not player:
        raise HTTPException(status_code=404, detail=f"Player not found: {player_name}")
    return {"status": "success", "player": player.name}

@app.delete("/players/{player_name}")
async def remove_player(player_name: str):
    """Remove a player."""
    try:
        ai_system.remove_player(player_name)
        return {"status": "success"}
    except Exception as e:
        logger.error(f"Failed to remove player: {e}")
        raise HTTPException(status_code=400, detail=str(e)) 