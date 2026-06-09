# 🌌 Solar System Ultimate: Interactive 3D Simulation

An interactive, real-time 3D simulation of the Solar System built from scratch using **C++** and modern **OpenGL**. This project bridges the gap between orbital physics and computer graphics, offering a seamless and educational space exploration experience.

> **Note:** This project is currently in active development. I am actively transitioning it from an educational simulation into a full-fledged space exploration game! 🚀

---

## ✨ Current Features

* **Accurate Orbital Mechanics:** Implements visual logarithmic scaling and variable orbital speeds inspired by Kepler's laws.
* **Advanced Earth Shading:** Utilizes multi-texturing for dynamic Day/Night cycles based on light direction, plus alpha blending for independent cloud layers.
* **Procedural Environments:** Real-time generation of over 1,700 unique asteroids for the Main Asteroid Belt and Kuiper Belt using procedural geometry and randomized scaling.
* **Hierarchical Transformations:** Accurate parent-child matrix inheritance (e.g., the Moon orbiting Earth while Earth orbits the Sun).
* **Interactive Command Center:** A lightweight, Immediate-Mode GUI built with **Dear ImGui** to control simulation speed, toggle orbital paths, and trigger auto-landing sequences.
* **High Performance:** Optimized render loop utilizing face culling, depth testing, and double buffering. Runs flawlessly at solid 60+ FPS (Tested smoothly on an RTX 4050 GPU and SSD storage).

---

## 🛠️ Tech Stack & Libraries

* **Language:** C++
* **Graphics API:** OpenGL (GLFW & GLAD)
* **Math Library:** GLM (OpenGL Mathematics)
* **UI Framework:** Dear ImGui
* **Texture Loading:** stb_image

---

## 🗺️ Development Roadmap (Upcoming Features)

I am taking this project to the next level for upcoming competitions. Here is what is currently cooking in the development pipeline:

- [ ] **Spaceship Integration:** Importing true 3D models using `Assimp` to replace the free-fly camera with a controllable spacecraft.
- [ ] **Next-Gen Graphics (PBR):** Upgrading from Phong lighting to Physically Based Rendering for realistic metallic and rough surfaces.
- [ ] **Post-Processing Pipeline:** Adding Bloom for sun glare, HDR tone mapping, and atmospheric scattering for planets.
- [ ] **Audio Engine:** Integrating spatial 3D audio for thrusters, ambient space music, and proximity UI alerts.
- [ ] **Gamification:** Implementing spaceship physics (Newtonian gravity), fuel management, and objective-based missions.

---


## 👨‍💻 Developer

**Developed by Ahmed Magdy** *Computer Science and Information Systems Student at the Egyptian Chinese University (ECU).* Passionate about graphics programming, AI, and building high-performance applications.

---

## 📸 Screenshots
<img width="2559" height="1560" alt="Screenshot 2026-06-09 065033" src="https://github.com/user-attachments/assets/d807ec31-2d74-4ed4-8a33-2d937436bdcb" />
<img width="2560" height="1600" alt="Screenshot 2026-05-06 000542" src="https://github.com/user-attachments/assets/740ab868-ce05-4f03-88b7-b4675837e896" />
<img width="2560" height="1600" alt="Screenshot 2026-05-06 000653" src="https://github.com/user-attachments/assets/2bda8ef5-5e25-43db-96db-3f57ce3c6bef" />
