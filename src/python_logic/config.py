from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    # Основные настройки
    APP_NAME: str = "SuperTetris Physics Engine"
    DEBUG: bool = False
    
    # Настройки физического движка
    PHYSICS_UPDATE_RATE: float = 60.0  # Hz
    GRAVITY: float = 9.81
    MAX_VELOCITY: float = 100.0
    
    # Настройки сетевого взаимодействия
    PHYSICS_SERVER_HOST: str = "physics-server"
    PHYSICS_SERVER_PORT: int = 9000
    
    class Config:
        env_file = ".env"
        case_sensitive = True

settings = Settings() 