from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
import subprocess
import json
import os

app = FastAPI()

# Enable CORS so your Firebase site can talk to your laptop
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], 
    allow_methods=["*"],
    allow_headers=["*"],
)

# FIXED LINE: Added quotes around /models, ., and models
release_path = r"C:\Pratyush\stunad-engine\build\test\Release"
app.mount("/models", StaticFiles(directory="./models"), name="models")

@app.post("/generate")
async def generate_model(data: dict):
    try:
        json_payload = json.dumps(data)
        
        # Run your C++ Engine
        result = subprocess.run(
            ["./stunad_test.exe", json_payload], 
            capture_output=True, 
            text=True
        )
        
        if result.returncode != 0:
            print("Engine Error:", result.stderr)
            raise HTTPException(status_code=500, detail=result.stderr)
            
        return {"gltf_url": "/models/output.gltf"}
        
    except Exception as e:
        print("System Error:", str(e))
        raise HTTPException(status_code=500, detail=str(e))

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)