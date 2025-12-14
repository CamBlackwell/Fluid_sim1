# 🌊 Real-Time Fluid Simulator

A 2D particle-based fluid simulation built in C++ using SFML, featuring multi-threaded collision detection and metaball rendering for realistic fluid visuals.
Mainly just cpu rendering, but still cool imo! 


![main_sim_video](https://github.com/user-attachments/assets/61c56da9-9ec5-4540-831c-c294db71e28a) 
![Fluid_sim_video](https://github.com/user-attachments/assets/4f56af39-f73b-4346-84b9-d8e0839556d9)

(excuse the choppy gifs, running the sim and obs killed my computer lol its smoother irl)



## ✨ Features

### Core Simulation
- **Real-time particle physics** with accurate collision detection and response
- **Multi-threaded collision detection** utilizing all CPU cores for optimal performance
- **Spatial grid optimization** for efficient neighbor searching (O(n) instead of O(n²))
- **Interactive gravity controls** - Use arrow keys to apply directional forces
- **Energy dampening** on wall collisions for realistic behavior

### Visual Effects
- **Metaball shader rendering** - Particles blend together to create smooth, organic fluid surfaces
- **Dynamic color gradients** based on particle density and intensity
- **Customizable visual parameters** - Adjust threshold, radius, and colors
- **Real-time rendering** at 60 FPS with up to 2000+ particles

### Performance
- Handles **2000+ particles** at 60 FPS
- **Multi-threaded architecture** scales with available CPU cores
- Optimized spatial partitioning reduces collision checks by ~95%
- Efficient GLSL shader for metaball calculations



## 🚀 Getting Started

### Prerequisites

- C++17 compatible compiler (GCC, Clang, or MSVC)
- SFML 3.0 or higher
- CMake (optional, for building)

### Installation

#### Windows (MSYS2)

```bash
# Install MSYS2 from https://www.msys2.org/

# Install dependencies
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-ucrt-x86_64-sfml

# Clone the repository
git clone https://github.com/yourusername/fluid-simulator.git
cd fluid-simulator

# Compile
g++ main.cpp -o fluid_sim.exe -lsfml-graphics -lsfml-window -lsfml-system -std=c++17 -O3

# For metaball version
g++ metaball.cpp -o metaball_sim.exe -lsfml-graphics -lsfml-window -lsfml-system -std=c++17 -O3
```

#### Linux

```bash
# Install SFML
sudo apt-get install libsfml-dev

# Clone and build
git clone https://github.com/yourusername/fluid-simulator.git
cd fluid-simulator
g++ main.cpp -o fluid_sim -lsfml-graphics -lsfml-window -lsfml-system -std=c++17 -O3 -pthread
```

#### macOS

```bash
# Install SFML via Homebrew
brew install sfml

# Clone and build
git clone https://github.com/yourusername/fluid-simulator.git
cd fluid-simulator
g++ main.cpp -o fluid_sim -lsfml-graphics -lsfml-window -lsfml-system -std=c++17 -O3
```

## 🎮 Usage

### Basic Simulation
```bash
./fluid_sim
```

### Metaball Fluid Rendering
```bash
./metaball_sim
```

### Controls
- **Arrow Keys** - Apply directional gravity forces
- **ESC** - Exit simulation
- **Resize Window** - Simulation adapts to new dimensions

## Technical Architecture

### Spatial Partitioning
The simulation uses a grid-based spatial hash to partition space into cells. Each particle is assigned to a cell based on its position, dramatically reducing collision detection complexity:

```
Without optimization: O(n²) - Check every particle against every other
With grid: O(n) - Only check particles in neighboring cells
```

### Multi-threading Strategy
Collision detection is parallelized across CPU cores:

```cpp
// Work is divided among threads
Thread 1: Process particles 0-512
Thread 2: Process particles 513-1024
Thread 3: Process particles 1025-1536
...
```

Each thread handles collision detection for its subset of particles independently, with careful synchronization to prevent race conditions.

### Metaball Rendering
The metaball shader creates fluid-like visuals using a field equation:

```glsl
// For each pixel, sum contributions from all particles
sum = Σ(radius² / distance²)

// If sum exceeds threshold, pixel is part of fluid
if (sum > threshold) {
    render_pixel();
}
```

This creates smooth, organic blending between particles.

## ⚙️ Configuration

Adjust these parameters in the code to customize behavior:

### Physics
```cpp
float GRAVITY = 1.5f;              // Force strength
float wall_absoribtion = 0.8f;     // Energy loss on collision (0-1)
int num_particles = 2050;          // Particle count
```

### Rendering
```cpp
// Metaball shader
float threshold = 1.5f;            // Fluid surface threshold
float radius = 7.0f;               // Particle influence radius
vec3 colour1 = vec3(0.0, 0.502, 0.502);  // Light blue
vec3 colour2 = vec3(0.0, 0.2, 1.0);      // Dark blue
```

### Performance
```cpp
const int CELL_SIZE = 100;         // Grid cell size (pixels)
const int NUM_THREADS;             // Auto-detected CPU cores
```

## 📊 Performance Benchmarks

| Particles | FPS (Single-threaded) | FPS (Multi-threaded 8 cores) |
|-----------|----------------------|------------------------------|
| 500       | 60                   | 60                           |
| 1000      | 45                   | 60                           |
| 2000      | 18                   | 58                           |
| 4000      | 5                    | 32                           |

*Tested on: Intel i7-9700K @ 3.6GHz, 16GB RAM, GTX 1660*

## 🛠️ Future Improvements

- [ ] Add pressure-based fluid dynamics (SPH - Smoothed Particle Hydrodynamics)
- [ ] Implement viscosity simulation
- [ ] Add particle lifetime and spawning system
- [ ] Export simulation as video
- [ ] GPU acceleration using compute shaders
- [ ] 3D visualization option
- [ ] Save/load simulation states

## 📚 Technical Details

### Collision Detection Algorithm
1. **Broad Phase**: Spatial grid divides space into cells
2. **Narrow Phase**: Only test particles within neighboring cells
3. **Response**: Elastic collision with impulse-based velocity updates

### Threading Model
- Main thread: Input handling, rendering, grid construction
- Worker threads: Parallel collision detection
- Synchronization: Join barriers between physics steps

## 🤝 Contributing

Contributions are welcome! Feel free to:
- Report bugs
- Suggest features
- Submit pull requests

---

⭐ Star this repo if you found it interesting!
