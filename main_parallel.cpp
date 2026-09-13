// Parallel version - OpenMP (Module-II) - print this for Module-II
// Same as main_serial.cpp, but row loop is parallelized with OpenMP
#include <omp.h>  // OpenMP
#include "rtweekend.h"
#include "Camera.h"
#include "HittableList.h"
#include "Material.h"
#include "Sphere.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string>

Color ray_color(const Ray& r, const Hittable& world, int depth) {
    hit_record rec;

    if (depth <= 0)
        return Color(0,0,0);

    if (world.hit(r, 0.001, infinity, rec)) {
        Ray scattered;
        Color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth-1);
        return Color(0,0,0);
    }

    Vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5*(unit_direction.y() + 1.0);
    return (1.0-t)*Color(1.0, 1.0, 1.0) + t*Color(0.5, 0.7, 1.0);
}

HittableList random_scene() {
    HittableList world;

    auto ground_material = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
    world.add(make_shared<Sphere>(Point3(0,-1000,0), 1000, ground_material));

    for (int a = -5; a < 5; a++) {
        for (int b = -5; b < 5; b++) {
            auto choose_mat = random_double();
            Point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<Material> sphere_material;

                if (choose_mat < 0.8) {
                    auto albedo = random_vec3() * random_vec3();
                    sphere_material = make_shared<Lambertian>(albedo);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    auto albedo = random_vec3(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<Metal>(albedo, fuzz);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                } else {
                    sphere_material = make_shared<Dielectric>(1.5);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<Dielectric>(1.5);
    world.add(make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
    world.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

    return world;
}

void write_color(std::ostream &out, Color pixel_color, int samples_per_pixel) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    auto scale = 1.0 / samples_per_pixel;
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);

    out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}

void render_scene(int num_threads, const std::string& output_filename, double& elapsed_time) {
    const auto aspect_ratio = 3.0 / 2.0;
    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 50;
    const int max_depth = 10;

    auto world = random_scene();

    Point3 lookfrom(13,2,3);
    Point3 lookat(0,0,0);
    Vec3 vup(0,1,0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;

    Camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

    std::vector<Color> image_buffer(image_width * image_height);

    omp_set_num_threads(num_threads);
    std::cout << "Rendering with " << num_threads << " thread(s)..." << std::flush;
    
    double start_time = omp_get_wtime();

    // ==================== PARALLELIZED WITH OPENMP (vs main_serial.cpp) ====================
    // Only this outer loop is parallelized - schedule(dynamic) for load balance
    #pragma omp parallel for schedule(dynamic)
    for (int j = image_height - 1; j >= 0; --j) {
        seed_random(42 + j, omp_get_thread_num()); // thread-local RNG (OpenMP)

        for (int i = 0; i < image_width; ++i) {
            Color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                Ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }
            image_buffer[(image_height - 1 - j) * image_width + i] = pixel_color;
        }
    }

    double end_time = omp_get_wtime();
    elapsed_time = end_time - start_time;
    std::cout << " Done in " << elapsed_time << " seconds.\n";
    // ==================== END PARALLELIZED BLOCK ====================

    std::ofstream out_file(output_filename);
    out_file << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    for (int i = 0; i < image_width * image_height; ++i) {
        write_color(out_file, image_buffer[i], samples_per_pixel);
    }
    out_file.close();
}

int main() {
    std::cout << "Starting Monte Carlo Path Tracer Benchmarks (Phases 1-5)\n";
    std::cout << "=========================================================\n";
#ifdef _OPENMP
    std::cout << "OpenMP enabled, max threads: " << omp_get_max_threads() << "\n";
#endif

    std::vector<int> thread_counts = {1, 2, 4, 8, 16};
    std::vector<double> times;

    std::ofstream csv_file("benchmark_results.csv");
    csv_file << "Threads,Time(s),Speedup,Efficiency\n";

    double serial_time = 0.0;

    for (int t : thread_counts) {
        double elapsed;
        std::string filename = "image_t" + std::to_string(t) + ".ppm";
        render_scene(t, filename, elapsed);
        
        times.push_back(elapsed);
        if (t == 1) serial_time = elapsed;

        double speedup = serial_time / elapsed;
        double efficiency = speedup / t;

        csv_file << t << "," << elapsed << "," << speedup << "," << efficiency << "\n";
    }

    csv_file.close();
    std::cout << "\nBenchmarks complete! Results written to benchmark_results.csv\n";
    return 0;
}
