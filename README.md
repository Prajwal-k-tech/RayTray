RayTray

Built this while going through Ray Tracing in 1 weekend and then parallelizing it for my Parallel computing course.

- `main.cpp` - serial ray tracer (RTIOW Ch 1-8: vec3, ray, sphere, camera, diffuse/metal/dielectric, antialiasing)
- `main_parallel.cpp` - OpenMP parallel version (`#pragma omp parallel for schedule(dynamic)` with thread-local RNG)
- `images/` - roadmap renders from gradient to final scene

Build:
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/raytracer        # serial -> serial_output.ppm
./build/raytracer_parallel # parallel -> image_t*.ppm + benchmark_results.csv
```
