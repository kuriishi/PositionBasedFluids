![pbf_32k](pbf_32k_1.gif)
<p align="center">32k粒子展示1</p>

![pbf_32k_blend](pbf_32k_2.gif)
<p align="center">32k粒子展示2</p>

## 介绍
基于3D位置的流体模拟，参照Miles Macklin与Matthias Müller的论文 *"Position based fluids."* 实现。

渲染使用OpenGL点精灵，模拟计算使用OpenGL Compute Shader。

## 性能
CPU: AMD 8845HS

GPU: AMD 780M 

4次约束投影迭代，在32k粒子的场景下，平均约35FPS

---- 

算法推导见[这里](https://zhuanlan.zhihu.com/p/31850164166)
