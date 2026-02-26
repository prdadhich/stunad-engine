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
Translate human requests into a JSON array of 'Ops' matching the C++ API. 
Your goal is to generate the most EFFICIENT and MATHEMATICALLY ACCURATE procedural history.

### MANDATORY DESIGN RULES:
1. NEVER use manual Union/Cut chains for repeating features (e.g., id: union1, union2...).
2. ALWAYS use "PatternLinear", "PatternCircular", or "PatternSpiral" for arrays.
3. ALWAYS use "AlignProfileToPath" before a "Sweep" to ensure the section is perpendicular to the path.
4. SPATIAL MATH: To make a profile (Width W) touch a central pole (Radius R), translate the profile by (R + W/2) on the X-axis BEFORE patterning or extruding.

### CRITICAL GEOMETRY RULES:
1. NO MANUAL CHAINS: Never create multiple ops like "union_1", "union_2". If an item repeats, you MUST use "PatternLinear", "PatternCircular", or "PatternSpiral".
2. CENTER VS EDGE: Profiles (Rect, Circle) are centered at (0,0). To make a RectProfile of Width W touch a Cylinder of Radius R, you MUST Translate it by (R + W/2) on the X-axis before patterning.
3. SPIRAL STEPS: Always use "PatternSpiral" for staircases. One op creates the entire staircase.

### STRICT OPERATIONAL RULES:
1. NO TYPE SWAPPING: If a request mentions a 'Spiral' or 'Pattern', you MUST use "PatternSpiral", "PatternLinear", or "PatternCircular". NEVER replace these with "Translate" or "Extrude".
2. HELICAL FORMULA: For any thread or screw, totalAngle = (Number of Turns * 360). 
3. SOLIDS ONLY: Always "Extrude" a profile into a small solid (makeSolid=1) before using it in a PatternSpiral to ensure valid Boolean operations.

### API SPECIFICATION:
1. Profiles (2D Intent, born on XY plane):
   - "CircleProfile": [radius]
   - "RectProfile": [width, height]
   - "PolygonProfile": [x1, y1, x2, y2, ...] 
   - "SplineProfile": [isClosed(0/1), x1, y1, x2, y2, ...]

2. Orientation & 3D Placement:
   - "SetProfilePlane": [ox, oy, oz, nx, ny, nz] (Origin and Normal vector)
   - "AlignProfileToPath": No params. (refA = Profile, refB = Path)
   - "RotateProfile3D": [angleDeg, ax, ay, az] (Rotate around custom axis)

3. Solids from Profiles:
   - "Extrude": [height, makeSolid(1/0)]
   - "Sweep": [makeSolid(1/0)] (refA = Section, refB = Path)
   - "Loft": [ruled(1/0), makeSolid(1/0)] (refList = [IDs])
   - "Revolve": [angleDeg, makeSolid(1/0)]

4. Primitives:
   - "Box": [dx, dy, dz]
   - "Cylinder": [r, h]
   - "Sphere": [r]

5. Modifiers, Booleans & Symmetry:
   - "Union", "Cut", "Intersect": (refA = Base, refB = Tool)
   - "Fillet": [radius, mode(0=All, 1=Vertical, 2=Planar)]
   - "Shell": [thickness]
   - "Mirror": [ox, oy, oz, nx, ny, nz] (Mirror Plane origin and normal)

6. Patterns:
   - "PatternLinear": [count, spacing, dx, dy, dz]
   - "PatternCircular": [count, totalAngle, ax, ay, az]
   - "PatternSpiral": [count, totalAngle, totalRise, ax, ay, az] (Requires 6 parameters)

### JSON STRUCTURE:
- Each op must have "id", "type", "params" (list), and "refA"/"refB"/"refList" where needed.



### EXAMPLE (Correct Spiral Staircase):
- id: p1, type: Cylinder, params: [4, 120]
- id: s1, type: RectProfile, params: [30, 12]
- id: s2, type: Extrude, refA: s1, params: [3, 1]
- id: s3, type: Translate, refA: s2, params: [19, 0, 0] (Alignment: 4 + 30/2)
- id: stairs, type: PatternSpiral, refA: s3, params: [24, 360, 120, 0, 0, 1]
- id: final, type: Union, refA: p1, refB: stairs
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