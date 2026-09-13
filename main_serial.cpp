#include "rtweekend.h"
#include "Camera.h"
#include "HittableList.h"
#include "Material.h"
#include "Sphere.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <chrono>

// Core Recursive Ray Color Function
Color ray_color(const Ray& r, const Hittable& world, int depth) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return Color(0,0,0);

    if (world.hit(r, 0.001, infinity, rec)) {
        Ray scattered;
        Color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth-1);
        return Color(0,0,0);
    }

    // Background Sky Gradient
    Vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5*(unit_direction.y() + 1.0);
    return (1.0-t)*Color(1.0, 1.0, 1.0) + t*Color(0.5, 0.7, 1.0);
}

// Generates the 3D Scene
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
                    // diffuse
                    auto albedo = random_vec3() * random_vec3();
                    sphere_material = make_shared<Lambertian>(albedo);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = random_vec3(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<Metal>(albedo, fuzz);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
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

// Writes RGB Color to File
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

// Serial Render Function
void render_scene_serial(const std::string& output_filename) {
    // Image Properties
    const auto aspect_ratio = 3.0 / 2.0;
    const int image_width = 400; 
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 50;
    const int max_depth = 10;

    auto world = random_scene();

    // Camera Settings
    Point3 lookfrom(13,2,3);
    Point3 lookat(0,0,0);
    Vec3 vup(0,1,0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;
    Camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

    std::vector<Color> image_buffer(image_width * image_height);

    std::cout << "Rendering Serially (1 Thread)..." << std::flush;
    
    // Benchmark Start (Using std::chrono instead of omp_get_wtime to ensure pure serial compilation capabilities)
    auto start_time = std::chrono::high_resolution_clock::now();

    // PURE SERIAL LOOP: No OpenMP Directives
    for (int j = image_height - 1; j >= 0; --j) {
        
        // Seed PRNG uniquely for this row
        seed_random(42 + j, 0);

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

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::cout << " Done in " << elapsed.count() << " seconds.\n";

    // Write to PPM file
    std::ofstream out_file(output_filename);
    out_file << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    for (int i = 0; i < image_width * image_height; ++i) {
        write_color(out_file, image_buffer[i], samples_per_pixel);
    }
    out_file.close();
}

int main() {
    std::cout << "Starting Pure Serial Monte Carlo Path Tracer\n";
    std::cout << "=============================================\n";
    render_scene_serial("serial_output.ppm");
    std::cout << "Render complete! Image saved to serial_output.ppm\n";
    return 0;
}
