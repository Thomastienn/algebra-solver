import asyncio

from fastapi import FastAPI, WebSocket
from fastapi.middleware.cors import CORSMiddleware

from structures import (
    SimplifyRequest, 
    SimplifyResponse,
    SystemSolveRequest,
    SystemSolveResponse
)
import cas

sessions = {}
app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
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

@app.post("/solve-system")
def solve_system(req: SystemSolveRequest) -> SystemSolveResponse:
    try:
        result = cas.solve(req.equations, req.variable)
        if result and result.strip():
            return SystemSolveResponse(result=f"{req.variable}={result}")
        else:
            return SystemSolveResponse(result="No solution found or system is inconsistent.")
    except Exception as e:
        return SystemSolveResponse(result=f"Error: {str(e)}")

@app.get("/")
def root():
    return {"message": "Welcome to the CAS backend!"}
