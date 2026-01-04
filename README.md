# CAN Bus Desktop Application

A cross-platform C++ desktop application for visualizing and interacting with CAN bus data, built using **SFML** and **ImGui (ImGui-SFML)**.

This project is designed to build and run on **Linux, Windows, and macOS** using **CMake**.

---

## Project Structure

```
CAN-Bus-Desktop-App/
├── assets/         # Fonts, images, and other runtime assets
├── dependencies/   # External dependencies (SFML, ImGui-SFML)
├── include/        # Project header files
├── src/            # Source files
├── CMakeLists.txt  # Top-level CMake configuration
└── build/          # Out-of-source build directory
```

---

## Requirements

- **CMake** (version 3.15 or newer)
- A C++17-compatible compiler  
  - GCC / Clang (Linux, WSL)
  - MSVC or MinGW (Windows)
  - Apple Clang (macOS)

> External dependencies such as SFML and ImGui-SFML are fetched and built automatically via CMake.

---

## Platform Support

This project is intended to be **cross-platform**:

- ✅ Linux (tested on Ubuntu / WSL)
- ✅ Windows
- ✅ macOS

CMake handles platform-specific build details automatically.  
Minor platform differences (executable extensions, runtime libraries) are handled by CMake or the operating system.

---

## Build Instructions

This project uses an **out-of-source build**.  
All build commands should be run from the `build/` directory.

### 1. Configure the project

```
cmake ../CAN-Bus-Desktop-App
```

This step generates the build system files.

---

### 2. Build the executable

```
cmake --build . -j
```

The `-j` flag enables parallel compilation.

---

### 3. Run the application

#### Linux / WSL / macOS

```
./src/CAN_BUS_IOT_exe
```

#### Windows (PowerShell)

```powershell
.\src\CAN_BUS_IOT_exe.exe
```

---

## Assets and Runtime Files

- Fonts and images are stored in the `assets/` directory in the source tree.
- During the build, assets are automatically copied next to the executable.
- Asset paths are resolved internally, so the application can be run from any directory.

---

## Networking Notes

- Avoid using privileged ports (e.g. port 22) unless running with elevated permissions.
- Use ports ≥ 1024 for cross-platform compatibility.

---

## Notes

- On Windows, required SFML runtime DLLs must be available next to the executable when using dynamic linking.
- On macOS, the application can be run from the terminal; app bundle packaging can be added later if desired.

---

