# Win32 Four-Function Calculator

A small Win32 desktop calculator written in C that mimics a traditional four-function calculator layout.

## Build

1. Install Visual Studio 2022 (Community or Build Tools) with the Desktop development with C++ workload.
2. Open this folder in Visual Studio Code.
3. Press `Ctrl+Shift+B` to run the **Build WinCalc (MSVC)** task. It calls `build.bat`, which initializes the MSVC environment and compiles the app into `build/WinCalc.exe`.

Alternatively, run the script manually from a Developer Command Prompt:

```cmd
build.bat
```

## Run

After building, launch the calculator:

```cmd
build\WinCalc.exe
```

Enter numbers with the on-screen keypad. The calculator supports repeated equals operations and a clear (`C`) key.

## Project Layout

- `src/main.c` – Entry point, window registration, and message loop
- `src/ui.c`, `src/ui.h` – Win32 control creation and routing user input to the calculator engine
- `src/calculator.c`, `src/calculator.h` – Calculator state machine and arithmetic logic
- `src/resource.h` – Control identifiers shared by the UI and resources
- `build.bat` – Helper script that locates the Visual Studio toolchain and compiles the project
- `.vscode/tasks.json` – VS Code build task configuration
