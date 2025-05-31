from fastapi import FastAPI
from loguru import logger
from config import Settings
from data_export import DataExporter
from alert_system import AlertSystem

app = FastAPI(title="Tetris Analytics")
settings = Settings()

# Инициализация компонентов
data_exporter = DataExporter(settings)
alert_system = AlertSystem(settings)

@app.on_event("startup")
async def startup_event():
    logger.info("Starting analytics service...")
    await data_exporter.start()
    await alert_system.start()
    logger.info("Analytics service started successfully")

@app.on_event("shutdown")
async def shutdown_event():
    logger.info("Stopping analytics service...")
    await alert_system.stop()
    await data_exporter.stop()
    logger.info("Analytics service stopped successfully")

@app.get("/health")
async def health_check():
    return {"status": "ok"}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "main:app",
        host=settings.server_host,
        port=settings.server_port,
        reload=True
    ) 