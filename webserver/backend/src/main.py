import asyncio

from fastapi import FastAPI, WebSocket
from fastapi.middleware.cors import CORSMiddleware

from .structures import (
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
        response_dict = cas.solve(req.equations, req.variable)
        result_str = response_dict.get("result", "")
        steps = response_dict.get("steps", [])
        
        if result_str and result_str.strip():
            return SystemSolveResponse(result=result_str, steps=steps)
        else:
            return SystemSolveResponse(result="No solution found or system is inconsistent.", steps=steps)
    except Exception as e:
        return SystemSolveResponse(result=f"Error: {str(e)}", steps=[])

@app.get("/")
def root():
    return {"message": "Welcome to the CAS backend!"}
