[简体中文](README_zh.md)

![pbf_32k](pbf_1_r.gif)
<p align="center">showcase 1</p>

![pbf_32k_blend](pbf_2_r.gif)
<p align="center">showcase 2</p>

![caustics](caustics.gif)
<p align="center">caustics showcase 1</p>

![caustcis2](caustics2.gif)
<p align="center">caustics showcase 2</p>

## Introduction
Real-time fluid simulation based on 3D position, implemented with reference to the paper *Position based fluids* by Miles Macklin and Matthias Müller.

Real-time screen-space fluid rendering, implemented with reference to the paper *Screen space fluid rendering with curvature flow* by W. J. van der Laan, S. Green, and M. Sainz.

Real-Time fluid caustics via image-space photon mapping, partially referencing the paper *Interactive image-space techniques for approximating caustics* by Chris Wyman and Scott Davis. 

## Features
Simulation powered by OpenGL Compute Shaders

Fluid rendering using point sprites (screen oriented quads)

Caustics rendering via image-space photon mapping

ImGui-based user interface for performance monitoring, parameter tuning, and real-time interaction

---- 

Fluid simulation algorithm derivation can be found [here](https://zhuanlan.zhihu.com/p/31850164166)

Fluid rendering algorithm introduction can be found [here](https://zhuanlan.zhihu.com/p/1896667370149352055)

Fluid caustics algorithm introduction can be found [here](https://zhuanlan.zhihu.com/p/1916917420641199169)