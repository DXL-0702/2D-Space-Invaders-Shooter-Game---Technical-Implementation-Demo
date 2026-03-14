#  Invader: High-Performance Space Invaders in C++

<div align="center">

![C++](https://img.shields.io/badge/C%2B%2B-17-blue?logo=cplusplus&logoColor=white)
![OpenGL](https://img.shields.io/badge/OpenGL-3.2-green?logo=opengl&logoColor=white)
![GLFW](https://img.shields.io/badge/GLFW-3.3-orange?logo=glfw&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-macOS-lightgrey)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

> 一个极致优化的 **C++17 + OpenGL** 太空射击游戏。
> **零堆分配** · **CPU 软光栅渲染** · **60 FPS 稳定运行** · **无外部引擎依赖**

<br>

<!-- 游戏截图区域：居中、带阴影、圆角 -->
<div style="max-width: 800px; margin: 0 auto;">
  <img src="./Show%20Space%20Invaders.png" alt="Space Invaders Gameplay" style="width: 100%; border-radius: 8px; box-shadow: 0 4px 15px rgba(0,0,0,0.5);"/>
  <p style="margin-top: 10px; color: #888; font-size: 0.9em;">
    <em>🎮 实机演示：躲避弹幕、击落敌机、挑战 Boss</em>
  </p>
</div>

<br>

</div>

---

## 📑 目录 (Table of Contents)

- [🌟 项目简介](#-项目简介)
- [🛠️ 技术栈](#️-技术栈)
- [🏗️ 系统架构](#️-系统架构)
  - [渲染管线：CPU 软光栅 + GPU 纹理](#渲染管线-cpu-软光栅--gpu-纹理)
  - [模块依赖图](#模块依赖图)
  - [游戏状态机](#游戏状态机)
- [🎮 游戏特性](#-游戏特性)
  - [敌机 AI 与弹幕系统](#敌机-ai-与弹幕系统)
  - [武器与道具](#武器与道具)
  - [关卡设计](#关卡设计)
- [⚙️ 性能优化](#️-性能优化)
- [🚀 快速开始](#-快速开始)
  - [环境依赖](#环境依赖)
  - [编译与运行](#编译与运行)
- [🎹 操作指南](#-操作指南)
- [🤝 贡献指南](#-贡献指南)
- [📄 许可证](#-许可证)

---

## 🌟 项目简介

**Invader** 是一个从零开始编写的经典太空射击游戏复刻版。它不仅仅是一个游戏，更是一个**计算机图形学与高性能系统编程的教学案例**。

本项目摒弃了现代游戏引擎沉重的开销，回归底层，直接使用 **OpenGL Core Profile** 进行渲染，并在 CPU 端实现了完整的**软件光栅化 (Software Rasterization)** 逻辑。所有游戏对象（玩家、敌机、子弹、粒子）的像素计算均在 CPU 完成，每帧仅通过一次纹理上传 (`glTexSubImage2D`) 和一次全屏三角形绘制 (`glDrawArrays`) 呈现到屏幕。

**核心亮点：**
- ✅ **极致性能**：全程无动态内存分配 (`malloc/new`)，杜绝 GC 停顿和内存碎片。
- ✅ **架构清晰**：严格的数据驱动设计，逻辑与渲染分离。
- ✅ **教育价值**：完整展示了游戏循环、碰撞检测、状态机、粒子系统等核心概念。

---

## 🛠️ 技术栈

| 类别 | 技术选型 | 说明 |
| :--- | :--- | :--- |
| **语言标准** | ![C++17](https://img.shields.io/badge/C%2B%2B-17-blue) | 利用 `std::optional`, `structured bindings` 等现代特性 |
| **图形 API** | OpenGL 3.2 Core | 仅使用纹理采样和全屏三角形，无固定管线 |
| **窗口管理** | GLFW 3.3 | 轻量级窗口与输入处理 |
| **构建工具** | GCC / Clang | 原生 `g++` 编译，无需 CMake |
| **内存模型** | Static Arrays | 全局定长数组池，O(1) 分配/删除 |

---

## 🏗️ 系统架构

### 渲染管线：CPU 软光栅 + GPU 纹理

这是本项目的灵魂所在。我们**不使用** `glBegin/glEnd`，**不使用** VBO/VAO 存储几何体，**不使用** 矩阵变换。

1.  **CPU 端 (Logic & Rasterization)**:
    *   维护一个 `uint32_t buffer[640][480]` 作为帧缓冲区。
    *   遍历所有游戏对象，根据数学公式直接计算像素颜色并写入 `buffer`。
    *   支持混合、透明度、像素抖动等特效。
2.  **GPU 端 (Display)**:
    *   将 `buffer` 整体上传至 GPU 纹理 (`GL_TEXTURE_2D`)。
    *   绘制一个覆盖全屏的三角形，片段着色器仅做简单的 `texture()` 采样。

```mermaid
graph LR
    A[Game Logic] -->|Write Pixels| B(CPU Frame Buffer)
    B -->|glTexSubImage2D| C{GPU Texture}
    C -->|Sample| D[Full Screen Triangle]
    D --> E[Monitor]