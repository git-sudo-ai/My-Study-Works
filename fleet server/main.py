from fastapi import FastAPI

app = FastAPI(title="Сервис мониторинга")


@app.get("/health")
async def health_check():
    return {"статус": "работает", "сообщение": "Сервис готов к приёму запросов"}
