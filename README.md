# CAN Bus Dashboard (ESP32 → TCP → C++ Desktop App)

A real-time **CAN bus visualization and diagnostics dashboard** built as an end-to-end system:

- **ESP32** reads CAN (TWAI) frames and streams them over **TCP**
- **C++ desktop application** receives, parses, decodes, and visualizes CAN data
- **SFML + ImGui** used for rendering, networking, and UI
- Designed using **multi-threading, producer–consumer queues, and clean separation of concerns**
<img width="300" height="auto" alt="can_bus_project_ss1" src="https://github.com/user-attachments/assets/8f1b7008-1ed2-4c1c-8c87-b8b29bb250c4" />
<img width="300" height="auto" alt="can_bus_project_ss2" src="https://github.com/user-attachments/assets/162d68d7-be26-4615-af50-265b9ccc669a" />



> ⚠️ This project is a **prototype / proof-of-concept** and has not (yet) been validated on a real vehicle.

---

## Overview

This project demonstrates a complete telemetry pipeline:

1. CAN frames are captured on an ESP32 using the TWAI driver
2. Frames are serialized into a fixed-size binary protocol
3. Data is streamed over TCP
4. A desktop C++ application parses and visualizes the data in real time

---

## Architecture Diagram

![System Architecture](docs/architecture.png)

---

## Key Features

### CAN & Networking
- ESP32 TWAI (CAN) driver integration
- TCP-based CAN frame streaming
- Fixed-size 13-byte CAN frame protocol
- Endianness-aware binary parsing
- Stream-safe TCP parsing (handles partial reads)

### Desktop Application
- Multi-threaded design:
  - Network thread for TCP + parsing
  - UI thread for ingestion + rendering
- Thread-safe producer–consumer queue
- Rolling CAN frame history
- Real-time signal decoding

### UI / Visualization
- SFML rendering (textures, sprites, fonts)
- ImGui diagnostics panel
- CAN frame table (ID, timestamp, DLC, data bytes)
- Gauge-style visualization (speed / RPM)
- Live telemetry readouts

---

## Technologies Used

### Languages & Libraries
- C++17
- SFML (Graphics, Windowing, TCP Networking)
- ImGui + ImGui-SFML

### C++ Concepts Demonstrated
- Multi-threading (`std::thread`, atomics)
- Producer–consumer queue pattern
- Lock-free UI data model
- Binary protocol design
- Endianness-safe serialization
- Separation of concerns
- Real-time data visualization

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

