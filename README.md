# CrabbyOpenGL

Welcome to **CrabbyOpenGL** — a personal collection of my OpenGL projects where I explore and implement modern graphics programming concepts using **OpenGL Core 3.3** and **C++**.

This repository serves as a learning sandbox and proof-of-concept lab for various rendering techniques and engine components, ranging from simple lighting to instanced rendering of hundreds of thousands of objects.

---

## 🧠 What’s Inside

This repo includes a mix of **major** and **minor** projects, each focused on a specific OpenGL topic or rendering technique. I'm actively working through foundational and advanced topics alike, so the repo will keep evolving as I learn and experiment.

### Highlights so far:
- ⚙️ **Renderer Architecture**: Custom renderers using shaders, VBOs, VAOs, and textures.
- 🌱 **Instancing**: Rendering 1 million+ blocks efficiently using TBOs.
- 💡 **Lighting**: Ambient, diffuse, and specular lighting with attenuation and spotlight features.
- 🖼️ **Framebuffers & Cubemaps**: Used for off-screen rendering and skyboxes.
- 🌾 **Large-scale Rendering**: 100,000 grass blocks rendered with performance in mind.
- 🧪 **Experiments with UBOs and TBOs** for optimal data transfer to GPU.
- 🔍 **Upcoming Topics**: Shadows, gamma correction, normal mapping, HDR, bloom, PBR, SSAO.

---

## 🗂️ Project Structure

Each project directory contains:
- `main.cpp` – Entry point for the application.
- `headers/OpenGL_Graphics/` – Central header file(s) for shared functionality.
- `shaders/` – GLSL vertex, fragment (and sometimes geometry) shaders.
- `assets/` – Textures, models, and other media files.
- `README.md` – Project-specific notes (coming soon!).

I follow a modular approach where each project is relatively self-contained but may reuse some common abstractions.

---

## 🚧 Work in Progress

This repo is an active WIP (Work In Progress). Expect:
- Code cleanup in future commits
- More project-level documentation
- Demo gifs/screenshots (coming soon)
- Utility tools to assist with debugging, profiling, and scene management

---

## 🛠 Tech Stack

- **Language:** C++
- **Graphics API:** OpenGL 3.3 Core Profile
- **Windowing & Input:** GLFW
- **Loading OpenGL functions:** GLAD
- **Math Library:** GLM
- **Image Loading:** stb_image.h

Other tools like Assimp, ImGui, or Bullet may be added later depending on needs.

---

## 🔥 Why This Exists

I'm a B.Tech ECE student diving deep into OpenGL out of sheer passion. I'm building this as a foundation before stepping into:
- **2D/3D Game Engines**
- **Physics Engines** (rigid body, collisions, etc.)
- **Vulkan**
- **Cloud integrations with game/data workloads**
  
Think of this repo as both a **playground** and a **portfolio**.

---

## 📦 How to Build

You’ll need:
- A C++20-compatible compiler
- CMake 3.10+ (or a simple Makefile)
- GLFW, GLAD, and GLM (already included or fetchable via CMake)

**Basic CMake usage:**
```bash
cd CrabbyOpenGL/YourProject
mkdir build && cd build
cmake ..
make
./YourProjectExecutable
