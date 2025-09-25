from pydantic import BaseModel

class SimplifyRequest(BaseModel):
    expression: str

class SimplifyResponse(BaseModel):
    simplified_expression: str
