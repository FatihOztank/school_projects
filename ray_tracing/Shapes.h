#pragma once
#include <iostream>
#include "glm/glm.hpp"


double random_double() {
	// returns a random double between -0.5 and 0.5
	return (rand() / (RAND_MAX + 1.0)) - 0.5;
}

glm::dvec3 random_unit_vector() {
	
	auto p = glm::dvec3(2.0 * random_double(), 2.0 * random_double(), 2.0 * random_double());
	return glm::normalize(p);
}




enum class Material {  // generate different reflection for each type of material
	Metal,Diffuse,Dielect
};


struct Ray { // can't do ray tracing without rays yani.
	// bir ray için bir originimiz bir de directionumuz olmalý.

	glm::dvec3 origin, dir;
	Ray(const glm::dvec3 & orig, const glm::dvec3 & dir)
		:origin(orig), dir(dir)
	{}

	glm::dvec3 at(double t) const {
		return origin + t * dir;
	}

};


class Shape {
public:
	glm::dvec3 center; //center of gravity for any shape
	Material mat;
	glm::dvec3 color;
	double fuzz;
	Shape() :center(0) {}  // default shape constructor
	virtual double hit(const Ray & r) = 0;

	Ray reflection(const glm::dvec3 & norm, const Ray & r, double t_val) {
		if (mat == Material::Diffuse) {
			double rand_dist = random_double() + 0.5;  // a random double between 0 and 1,
			auto target = glm::normalize(glm::dvec3(random_double() + 0.5, random_double() + 0.5, random_double() + 0.5));
			// rand vec with len=1
			glm::dvec3 rand_point = r.at(t_val) + norm + random_unit_vector();
			return Ray(r.at(t_val),
				glm::normalize(rand_point - r.at(t_val)));
		}
		else if (mat == Material::Metal) {
			glm::dvec3 target_inv = 2.0*(r.at(t_val) - r.origin);
			glm::dvec3 tmp = target_inv - target_inv * glm::dot(norm, glm::normalize(r.at(t_val)));
			glm::dvec3 target = r.at(t_val) + norm + tmp + fuzz*random_unit_vector();  
			// ýþýnýn gideceði noktayý bulup norm + r.at(t_val) ' i ekliyorsun.
			return Ray(r.at(t_val),
				glm::normalize(target - r.at(t_val)));
		}
		else {// transparents
			// use fuzz parameter as eta/eta_prime here
			auto cos_theta = fmin(glm::dot(-r.at(t_val), norm), 1.0);
			glm::dvec3 r_out_perp = fuzz * (r.at(t_val) + cos_theta * norm);
			glm::dvec3 r_out_parallel = -sqrt(fabs(1.0 - glm::dot(r_out_perp, r_out_perp))) * norm;
			glm::dvec3 target = r_out_perp + r_out_parallel;
			return Ray(r.at(t_val),
				glm::normalize(target - r.at(t_val)));
		}
	}
};


class Sphere : public Shape {
public:
	double radius;
public:
	Sphere(glm::dvec3 c, double r, Material mat)
		:radius(r) {
		this->center = c;
		this->mat = mat;
		this->color = glm::dvec3(0.5,0.5,0.5);
		this->fuzz = 0.0;
	}
	Sphere(glm::dvec3 c, double r,Material mat,glm::dvec3 color, double f)
		:radius(r) {
		this->center = c;
		this->mat = mat;
		this->color = color;
		this->fuzz = f;
	}
	double hit(const Ray & ray) { //Shape::hit olarak declare etmen sadece class dýþýndaysan gerekli, 
		//yoksa gereksiz, boþ yere hata alýrsýn.
		glm::dvec3 oc = ray.origin - center;
		auto a = glm::dot(ray.dir, ray.dir);
		auto b = 2.0 * glm::dot(oc, ray.dir);
		auto c = glm::dot(oc, oc) - radius * radius;
		auto discriminant = b * b - 4 * a*c;

		if (discriminant < 0) { return -1.0; }
		else {
			return (-b - sqrt(discriminant)) / (2 * a);
		}
	}


};
