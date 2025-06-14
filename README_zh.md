[English](README.md)

![pbf](pbf_1_r.gif)
<p align="center">效果展示1</p>

![pbf](pbf_2_r.gif)
<p align="center">效果展示2</p>

![caustics](caustics.gif)
<p align="center">焦散展示1</p>

![caustcis2](caustics2.gif)
<p align="center">焦散展示2</p>

## 介绍
基于3D位置的实时流体模拟, 参照Miles Macklin与Matthias Müller的论文 *"Position based fluids."* 实现.

基于屏幕空间的实时流体渲染, 参照W. J. van der Laan, S. Green与 M. Sainz的论文 *"Screen space fluid rendering with curvature flow"* 实现.

基于图像空间下的光子映射的实时流体焦散, 部分参照Chris Wyman与Scott Davis的论文 *"Interactive image-space techniques for approximating caustics"* 实现.

## 特点
模拟计算使用OpenGL的Compute Shaders

流体渲染使用点精灵(屏幕空间四边形)

焦聚渲染基于图像空间下的光子映射

用户界面使用imgui, 用于检测各部分性能, 以及实时调整参数与交互

---- 

流体模拟的算法推导见[这里](https://zhuanlan.zhihu.com/p/31850164166)

流体渲染的算法介绍见[这里](https://zhuanlan.zhihu.com/p/1896667370149352055)

流体焦散的算法介绍见[这里](https://zhuanlan.zhihu.com/p/1916917420641199169)