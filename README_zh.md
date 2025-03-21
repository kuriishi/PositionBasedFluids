![pbf](pbf.gif)

一个较弱的、未经优化的CPU版本实现的Position Based Fluids，用以验证算法。优化和加速的工作之后将使用compute shader或者cuda在GPU版本中进行。

使用std::execution::par_unseq策略进行并行加速，MSVC提供了其实现，若使用g++，可能需要安装依赖库如TBB。其余依赖见CMakeLists.txt。
