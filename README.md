# FanIme Engine

General IME engine for [FanImeTsf](https://github.com/fanlumaster/FanImeTsf).

## How to build

Prerequisites:

- Visual Studio 2022
- CMake 3.21+
- Python 3.10+
- vcpkg
- Git

Build steps:

```powershell
git clone --recursive git@github.com:fanlumaster/FanImeEngine.git
cd .\FanImeEngine\
python .\tests\scripts\prepare_env.py
cd .\tests\
.\scripts\llaunch.ps1
```

Then, you could check the outputs in the terminal.
