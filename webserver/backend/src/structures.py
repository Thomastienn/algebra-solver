from pydantic import BaseModel
from typing import List, Dict

class SimplifyRequest(BaseModel):
    expression: str

class SimplifyResponse(BaseModel):
    simplified: str

class SystemSolveRequest(BaseModel):
    equations: List[str]
    variable: str

class SystemSolveResponse(BaseModel):
    result: str
    steps: List[str] = []
