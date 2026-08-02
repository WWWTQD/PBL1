# PBL1 – Maximum Flow on Network G

![C++](https://img.shields.io/badge/C++-17-blueviolet)
![Platform](https://img.shields.io/badge/Platform-Desktop-green)
![Status](https://img.shields.io/badge/Status-Completed-yellow)

🧮 A computational programming project that visually simulates algorithms for finding the maximum flow in a flow network, focusing on algorithmic accuracy and graphical visualization.

---

## 🚀 Introduction

PBL1 is a group project developed to build a complete program for **finding the maximum flow on network G**.

The project focuses on:

* High-performance graph traversal and pathfinding
* Interactive graphical user interface using SDL2
* Step-by-step visual execution of flow algorithms

The project is developed by a **2-member team**, including:
- Trương Quang Đạt (102240304)
- Nguyễn Hải Long (102240318)

**Instructor:** Ts. Phạm Công Thắng

---

## ✨ Main Features

* 🖱️ **Interactive Graph Creation:** Add nodes, sources, sinks, and connect directed edges via mouse.
* ⌨️ **Dynamic Capacity Input:** Assign capacities to edges directly using the keyboard.
* ⚙️ **Multiple Algorithms Supported:**
    * Ford-Fulkerson
    * Edmonds-Karp
    * Dinic's Algorithm
* 🎨 **Visual Simulation:** Color-coded edges to distinguish forward/backward edges and visualize flow updates step-by-step.
* 🔄 **Real-time Reset & Navigation:** Restart simulations or switch algorithms without exiting the application.

---

## 🧱 Technologies Used

### Core

* **Language:** C (Standard C23)
* **Graphics Library:** SDL2, SDL2_ttf
* **Concepts:** Graph Theory, BFS/DFS, Dynamic Memory

### Tools

* **IDE:** CLion
* **Build Tool:** CMake
* **Compiler:** GCC (MinGW-w64)
* **Version Control:** Git & GitHub

## 💻 How to Run

General requirements:
- **MinGW GCC Compiler**
- **CMake** (v3.30 or later)
- **SDL2 & SDL2_ttf Libraries** (Included in the project directory)

### 1. Run using CLion (Recommended)

Open the project folder in CLion and let CMake load the project.

```bash
# Reload CMake Project to update library paths
# Click the 'Run' button (Shift + F10) targeting PBL1_PROJECT
```

The graphical window will launch automatically.

### 2. Run using Terminal (CMake)

Open your Terminal at the project's root directory:

```bash
mkdir cmake-build-debug
cd cmake-build-debug
cmake ..
cmake --build .
```

After the build completes, run the generated `.exe` file.

---

## 📸 Demo / Screenshot

![Interface Preview](demo.png)
> Graphical interface for algorithm selection and network building.

---

## 🤝 Contributing

This project is intended for educational purposes as part of the university curriculum.

Any suggestions, feedback, or contributions are greatly appreciated.