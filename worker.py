from fastapi import FastAPI,Request, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles

import subprocess
import json
import os
from dotenv import load_dotenv # Optional: pip install python-dotenv
import google.generativeai as genai

# Load from .env file if you prefer that method
load_dotenv() 

# Get the key from the OS environment
api_key = os.getenv("GEMINI_API_KEY")

if not api_key:
    print("ERROR: GEMINI_API_KEY not found in environment variables!")
else:
    genai.configure(api_key=api_key)
    ai_model = genai.GenerativeModel('gemini-2.0-flash')






app = FastAPI()

# Enable CORS so your Firebase site can talk to your laptop
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Allows Firebase to access your laptop
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)
# FIXED LINE: Added quotes around /models, ., and models
release_path = r"C:\Pratyush\stunad-engine\build\test\Release"
EXE_PATH = os.path.join(release_path, "stunad_test.exe")

# Update the mount to look inside your build folder for the models
models_path = os.path.join(release_path, "models")
if not os.path.exists(models_path):
    os.makedirs(models_path)

app.mount("/models", StaticFiles(directory=models_path), name="models")



CAD_SYSTEM_PROMPT = """
You are the AI Compiler for the Stunad Geometric Kernel. 
Translate human requests into a JSON array of 'Ops' matching this C++ API:

### API SPECIFICATION (Functions & Params):
1. Profiles (2D):
   - "CircleProfile": [radius]
   - "RectProfile": [x, y]
   - "PolygonProfile": [[x1, y1], [x2, y2], ...] -> Flatten to [x1, y1, x2, y2...] in params
   - "SplineProfile": [[x1, y1], [x2, y2], ...] -> Flatten to [x1, y1, x2, y2...] in params

2. Profile Transforms (Moves 2D profiles in 3D space):
   - "ProfileTranslate": [x, y, z] (Target: refA)
   - "ProfileRotate": [angleDeg] (Target: refA)
   - "ProfileScale": [s] (Target: refA)

3. Solids from Profiles:
   - "Loft": No params. (Inputs: refList = [id1, id2, ... idN] in order)

4. Basic Solids:
   - "Box": [dx, dy, dz]
   - "Cylinder": [r, h]
   - "Sphere": [r]
   - "Cone": [rBottom, rTop, height]

5. Boolean Operations:
   - "Union", "Cut", "Intersect": No params. (refA = Base, refB = Tool)

6. Modifiers:
   - "Shell": [thickness] (Target: refA)
   - "Fillet": [radius] (Target: refA, selectionRule: "all_edges")

7. Transformations:
   - "Translate": [x, y, z] (Target: refA)
   - "Rotate": [rx, ry, rz] (Target: refA)
   - "Scale": [s] or [sx, sy, sz] (Target: refA)

When transforming an object for a boolean operation, ensure the boolean refers to the ID of the transformed result.

### JSON STRUCTURE:
- Each op must have "id", "type", and "params".
- Use "refA", "refB", or "refList" as specified above.

Example for a Cone with a Hole:
{
  "ops": [
    {"id": "c1", "type": "Cone", "params": [20, 0, 50]},
    {"id": "h1", "type": "Cylinder", "params": [5, 60]},
    {"id": "f1", "type": "Cut", "refA": "c1", "refB": "h1"}
  ]
}
"""



@app.post("/ai-prompt")
async def ai_prompt(request: Request):
    data = await request.json()
    user_input = data.get("prompt")
    print(f"\n--- NEW AI REQUEST: {user_input} ---", flush=True)
    
    try:
        # Ask Gemini to generate the JSON
        response = ai_model.generate_content(f"{CAD_SYSTEM_PROMPT}\nUser: {user_input}")
        
        # Clean the response text
        clean_json = response.text.replace('```json', '').replace('```', '').strip()
        payload = json.loads(clean_json)
        print(f"DEBUG: AI generated this JSON: {json.dumps(payload, indent=2)}")
        # Execute your C++ Kernel with the AI-generated JSON
        result = subprocess.run(
            [EXE_PATH, json.dumps(payload)],
            capture_output=True, text=True,
            cwd=release_path
        )
        print("C++ Standard Output:", result.stdout, flush=True)
        print("C++ Errors (if any):", result.stderr, flush=True)
        
        return {"status": "success", "payload": payload, "output": result.stdout}
        
    except Exception as e:
        print(f"!!! PYTHON ERROR: {str(e)}", flush=True)
        return {"status": "error", "message": str(e)}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)