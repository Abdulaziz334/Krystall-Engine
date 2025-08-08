<#
Start-AGI.ps1
Creates a MetalifeX-AGI starter project (or clones a repo), sets up Python venv,
installs dependencies, creates .env from example, and launches the agent.

Usage:
  # 1) To create local project:
  .\start_agi.ps1

  # 2) To clone an existing GitHub repo and run:
  .\start_agi.ps1 -RepoUrl "https://github.com/yourname/metalifex-agi.git"

Notes:
 - Requires Python 3.8+ on PATH.
 - Run PowerShell with sufficient privileges to create files.
 - This script does NOT expose your API keys; edit .env manually before running OpenAI.
#>

param(
    [string]$RepoUrl = "",
    [string]$Dir = "metalifex-agi",
    [switch]$Force
)

function Write-Ok($msg){ Write-Host "[OK]  $msg" -ForegroundColor Green }
function Write-Warn($msg){ Write-Host "[WARN] $msg" -ForegroundColor Yellow }
function Write-Err($msg){ Write-Host "[ERR]  $msg" -ForegroundColor Red }

# 1) Prepare folder
if (Test-Path $Dir) {
    if (-not $Force) {
        $ans = Read-Host "Directory '$Dir' already exists. Remove and recreate? (y/N)"
        if ($ans -ne 'y') { Write-Warn "Aborted by user."; exit 1 }
    }
    Remove-Item -Recurse -Force $Dir
}
New-Item -ItemType Directory -Path $Dir | Out-Null
Set-Location $Dir

# 2) Clone or create files
if ($RepoUrl -ne "") {
    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        Write-Warn "git not found. Install git or run without -RepoUrl to create files locally."
    } else {
        Write-Host "Cloning $RepoUrl ..."
        git clone $RepoUrl . 2>&1
        Write-Ok "Cloned repo."
    }
} else {
    Write-Host "Creating starter project files..."
    # README
    @"
# MetalifeX-AGI (Starter)

Quick start: create venv, install deps, set .env, run.

Run: `python src/main.py`
"@ | Out-File -Encoding UTF8 README.md

    # .gitignore
    @"
venv/
__pycache__/
.env
memory.json
.env.*
*.pyc
.DS_Store
"@ | Out-File -Encoding UTF8 .gitignore

    # example.env
    @"
# Example env for local run
OPENAI_API_KEY=
HF_MODEL=gpt2
PORT=8080
"@ | Out-File -Encoding UTF8 example.env

    # requirements.txt (conservative)
    @"
openai>=0.27.0
transformers>=4.30.0
torch>=2.0.0
requests
beautifulsoup4
pytest
python-dotenv
chromadb
langchain
"@ | Out-File -Encoding UTF8 requirements.txt

    # Create src structure
    New-Item -ItemType Directory -Path src | Out-Null
    New-Item -ItemType Directory -Path src\models | Out-Null
    New-Item -ItemType Directory -Path src\agent | Out-Null

    # src/models/llm_wrapper.py
    @"
import os
from typing import Optional

class LLMWrapper:
    def __init__(self):
        self.backend = None
        self.openai = None
        self.pipeline = None
        if os.getenv('OPENAI_API_KEY'):
            try:
                import openai
                openai.api_key = os.getenv('OPENAI_API_KEY')
                self.openai = openai
                self.backend = 'openai'
                print('LLM: using OpenAI')
            except Exception as e:
                print('OpenAI init error:', e)
        if self.backend is None and os.getenv('HF_MODEL'):
            try:
                from transformers import pipeline
                model = os.getenv('HF_MODEL')
                print(f'LLM: loading local HF model {model} (may be slow)')
                self.pipeline = pipeline('text-generation', model=model, device_map='auto')
                self.backend = 'hf'
            except Exception as e:
                print('HF init error:', e)
        if self.backend is None:
            print('No LLM backend available. Set OPENAI_API_KEY or HF_MODEL.')

    def generate(self, prompt: str, max_tokens: int = 256, temperature: float = 0.2) -> str:
        if self.backend == 'openai':
            try:
                resp = self.openai.ChatCompletion.create(
                    model='gpt-3.5-turbo',
                    messages=[{'role':'user','content':prompt}],
                    max_tokens=max_tokens,
                    temperature=temperature
                )
                return resp.choices[0].message.content.strip()
            except Exception as e:
                return f'[LLM ERROR: {e}]'
        elif self.backend == 'hf':
            try:
                out = self.pipeline(prompt, max_new_tokens=max_tokens, do_sample=True, temperature=temperature)
                text = out[0]['generated_text']
                if text.startswith(prompt):
                    text = text[len(prompt):]
                return text.strip()
            except Exception as e:
                return f'[LLM ERROR: {e}]'
        else:
            return '[NO LLM BACKEND]'
"@ | Out-File -Encoding UTF8 src\models\llm_wrapper.py

    # src/agent/planner.py
    @"
from typing import List

class Planner:
    def __init__(self, llm):
        self.llm = llm

    def create_plan(self, task: str) -> List[str]:
        prompt = (\"You are an orderly planner. Break the following task into 3-6 concise, numbered steps.\\n\"
                  f\"Task:\\n{task}\")
        out = self.llm.generate(prompt, max_tokens=200)
        steps = []
        for line in out.splitlines():
            line=line.strip()
            if not line: continue
            if line[0].isdigit():
                parts=line.split('.',1)
                if len(parts)>1: steps.append(parts[1].strip())
                else: steps.append(line)
            else:
                steps.append(line)
        if not steps: steps=[task]
        return steps
"@ | Out-File -Encoding UTF8 src\agent\planner.py

    # src/agent/memory.py
    @"
import json, os, time
MEM_FILE = 'memory.json'

def load_memory():
    if os.path.exists(MEM_FILE):
        try:
            with open(MEM_FILE,'r',encoding='utf-8') as f:
                return json.load(f)
        except Exception:
            return {'history':[]}
    return {'history':[]}

def save_memory(mem):
    with open(MEM_FILE,'w',encoding='utf-8') as f:
        json.dump(mem,f,ensure_ascii=False,indent=2)
"@ | Out-File -Encoding UTF8 src\agent\memory.py

    # src/agent/executor.py
    @"
import shlex, subprocess, time
from .memory import save_memory

def safe_run_shell(command:str, timeout:int=8):
    try:
        args = shlex.split(command)
        proc=subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        out,err = proc.communicate(timeout=timeout)
        return {'ok':True,'stdout':out,'stderr':err,'rc':proc.returncode}
    except subprocess.TimeoutExpired:
        proc.kill()
        return {'ok':False,'error':'timeout'}
    except FileNotFoundError:
        return {'ok':False,'error':'not found'}
    except Exception as e:
        return {'ok':False,'error':str(e)}

class Executor:
    def __init__(self, llm, memory):
        self.llm = llm
        self.memory = memory

    def execute_step(self, step:str):
        lower=step.lower()
        if lower.startswith('run:'):
            cmd=step.split(':',1)[1].strip()
            print(f'Shell request: {cmd}')
            ok = input('Allow run? (y/N): ').strip().lower()
            if ok!='y':
                return {'action':'run','cmd':cmd,'result':'denied'}
            res=safe_run_shell(cmd)
            self.memory.setdefault('history',[]).append({'time':int(time.time()),'step':step,'result':res})
            save_memory(self.memory)
            return {'action':'run','cmd':cmd,'result':res}

        elif lower.startswith('write:'):
            rest = step.split(':',1)[1].strip()
            if '|' in rest:
                fname,content = rest.split('|',1)
                with open(fname.strip(),'w',encoding='utf-8') as f:
                    f.write(content.strip())
                res={'ok':True,'file':fname.strip()}
                self.memory.setdefault('history',[]).append({'time':int(time.time()),'step':step,'result':res})
                save_memory(self.memory)
                return {'action':'write','result':res}
            return {'action':'write','error':'bad format'}

        else:
            prompt=f'Perform the step concisely: {step}'
            out = self.llm.generate(prompt, max_tokens=200)
            res={'action':'llm','text':out}
            self.memory.setdefault('history',[]).append({'time':int(time.time()),'step':step,'result':res})
            save_memory(self.memory)
            return res
"@ | Out-File -Encoding UTF8 src\agent\executor.py

    # src/agi_agent.py (main)
    @"
\"\"\" Simple interactive AGI-like agent (starter) \"\"\"
import os, json, time, shlex, subprocess
from models.llm_wrapper import LLMWrapper
from agent.memory import load_memory, save_memory

def web_search_duckduckgo(query: str, max_results: int = 5):
    try:
        import requests
        from bs4 import BeautifulSoup
    except Exception:
        return [{'title':'error','snippet':'requests or bs4 missing'}]
    url = 'https://html.duckduckgo.com/html/'
    resp = requests.post(url, data={'q': query}, timeout=15)
    from bs4 import BeautifulSoup
    soup = BeautifulSoup(resp.text, 'html.parser')
    results = []
    for a in soup.select('a.result__a')[:max_results]:
        title = a.get_text()
        href = a.get('href')
        snippet_tag = a.find_parent().select_one('.result__snippet')
        snippet = snippet_tag.get_text() if snippet_tag else ''
        results.append({'title': title, 'link': href, 'snippet': snippet})
    return results

class Planner:
    def __init__(self, llm):
        self.llm = llm
    def create_plan(self, task: str):
        prompt = (\"You are an orderly planner. Break the following task into 3-6 concise, numbered steps.\\n\"
                  f\"Task:\\n{task}\")
        out = self.llm.generate(prompt, max_tokens=200)
        steps = []
        for line in out.splitlines():
            line=line.strip()
            if not line: continue
            if line[0].isdigit():
                parts=line.split('.',1)
                if len(parts)>1: steps.append(parts[1].strip())
                else: steps.append(line)
            else:
                steps.append(line)
        if not steps: steps=[task]
        return steps

class Executor:
    def __init__(self, llm, memory):
        self.llm = llm
        self.memory = memory
    def execute_step(self, step: str):
        lower = step.lower()
        if lower.startswith('search:'):
            q = step.split(':',1)[1].strip()
            results = web_search_duckduckgo(q)
            return {'action':'search','query':q,'results':results}
        elif lower.startswith('run:'):
            cmd = step.split(':',1)[1].strip()
            print(f'Shell request: {cmd}')
            ok = input('Allow execution? (y/N): ').strip().lower()
            if ok!='y':
                return {'action':'run','cmd':cmd,'result':'denied'}
            try:
                args = shlex.split(cmd)
                proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
                out,err = proc.communicate(timeout=10)
                res={'ok':True,'stdout':out,'stderr':err}
            except Exception as e:
                res={ 'ok':False,'error':str(e) }
            self.memory.setdefault('history',[]).append({'time':int(time.time()),'step':step,'result':res})
            save_memory(self.memory)
            return {'action':'run','cmd':cmd,'result':res}
        elif lower.startswith('write:'):
            rest = step.split(':',1)[1].strip()
            if '|' in rest:
                filename, content = rest.split('|',1)
                filename=filename.strip()
                content=content.strip()
                with open(filename,'w',encoding='utf-8') as f:
                    f.write(content)
                res={'ok':True,'file':filename}
                self.memory.setdefault('history',[]).append({'time':int(time.time()),'step':step,'result':res})
                save_memory(self.memory)
                return {'action':'write','result':res}
            else:
                return {'action':'write','error':'bad format'}
        else:
            prompt = f'Perform the following step concisely: {step}'
            out = self.llm.generate(prompt, max_tokens=200)
            res={'action':'llm','text':out}
            self.memory.setdefault('history',[]).append({'time':int(time.time()),'step':step,'result':res})
            save_memory(self.memory)
            return res

def main():
    llm = LLMWrapper()
    mem = load_memory()
    planner = Planner(llm)
    executor = Executor(llm, mem)

    print('MetalifeX AGI - interactive agent. \"exit\" to quit.')
    while True:
        task = input('\\nEnter task: ').strip()
        if not task: continue
        if task.lower() in ('exit','quit'): break
        mem.setdefault('history',[]).append({'time':int(time.time()),'task':task})
        save_memory(mem)
        steps = planner.create_plan(task)
        print('\\nPlan:')
        for i,s in enumerate(steps,1): print(f'{i}. {s}')
        run = input('Run plan? (y/N): ').strip().lower()
        if run!='y': continue
        results=[]
        for s in steps:
            r = executor.execute_step(s)
            results.append(r)
        import json; print(json.dumps(results,ensure_ascii=False,indent=2))

if __name__=='__main__':
    main()
"@ | Out-File -Encoding UTF8 src\agi_agent.py

    # src/main.py
    @"
from agi_agent import main

if __name__=='__main__':
    main()
"@ | Out-File -Encoding UTF8 src\main.py

    Write-Ok "Starter project files created."
}

# 3) Create (or reuse) venv
if (-not (Test-Path ".\venv")) {
    Write-Host "Creating Python virtual environment..."
    python -m venv venv
} else {
    Write-Host "Using existing virtual environment at .\\venv"
}

# Activate venv in this session
$venvActivate = Join-Path (Get-Location) "venv/Scripts/Activate.ps1"
if (Test-Path $venvActivate) {
    . $venvActivate
    Write-Ok "Virtual environment activated."
} else {
    Write-Warn "Virtual environment activation script not found. Ensure python3 venv created."
}

# 4) Upgrade pip and install deps
Write-Host "Upgrading pip..."
pip install --upgrade pip

Write-Host "Installing requirements (may take several minutes)..."
pip install -r requirements.txt

Write-Ok "Dependencies installed."

# 5) Copy example.env to .env if not present
if (-not (Test-Path ".env")) {
    if (Test-Path "example.env") {
        Copy-Item example.env .env
        Write-Host "`nA copy of example.env has been created as .env."
        Write-Host "Edit .env and add your OPENAI_API_KEY or set HF_MODEL before using OpenAI or local HF."
    } else {
        Write-Warn "example.env not found; create .env manually."
    }
} else {
    Write-Host ".env already exists."
}

# 6) Run tests (optional)
if (Get-Command pytest -ErrorAction SilentlyContinue) {
    Write-Host "Running smoke tests..."
    pytest -q || Write-Warn "Tests failed or pytest returned non-zero code (continue anyway)."
} else {
    Write-Host "pytest not found in PATH (skipping tests)."
}

# 7) Launch AGI (background)
$startChoice = Read-Host "Start agent now? (1=foreground, 2=background) [1]"
if ($startChoice -eq "2") {
    Write-Host "Starting agent in background window..."
    Start-Process -FilePath "powershell" -ArgumentList "-NoExit","-Command","python src/main.py" -WindowStyle Normal
    Write-Ok "Agent started in new window (foreground process)."
} else {
    Write-Host "Running agent in this console (Ctrl+C to stop)..."
    python src/main.py
}

Write-Ok "Script complete. If agent didn't start, inspect files and .env settings."