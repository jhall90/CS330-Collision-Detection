# Brick Collision Simulation

CS 330: Computational Graphics and Visualization (Module Eight, Coding Collisions) at Southern New Hampshire University, enhanced for the CS 499 Capstone in the algorithms and data structures category.

The original version is preserved under the [`v1.0-original`](../../tree/v1.0-original) tag.

## Overview

A 2D C++ program built with OpenGL and GLFW. Rows of bricks are drawn across the top of the window. Two random bricks in each row are destructible and the rest are reflective. Circles bounce off walls and reflective bricks and destroy destructible ones.

## Enhancements

**Uniform grid spatial partitioning.** The original checked every circle against every brick each frame, which is O(n x m). The enhanced version divides the world into a uniform grid of 0.1 unit cells stored as a flat vector. Each brick is inserted into every cell it overlaps when the bricks are built and is removed when destroyed. Each frame, a circle only checks the bricks in the cells its own bounding box overlaps. A per-query stamp keeps a brick that spans several cells from being checked twice. The grid lives in `UniformGrid.h`.

**Exact collision test.** The original treated each circle as a square, so circles could register hits on brick corners they never touched. The enhanced version uses the closest-point method: it clamps the circle's center to the brick to find the nearest point, then compares squared distances so no square root is needed.

**Realistic reflections.** Circles bounce at a mirrored 45 degree angle off walls and bricks. The face that was hit is found from the smaller overlap axis, and the circle is pushed back outside the brick so it cannot get stuck.

**Bug fixes from the code review.** Circle colors used integer division and were limited to about eight colors. The aspect ratio was computed but never used, so circles were drawn as ovals. Circles could slide along walls because a wall hit picked a fully random new direction.

**Benchmark instrumentation.** Collision checks per frame and average collision time are printed to the console every 120 frames, and the G key switches between brute force and the grid in the same build.

## Controls

| Key | Action |
|---|---|
| Space | Launch circles from the bottom center (holding adds one per frame) |
| 1 / 2 / 3 | Spawn 100 / 1,000 / 5,000 circles at random positions |
| C | Clear all circles |
| R | Rebuild the bricks with a new random layout |
| G | Toggle between brute force and the uniform grid |
| D | Toggle between the original 32-brick layout and a dense 512-brick layout |
| Esc | Close the window |

## Benchmark results

Release build, Win32. Average collision time per frame, measured with `std::chrono` around the collision pass only.

**Original layout (32 bricks)**

| Circles | Brute force checks | Grid checks | Brute force time | Grid time | Speedup |
|---|---|---|---|---|---|
| 100 | 3,200 | ~56 | 0.066 ms | 0.009 ms | ~7x |
| 1,000 | 32,000 | ~594 | 0.572 ms | 0.054 ms | ~10x |
| 5,000 | 160,000 | ~2,610 | 3.02 ms | 0.234 ms | ~13x |

**Dense layout (512 bricks)**

| Circles | Brute force checks | Grid checks | Brute force time | Grid time | Speedup |
|---|---|---|---|---|---|
| 100 | 51,200 | ~428 | 0.965 ms | 0.026 ms | ~37x |
| 1,000 | 512,000 | ~4,170 | 9.31 ms | 0.151 ms | ~62x |
| 5,000 | 2,560,000 | ~20,900 | 47.8 ms | 0.727 ms | ~66x |

At 60 FPS a frame has about 16.7 ms. Brute force on the dense layout at 5,000 circles spends 47.8 ms on collisions alone, while the grid does the same work in under 1 ms.

## Project structure

```
Projects/8-2_Assignment/Source/MainCode.cpp     program logic, input, and benchmark
Projects/8-2_Assignment/Source/UniformGrid.h    uniform grid data structure
Projects/8-2_Assignment/8-2_Assignment.vcxproj  Visual Studio project
Utilities/                                      shared headers from the course
```

## Requirements and setup

- Windows with Visual Studio 2022 (C++ desktop development workload)
- The CS 330 course `Libraries` folder containing GLFW, GLEW, and GLM

1. Copy the `Libraries` folder from the CS 330 course content into the repository root, next to `Projects` and `Utilities`. It is listed in `.gitignore` and is not part of this repository.
2. Open `Projects/8-2_Assignment/8-2_Assignment.sln` in Visual Studio.
3. Select **Release** and **Win32 (x86)**, then build and run with Ctrl+F5.
4. If Windows reports `glew32.dll` missing, copy it from `Libraries/GLEW/bin/Release/Win32` into the folder with the built `.exe`.

## Known limitations

- Holding Space adds a circle every frame with no limit.
- Movement is applied per frame instead of per unit of time, so speed depends on the monitor's refresh rate.
- Circles do not collide with each other.
- The grid uses one cell size (0.1) for both layouts. A smaller cell would suit the dense layout's smaller circles, which is a tuning trade-off left for future work.