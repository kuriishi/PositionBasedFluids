![pbf_32k](pbf_32k_1.gif)
<p align="center">32k Particles Showcase 1</p>

![pbf_32k_blend](pbf_32k_2.gif)
<p align="center">32k Particles Showcase 2</p>

## Introduction
A position-based fluid simulation based on 3D spatial positioning, implemented with reference to the paper "Position Based Fluids" by Miles Macklin and Matthias Müller.

Rendering utilizes OpenGL point sprites, while simulation computations are performed using OpenGL Compute Shaders.

## Performance
CPU: AMD 8825HS

GPU: AMD 780M 

With 4 constraint projection iterations, the simulation achieves an average of ~35 FPS in scenarios containing 32k particles.

---- 

Algorithm derivation details can be found [here](https://zhuanlan.zhihu.com/p/31850164166).
