# Brick Collision Simulation

Original project from CS 330: Computational Graphics and Visualization at Southern New Hampshire University (Module Eight, Coding Collisions). This version is the starting point for my CS 499 Capstone algorithms and data structures enhancement.

## Overview

A 2D C++ program built with OpenGL and GLFW. Four rows of eight bricks are drawn across the top of the window. Two random bricks in each row are destructible and the rest are reflective. Circles launched with the spacebar move around the window, bounce off reflective bricks, and destroy destructible ones.

## Controls

- **Space**: launch a circle (holding the key adds one every frame)
- **Esc**: close the window

## Project structure

```
Projects/8-2_Assignment/Source/MainCode.cpp   all program logic
Projects/8-2_Assignment/8-2_Assignment.vcxproj   Visual Studio project
Utilities/   shared headers from the course
```

## Requirements

- Windows with Visual Studio 2022 (C++ desktop development workload)
- The CS 330 course `Libraries` folder containing GLFW, GLEW, and GLM

## Setup

The project file looks for libraries at `..\..\Libraries` relative to the project, so the folder layout needs to be:

```
coding_collisions/
  Libraries/
    GLEW/
    GLFW/
    glm/
  Projects/
  Utilities/
```

1. Copy the `Libraries` folder from the CS 330 course content into the repository root, next to `Projects` and `Utilities`.
2. Open `Projects/8-2_Assignment/8-2_Assignment.vcxproj` in Visual Studio.
3. Set the platform to **Win32** (x86) and build.
4. Copy `glew32.dll` from `Libraries/GLEW/bin/Release/Win32` into the folder with the built `.exe` if Windows reports it missing.
5. Run the program and press Space to launch circles.

## Known limitations

These are planned for the CS 499 enhancement:

- Every frame, each circle is checked against every brick, which is O(n x m) and does not scale as circles are added.
- The collision test treats circles as squares, so a circle can register a hit on a brick corner it never touches.
- Holding Space adds a circle every frame with no limit.
- Movement speed is applied per frame instead of per unit of time, so speed depends on the monitor's refresh rate.