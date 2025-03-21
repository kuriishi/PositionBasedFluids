![pbf](pbf.gif)

A weak, unoptimized CPU version implementation of Position Based Fluids for algorithm verification. Optimization and acceleration will be addressed in a future GPU version using compute shaders or CUDA.

Parallel acceleration is implemented using the std::execution::par_unseq strategy. While MSVC provides its implementation, g++ users may need to install dependency libraries like TBB. See CMakeLists.txt for other dependencies.
