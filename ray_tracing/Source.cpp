#include <iostream>
#include <fstream>
#include <vector>
#include "glm/glm.hpp"
#include "Shapes.h"
#include "IO.h"
#include "PixelBuffer.h"



struct Camera {

	glm::dvec3 position, left_bottom, horizontal, vertical;
	double focal_length;

	Camera(const glm::dvec3 position, const glm::dvec3 target, const PixelBuffer& pixel_buffer)
		:position(position), focal_length(1)
	{
		glm::dvec3 up(0, 1, 0); // up vector for the camera, changing this value changes way we hold the camera
		auto forward = glm::normalize(target - position);
		auto right = glm::normalize(glm::cross(forward, up));  // (up, forward) yazarsan left olur çünkü cross product non-commutative
		// (sað el kuralý)
		up = glm::normalize(glm::cross(forward, -right)); // up vektörü için saðlama yaptýk sanýrým?


		horizontal = right * (double(pixel_buffer.dimensions.x) / pixel_buffer.dimensions.y); // raster space ile olan ratioyu korumak için
		vertical = up * 1.;
		left_bottom = position + forward * focal_length - horizontal * 0.5 - vertical * 0.5; // origin gibi düþün bunu
	}

	glm::dvec3 raster_to_world(const glm::dvec2 & r) { // map 2-D point in plane into 3-D space
		return left_bottom + r.x*horizontal + r.y*vertical;
	}
};



glm::dvec3 pixelColor(std::vector<Sphere> & shapes, const Ray & r,int depth) { // calculates the color of pixel wrt normals of the shapes and back.color
	int arr_len = shapes.size();
	double min_t = DBL_MAX;
	int index = -1;
	//std::cout << arr_len;

	if (depth <= 0) {
		return glm::dvec3(0);
	}

	for (int i = 0; i < arr_len; i++) {  //finds the index of the shape with the min. t value.
		double t_val = shapes[i].hit(r);
		if ((t_val >= 0.01) && (t_val < min_t)) {  // ignore close hits to prevent shadow acne?
			min_t = t_val;
			index = i;
		}
	}
	if (index == -1) {
		// no intersect with arr, default background color
		// default background, günbatýmý resmetmeye çalýþacam, en alt sarý-beyaz, en üste doðru turuncu-kýrmýzý.

		auto t = 0.5*(r.dir.y + 1.0);
		return ((1.0 - t)*glm::dvec3(1.0, 0.75, 0.5) + t * glm::dvec3(1.0, 0.75, 0.75)); // arka plan rengi
	}
	else {
		glm::dvec3 N = glm::normalize(r.at(min_t) - shapes[index].center);  // surface normal
		double rand_dist = random_double() + 0.5;  // a random double between 0 and 1,
		auto target = glm::normalize(glm::dvec3(random_double() + 0.5, random_double() + 0.5, random_double() + 0.5)); 
		// rand vec with len=1
		glm::dvec3 rand_point = N + rand_dist * target; // materyal tipine göre yansýyan rayi gösteren bir fonksiyon yazmalýyým?
		return (shapes[index].color * pixelColor(shapes,shapes[index].reflection(N,r,min_t),depth-1)); // 0.5 attuentitation mu oluyor þimdi?
	}
	
}





int main() {
	std::cout << "Initial Project is running" << std::endl << std::endl;

	PixelBuffer pixel_buffer(glm::ivec2(640, 480));
	Camera camera(glm::dvec3(0, 0, 15), glm::dvec3(0, 0, 0), pixel_buffer); // ilk param. origin, 2.param direction
	int smp_per_pixel = 100; int max_depth = 5;
	glm::dvec3 ground(0.4, 0.2, 0.0);
	glm::dvec3 blue(0.2, 0.4, 0.8);
	glm::dvec3 white(1.0, 1.0, 1.0);

	std::vector<Sphere> objects = {
		Sphere(glm::dvec3(0,-1.5,-1), 3, Material::Metal,blue,0),
		Sphere(glm::dvec3(-6,-1.5,-1), 3, Material::Diffuse),
		Sphere(glm::dvec3(9,6,2), 4, Material::Diffuse),
		Sphere(glm::dvec3(0,-104.5,-1),100, Material::Diffuse,ground,0),
		Sphere(glm::dvec3(-8,8,-1),2, Material::Dielect,ground,0)
	};

	// lab starts from here

	for (int y = 0; y < pixel_buffer.dimensions.y; y++)
	{
		for (int x = 0; x < pixel_buffer.dimensions.x; x++) {
			// we're looping through each pixel here
			glm::dvec3 color(0);  // color vector for our pixel
			// for antialiasing, generate (num of smpl per pixel) rays for each pixel
			for (int s = 0; s < smp_per_pixel; s++) {
				glm::dvec2 uv(x + random_double(), y + random_double());
				uv /= pixel_buffer.dimensions;  //   /=    => elementwise division, normalization step for the point

				

				Ray ray(camera.position, glm::normalize(camera.raster_to_world(uv) - camera.position));
				// a-b vektörü, b'den a'ya çizilen bir vektördür.

				color += pixelColor(objects, ray,max_depth)/=smp_per_pixel;
			}
			glm::dvec3 gc_color(sqrt(color.x), sqrt(color.y), sqrt(color.z)); // gamma corrected color, gamma = 2

			// bu loopta oluþan resim istediðimizin x eksenine göre tersi olur çünkü resmin koordinat 
			// sistemi y ekseni için terstir.
			// pixel_buffer.set(x, y, color); böyle pixel valueleri yazarsan resim x eksenine göre ters çýkar.
			pixel_buffer.set(x, pixel_buffer.dimensions.y - y - 1, gc_color); // doðrusu bu
		}
	}



	std::ofstream output;
	output.open("./image.ppm", std::ios::out | std::ios::trunc);
	if (!output.is_open())
		return 1;

	std::cout << "Outputting...";
	IO::write_as_PPM(pixel_buffer, output);  // function to write an image  in ppm format
	output.close();
	std::cout << "done!" << std::endl;

	return 0;
}