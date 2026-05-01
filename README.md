# Octree Mesh Preprocessor

An offline preprocessing tool for 3D meshes that constructs an octree spatial partitioning structure to accelerate ray-geometry intersection tests. The tool exports the optimized scene in a custom binary format, enabling significantly faster raycasting without requiring modifications to existing ray tracing engines.

---

## Overview

Loading a complex 3D model and performing ray intersection tests against every face is expensive — on a 500,000 face mesh, a brute force raycast can take over 20,000 μs per ray. This tool preprocesses the mesh into an octree, reducing that to ~54 μs per ray: a **370x speedup**.

The workflow is:
1. Import a 3D model via Assimp
2. Build an octree over the mesh geometry
3. Visualize and inspect the structure interactively
4. Export the octree in a custom binary format for use at runtime

---

## Features

- **Octree construction** — recursive spatial subdivision with configurable max faces per node and max depth
- **Interactive visualization** — view the mesh with octree leaf node boundaries, colored by depth, face density, or random node coloring
- **Benchmark mode** — compare optimized (octree) vs brute force raycasting in real time with live FPS feedback
- **Custom binary export** — exports the octree structure for use in external ray tracing pipelines
- **Assimp-based import** — supports OBJ, FBX, GLTF, STL and other common formats

---

## Benchmark Results

Tested on a 500,000 face mesh over 10,000 rays:

System Specs:
 - Intel Core I7-12700H
 - NVIDIA GeForce RTX 3050 Ti
 - 16 GB DDR5 RAM

| Method | Avg. time per ray | Speedup |
|---|---|---|
| Brute Force | ~20,000 μs | 1x |
| Octree Optimized | ~54 μs | ~370x |

The optimized raycast narrows intersection candidates to a small subset of leaf nodes, testing far less than 0.1% of total geometry per ray.

---

## Dependencies

- [Assimp](https://github.com/assimp/assimp) — model import
- [GLEW](https://glew.sourceforge.net/) — OpenGL extension loading
- [FreeGLUT](https://freeglut.sourceforge.net/) — windowing and input
- [GLM](https://github.com/g-truc/glm) — math library
- [Dear ImGui](https://github.com/ocornut/imgui) — UI and menu bar

---

## Usage

### Import and preprocess
Load any supported 3D model file. The tool currently processes the first mesh in the file — multi-mesh scenes are a known limitation planned for future work.

### Visualization modes
Use the menu bar to switch between coloring modes:
- **Default** — flat lit grey
- **Depth** — color by octree depth (blue → red)
- **Face density** — color by faces per leaf node (blue → red)
- **Random** — unique color per node

Toggle **Show Node Boundaries** to render leaf node bounding boxes. Nodes with fewer than 10 faces are shown in red, denser nodes in green.

### Benchmark mode
- **Start Optimized** — fires a random ray toward the origin every 10ms using octree traversal
- **Start Brute Force** — same rays tested against every face in the mesh
- **Singular Raycast** — fires one ray on demand
- **Stop** — returns to normal rendering

### Export
Exports the octree structure to a custom binary format for use in external applications.

---

## Octree Configuration

Two compile-time constants control tree growth:

```cpp
static int MAX_FACES 10   // faces per node before splitting
static int MAX_DEPTH 10   // maximum tree depth
```

Increasing `MAX_FACES` produces a shallower tree with faster construction but slower raycasts. Decreasing it produces finer subdivision at the cost of memory.

---

## Known Limitations

- Only the first mesh in a multi-mesh file is processed
- Raycasts are performed in model space — the ray origin must account for the mesh's world transform
- The binary export format is custom and requires a compatible importer on the receiving end