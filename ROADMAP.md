# Ray Tracing + OpenMP — 2-Day Assignment Roadmap

> Goal: Learn RTIOW properly, build serial version, parallelize with OpenMP yourself.
> Deadline context: Review 07/09, Submit 08/09. You have ~16 focused hours.

Yes, you're right — there are essentially **2 references you need**:

1. **RTIOW (serial / learning)** — the book you follow to learn + build serial code
2. **Parallel reference repo (parallel / strategy)** — you read to figure out how to parallelize what you built

Everything else below is backup / viva ammo / report template.

---

## 1. Core References (the only 2 that matter)

### R1 — Serial / Learning: Ray Tracing in One Weekend
- Book: https://raytracing.github.io/books/RayTracingInOneWeekend.html
- GitHub: https://github.com/RayTracing/raytracing.github.io
- **Use for:** MODULE-I (objective, algorithm, pseudocode, serial C++ code)
- **Scope cut — DO ONLY Ch 1–8:**
  - Ch 1-5: vec3, ray, sphere intersection, normals, camera
  - Ch 6-8: antialiasing (samples-per-pixel), diffuse materials, gamma
  - SKIP: metals, dielectrics/glass, final scene, defocus blur
  - Why: Ch 1-8 already looks impressive + has perfect OpenMP parallelism. Full book won't fit in 2 days.

### R2 — Parallel / Strategy: My pick + your options

> **MY PICK FOR YOU: guyleaf as primary, SantiagoDt for the report format.**
> Reason: guyleaf has the cleanest `serial/` vs `openmp/` diff — you can see in 30 seconds exactly what to add to your RTIOW code. SantiagoDt's `report.pdf` is what you copy for Q10 tables/graphs. Use both, but code from guyleaf's pattern.

| Repo (entire point = RTIOW + OpenMP) | What it is | Use it for |
|---|---|---|
| **guyleaf/Parallelized-Ray-Tracing-In-One-Weekend** https://github.com/guyleaf/Parallelized-Ray-Tracing-In-One-Weekend — **PICK** | Serial + OpenMP + CUDA side-by-side + benchmarks | See the exact OpenMP diff to apply to your code |
| **SantiagoDt/Raytracing-parallel-performance** https://github.com/SantiagoDt/Raytracing-parallel-performance — **PICK for report** | Serial + 5 OpenMP strategies (rows/cols/blocks/dynamic) + full report.pdf | Copy table/graph format for time analysis |
| usatie/csc746-tp-ray-tracing-omp https://github.com/usatie/csc746-tp-ray-tracing-omp | Actual Parallel Computing course project (CSC746) doing RTIOW+OMP | Cite in viva: "this is a standard PDC project" |
| ChristianLagares/RayTracingInOneWeekend https://github.com/ChristianLagares/RayTracingInOneWeekend | RTIOW with OpenMP (CPU) + CUDA (GPU) + SIMD | Backup if guyleaf doesn't compile |
| rodrimc/RayTracing_Parallel https://github.com/rodrimc/RayTracing_Parallel | Simple serial → OpenMP → CUDA progression | Simplest code if RTIOW feels heavy |
| kaustubh0201/Ray-Tracer https://github.com/kaustubh0201/Ray-Tracer | RT + OpenMP + SIMD intrinsics | Only if asked about advanced optimization |

> Rule: Learn from R1, build serial yourself. Read R2 to learn *where* to put `#pragma omp` + how to benchmark. Don't copy-paste R2 — code it yourself if time permits, else adapt with understanding.

---

## 2. Supporting References (use only when stuck)

| When | Link |
|---|---|
| Code too long / running out of time — fallback to 99-line tracer | http://kevinbeason.com/smallpt/ — already has `#pragma omp parallel for schedule(dynamic,1)`. Serial = compile without `-fopenmp`, parallel = with it. |
| OpenMP gives only 1.7x instead of 8x — read this | https://stackoverflow.com/questions/79256572 + https://github.com/RayTracing/raytracing.github.io/discussions/1664 — RNG race + stdout serialization + scheduling pitfalls |
| Need simple assignment-style raytracer reference | https://github.com/irri/simple_raytracer — Tokyo Tech assignment, OpenMP built-in |
| Need performance paper to cite | https://www.researchgate.net/publication/377787334 (ray tracing + OpenMP analysis) |

---

## 3. Order of Work (follow strictly)

### DAY 1 — Serial + MODULE-I

**Step 1 (2h): Theory crash**
1. Read RTIOW Ch 1-8 (don't code yet, just understand the loop)
2. Mental model: `for each pixel → for each sample → cast ray → intersect spheres → shade → bounce → accumulate → gamma correct`
3. Skim R2's `main_sequencial.cpp` to see how small a serial file can be

**Step 2 (4h): Serial code (MODULE-I Q4)**
- Build RTIOW Ch1-8 in C++. Key decisions for tomorrow:
  - Write pixels to `image[j][i]` array in memory, NOT `std::cout` per pixel
  - Isolate RNG in one function `random_double()` so you can make it thread-local tomorrow
  - Add at top of `main()`: `printf("Name: <YOU> Roll: <NUM>\n");` — required for marks
- Compile: `g++ -O3 -o ray_serial main.cpp`
- Test cases (increasing dimension — mandatory):
  - T1: 200x100, spp=10
  - T2: 400x200, spp=50
  - T3: 800x400, spp=100
- Save: `time ./ray_serial > out_T1.ppm` + screenshot terminal showing Name/Roll + time

**Step 3 (2h): MODULE-I handwritten (Q1,2,3,5)**
- Q1 Objective: photorealistic rendering is embarrassingly parallel, need OpenMP to cut wall time
- Q2 Pseudocode/flowchart: ray-gen → intersect → shade → recurse
- Q3 Demo: hand-trace a 3x2 image, 1 ray per pixel, show color computation
- Q5 Complexity: `O(W × H × S × B × N)` where W,H=res, S=samples, B=bounces, N=objects. Table T1<T2<T3 times to prove it.

### DAY 2 — Parallel + MODULE-II + PDF

**Step 4 (3h): Parallel code (MODULE-II Q9)**
Single change on outer pixel loop:

```cpp
#include <omp.h>
#pragma omp parallel for schedule(dynamic) shared(image)
for (int j = image_height-1; j >= 0; --j) {
  // thread-local seed: unsigned int seed = 1234 + omp_get_thread_num()*7919;
  for (int i = 0; i < image_width; ++i) {
    // spp sampling loop per pixel
  }
}
```

3 fixes that decide your marks:
1. **RNG race:** RTIOW `random_double()` uses `static` distribution → not thread-safe. Use per-thread seed / `threadprivate` / `drand48_r`.
2. **No cout in parallel region:** dump image array to file AFTER the `#pragma` block.
3. **`schedule(dynamic)`** not `static`: glass/mirror pixels bounce more than diffuse → load imbalance.

Compile: `g++ -O3 -fopenmp -o ray_omp main.cpp`

**Step 5 (2h): Benchmarks (Q10 — highest marks-per-hour)**
Same T1,T2,T3, run at 1/2/4/8 threads:
```bash
export OMP_NUM_THREADS=4; time ./ray_omp
```
Build one table: `Threads | Time(s) | Speedup (T1/Tp) | Efficiency (S/p)`. One speedup graph. Verify `diff serial.ppm parallel.ppm` ≈ identical → proves correctness.

Expected: ~4x on 4 cores, 5-8x on 8 cores. If you get <2x, you hit one of the 3 pitfalls above.

**Step 6 (3h): Handwritten + PDF assembly**
- Q6 Parallelizable blocks: pixel row loop YES (independent, best target), spp loop YES (reduction), `ray_color` recursion NO (sequential dependency), scene data read-only SAFE
- Q7 Parallel pseudocode: copy Q2 + add `#pragma omp parallel for schedule(dynamic)` line + note thread-private RNG
- Q8 Demo: same 3x2 image, show pixels computed concurrently by threads
- Q10: parallel complexity `O(W·H·S·B·N / p + overhead)`, Amdahl's law, why dynamic beats static
- Final PDF order (strict per guidelines):
  - (a) Handwritten Q1,2,3,5,6,7,8,10
  - (b) Serial code + output with Name/Roll visible
  - (c) Parallel OpenMP code + output with Name/Roll visible

---

## 4. Viva Cheat Sheet (10 min prep, saves marks)

1. "What did you parallelize?" → Outer pixel-row loop. Each pixel independent = embarrassingly parallel.
2. "Why dynamic scheduling?" → Pixels have uneven cost (more bounces = more work). Static idles threads.
3. "What breaks in parallel?" → Shared RNG state + stdout serialization. Fixed with thread-local seeds + image buffer.
4. "Speedup?" → Near-linear until core count, then Amdahl overhead dominates. Show your table.
5. "Why ray tracing needs parallel?" → `W×H×S` rays × bounces × objects = millions of independent float ops. Classic PDC problem.

---

## 5. If You Run Out of Time (escape hatch)

At end of Day 1 Hr 6, if serial RTIOW isn't rendering: **pivot to smallpt**. Compile without `-fopenmp` = serial submission, with it = parallel submission. Spend remaining time on theory + benchmarks + handwritten. A finished tiny tracer with perfect analysis beats a half-finished RTIOW.

---

## 6. Full Link Dump

- RTIOW book: https://raytracing.github.io/books/RayTracingInOneWeekend.html
- RTIOW source: https://github.com/RayTracing/raytracing.github.io
- smallpt: http://kevinbeason.com/smallpt/
- SantiagoDt (report template): https://github.com/SantiagoDt/Raytracing-parallel-performance
- guyleaf (serial vs OMP): https://github.com/guyleaf/Parallelized-Ray-Tracing-In-One-Weekend
- CSC746 course project: https://github.com/usatie/csc746-tp-ray-tracing-omp
- ChristianLagares OMP+CUDA: https://github.com/ChristianLagares/RayTracingInOneWeekend
- kaustubh OMP+SIMD: https://github.com/kaustubh0201/Ray-Tracer
- rodrimc serial/OMP/CUDA: https://github.com/rodrimc/RayTracing_Parallel
- Pitfalls SO: https://stackoverflow.com/questions/79256572
- Pitfalls discussion: https://github.com/RayTracing/raytracing.github.io/discussions/1664
