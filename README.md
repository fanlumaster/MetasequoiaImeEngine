# MetasequoiaIme Engine

General IME engine for [MetasequoiaImeTsf](https://github.com/fanlumaster/MetasequoiaImeTsf).

## How to build

Prerequisites:

- Visual Studio 2022
- CMake 3.21+
- Python 3.10+
- vcpkg
- Git

Build steps:

```powershell
git clone --recursive git@github.com:fanlumaster/MetasequoiaImeEngine.git
cd .\MetasequoiaImeEngine\
python .\tests\scripts\prepare_env.py
cd .\tests\
.\scripts\llaunch.ps1
```

Then, you could check the outputs in the terminal.
