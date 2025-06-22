[English](README.md)

---

# 特点

- 基于3D位置的实时流体模拟
- 屏幕空间下的实时流体渲染
- 基于光子映射的实时水体焦散
- 卡通风格水体与浮沫的实时渲染
- 使用ImGui的简单用户交互界面
- 计算负载放置于Compute Shader

# 基于3D位置的实时流体模拟

参照Miles Macklin与Matthias Müller的论文 *"Position based fluids."* 实现.

推导过程见[这里](https://zhuanlan.zhihu.com/p/31850164166)

![pbf_1](showcase/pbf_1.gif)

<p align="center"> 流体模拟 效果展示1 </p>

![pbf_2](showcase/pbf_2.gif)

<p align="center"> 流体模拟 效果展示2 </p>

# 屏幕空间下的实时流体渲染

参照W. J. van der Laan, S. Green与 M. Sainz的论文 *"Screen space fluid rendering with curvature flow"* 实现.

算法介绍见[这里](https://zhuanlan.zhihu.com/p/1896667370149352055)

![fluid_1](showcase/fluid_1.gif)

<p align="center"> 流体渲染 效果展示1 </p>

![fluid_2](showcase/fluid_2.gif)

<p align="center"> 流体渲染 效果展示2 </p>

# 基于光子映射的实时水体焦散

部分参照Chris Wyman与Scott Davis的论文 *"Interactive image-space techniques for approximating caustics"* 实现.

算法介绍见[这里](https://zhuanlan.zhihu.com/p/1916917420641199169)

![caustics_1](showcase/caustics_1.gif)

<p align="center"> 流体焦散 效果展示1 </p>

![caustics_2](showcase/caustics_2.gif)

<p align="center"> 流体焦散 效果展示2 </p>

# 卡通风格水体与浮沫的实时渲染 

参照Liordino dos S. Rocha Neto的论文*"Real-Time Screen Space Rendering of Cartoon Water"* 与 *"Cartoon Water Rendering with Foam and Surface Smoothing"* 实现. 

算法介绍见[这里](https://zhuanlan.zhihu.com/p/1920252664308040967)

![cartoon_1](showcase/cartoon_1.gif)

<p align="center"> 卡通风格水体与浮沫 效果展示1 </p>

![cartoon_2](showcase/cartoon_2.gif)

<p align="center"> 卡通风格水体与浮沫 效果展示2 </p>
