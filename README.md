# SPH - Smoothed Particle Hydrodynamics Fluid Simulator

A real-time 3D fluid dynamics simulation using **Smoothed Particle Hydrodynamics (SPH)**.

## Quick Start

### Prerequisites
- **CMake 3.20+**
- **C++23 compiler** (Clang 18+ on macOS/Linux, MSVC 19.35+ on Windows)
- **OpenGL 3.3+**
- **GLFW3**
- GLAD (included)
- ImGui (included)

### Installation & Build

#### macOS 
```bash
brew install cmake glfw

git clone <repository-url>
cd SPH
mkdir build && cd build
cmake ..
cmake --build .

# Run
./NES
```

#### Linux 
```bash
sudo apt-get update
sudo apt-get install cmake libglfw3-dev libglew-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev

git clone <repository-url>
cd SPH
mkdir build && cd build
cmake ..
cmake --build .

./NES
```

#### Windows
```bash
git clone <repository-url>
cd SPH
mkdir build && cd build
cmake ..
cmake --build . --config Release

./Release/NES.exe
```

## Controls

### Camera Movement
Use **keyboard** to navigate the 3D view:

| Key | Action |
|-----|--------|
| **W** | Move forward |
| **S** | Move backward |
| **A** | Move left |
| **D** | Move right |
| **Space** | Move up |
| **Shift** | Move down |

### Camera Rotation & Zoom
| Key | Action |
|-----|--------|
| **Q** | Rotate left (yaw) |
| **E** | Rotate right (yaw) |
| **↑ Arrow** | Rotate up (pitch) |
| **↓ Arrow** | Rotate down (pitch) |
| **+/=** | Zoom in |
| **-** | Zoom out |

## Simulation Parameters

The **SPH Config** panel (top-left corner) displays sliders.
I would suggest playing around the box bounds sliders to create movement in the fluid.
The box spawns small so it is best to make it larger to begin.
Some of hte sliders will cause the simulation to explode if moved too far in either direction. I have not placed greate limits on them yet.
If the simulation does explode, it can be best to just restart it.
If the simulation is too heavy for your computer adjust the first parameter (num particles) of the function call on line 113 of Renderer.cpp.

## Features

- **SPH simulation** with thousands of particles
- **Multi-threaded execution** for parallel density/pressure/viscosity calculations (needs to be revised)
- **Spatial hashing** for efficient neighbor lookup
- **Boundary collisions** with customizable damping
- **Live parameter adjustment** via ImGui sliders
- **FPS counter** showing performance metrics

## Architecture

### Core Components

| Component | Purpose |
|-----------|---------|
| **SPH** | Main simulation engine, owns particles, manages all physics calculations |
| **Particle** | Individual fluid element with position, velocity, density, pressure |
| **Renderer** | OpenGL rendering of particles and container bounds |
| **Camera** | Keyboard-controlled 3D navigation |
| **ImGuiManager** | Real-time UI for parameter adjustment |
| **Window** | GLFW window management and event loop |

