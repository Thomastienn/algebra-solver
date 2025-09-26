import asyncio

from fastapi import FastAPI, WebSocket
from fastapi.middleware.cors import CORSMiddleware

from structures import (
    SimplifyRequest, 
    SimplifyResponse
)
import cas

sessions = {}
app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["https://algebra-solver-nine.vercel.app"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.websocket("/connect/{session_id}")
async def connect(ws: WebSocket, session_id: str):
    await ws.accept()
    sessions[session_id] = ws
    try:
        while True:
            await asyncio.sleep(10)  # keep handler alive
    finally:
        # Browser closed -> cleanup
        sessions.pop(session_id, None)
        print(f"Frontend {session_id} disconnected")
        await ws.close()

@app.post("/simplify")
def simplify(req: SimplifyRequest) -> SimplifyResponse:
    cas_result = cas.simplify(req.expression)
    return SimplifyResponse(simplified=cas_result)
