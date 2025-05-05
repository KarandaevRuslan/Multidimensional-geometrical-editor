# NDEditor
<!-- Formerly “Multidimensional Geometrical Editor” -->

<!-- Badges --------------------------------------------------------------->
[![CI](https://github.com/KarandaevRuslan/Multidimensional-geometrical-editor/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/KarandaevRuslan/Multidimensional-geometrical-editor/actions/workflows/build.yml)
[![License](https://img.shields.io/badge/license-TBD-blue.svg)](#license)

## Overview
**NDEditor** is a cross‑platform desktop application for **creating, editing and exploring high‑dimensional objects**.  
It targets **enthusiasts, students, teachers and researchers** who want to visualise geometry in 3–20 dimensions, change projections (orthographic · stereographic · perspective · none) and watch objects rotate in real time.

Built with **C++17 · CMake · Qt 5/6 · OpenGL** and distributed for **Windows, Linux and macOS** on both **x64** and **ARM64**.

---

## Features
- **Unlimited scenes** open in parallel, with copy / paste of objects between them  
- Object templates: *none · hyper‑cube · simplex · n‑cross‑polytope · permutohedron*  
- Edit vertices, edges or rotation planes; full **undo/redo** (Ctrl + Z / Y or Shift + Z)  
- Adjacency‑matrix edge editor with colour cues (green = edge, red = no edge)  
- Per‑axis scaling, translation and colour settings  
- Scene‑level background colour and projection saved to `config.json`  
- **JSON** import/export of single shapes *and* entire scenes  
- **Free‑flight camera** (Shift + F) & mouse‑look / arrow‑look  
- Dynamic axes with tick labels (toggle Alt + H)  

> **Dimensional limit:** technically ∞, but a soft lock of 20 is set for performance.

---

## Table of Contents
1. [Screenshots](#screenshots)  
2. [Getting Started](#getting-started)  
3. [Usage & Hotkeys](#usage--hotkeys)  
4. [Build from Source](#build-from-source)  
5. [Configuration](#configuration)  
6. [Roadmap](#roadmap)  
7. [Contributing](#contributing)  
8. [License](#license)  
9. [Contact](#contact)  

---

## Screenshots
<!-- TODO: Add PNG/GIF files to /images and reference them here -->
<!-- Example: -->
<!-- <p align="center"><img src="images/teaser.gif" width="720"></p> -->

---

## Getting Started

### Pre‑built binaries
Grab the latest release from the [Releases](../../releases) page.

| OS      | Arch     | Package                        |
|---------|----------|--------------------------------|
| Windows | x86‑64 / ARM64 | `NDEditor-*.zip` with `NDEditor.exe` |
| Linux   | x86‑64 / ARM64 | `NDEditor-*.AppImage` |
| macOS   | Intel / Apple Silicon | `NDEditor.app.tar.gz` |

Unpack, run **NDEditor** and enjoy.

### Quick demo
`File → Example scene` spawns a *tesseract* and *simplex-5d* so you can immediately rotate, recolour and experiment.

---

## Usage & Hotkeys

| Action                        | Shortcut (layout‑independent) |
|-------------------------------|-------------------------------|
| Toggle free‑flight camera     | <kbd>Shift</kbd> + <kbd>F</kbd> |
| Toggle axes & tick labels     | <kbd>Alt</kbd> + <kbd>H</kbd> |
| Copy / Paste                  | <kbd>Ctrl</kbd> + <kbd>C</kbd> / <kbd>Ctrl</kbd> + <kbd>V</kbd> |
| Undo / Redo                   | <kbd>Ctrl</kbd> + <kbd>Z</kbd> / <kbd>Ctrl</kbd> + <kbd>Y</kbd> |
| Delete selection              | <kbd>Del</kbd> |
| Mouse wheel                   | Zoom in / out |
| Right‑mouse + drag            | Look around (standard view) |
| Arrow keys                    | Look around (all platforms) |

All changes are rendered **instantly**; there is no separate “edit vs view” mode.

---

## Build from Source

<details>
<summary><strong>Linux (Qt 5 system packages)</strong></summary>

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build      qtbase5-dev qtchooser qtbase5-dev-tools qtdeclarative5-dev      qtquickcontrols2-5-dev qtwebengine5-dev libglu1-mesa-dev      libopengl-dev libqt5opengl5-dev libqt5x11extras5-dev

git clone https://github.com/KarandaevRuslan/Multidimensional-geometrical-editor.git
cd Multidimensional-geometrical-editor
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target NDEditor
./build/NDEditor
```
</details>

<details>
<summary><strong>Windows (MSVC 2022 + Qt 6)</strong></summary>

```powershell
# install Qt 6.x (or use install-qt-action in CI)
git clone https://github.com/KarandaevRuslan/Multidimensional-geometrical-editor.git
cd Multidimensional-geometrical-editor
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target NDEditor
.\build\Release\NDEditor.exe
```
</details>

> See `.github/workflows/build.yml` for the full multi‑platform configuration (x86‑64 & ARM64 for Windows, Linux, macOS).

---

## Configuration
`config.json` is auto‑generated on first run. Settings you can tweak manually:

| Key                         | Description                               |
|-----------------------------|-------------------------------------------|
| `sceneObjDefaultColor`      | Default RGB hex colour applied to shapes  |
| `sceneOverlayNumberPenColor`| Colour of axis coordinate numbers         |
| `sceneRendererClearColor`   | Background colour of the viewport         |

---

## Roadmap
The following high‑level goals are planned for the next releases.  
(PRs are welcome—check the Issue tracker before starting major work!)

- **Performance & memory optimisations** for scenes with >10k vertices  
- **Custom hotkey bindings** (configurable JSON)  
- **Animation timeline**: key‑frame rotations and camera paths  
- **Additional export formats**: OBJ for 3‑D slices and GLTF with animation  
- **Plugin API** for procedural shape generators (C++ / Python)  
- **Localization** using Qt `.ts` files (🇷🇺 Russian first, community translations later)  
- **VR / AR preview** (OpenXR) for immersive 4‑D exploration  
- **Continuous integration of clang‑tidy & static‑analysis** in CI workflow 

---

## Contributing
Contributions are **very welcome**!  
Please follow these general guidelines:

1. **Fork** the repo & create a feature branch:  
   ```bash
   git checkout -b feature/my-awesome-idea
   ```
2. Keep commits small and descriptive (Conventional Commits style appreciated).
3. Run / extend **Google Tests** (`build/Test`).
4. Open a **Pull Request** to `main`.  
   CI must pass before review.

For bug reports or feature requests open a GitHub Issue or write an email.

---

## License
This project is licensed under the **MIT License**—see [`LICENSE`](LICENSE) for details.  
MIT is short, permissive and widely understood, making it easy for anyone to use the code (including in commercial or academic projects) while still requiring attribution.

---

## Contact
**Ruslan Karandaev**  
- Email: <karandaevruslan@gmail.com>  
- GitHub Issues: <https://github.com/KarandaevRuslan/Multidimensional-geometrical-editor/issues>  
- SimpleX Chat / Session ID: <!-- TODO -->

---

*Made with ❤️, C++ 17 and OpenGL shaders.*
