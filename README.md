# Metasequoia IME Engine(水杉输入法引擎)

General IME engine for [MetaIME](https://github.com/fanlumaster/MetaIME).

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
