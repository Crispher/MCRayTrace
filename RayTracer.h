#pragma once
#include "Screen.h"
#include "Scene.h"
#include "IntersectionTester.h"
#include "Sampler.h"

class RayTracer
{
public:

	int depth;
	int sampleSize;
	IntersectionTester *intersectionTesterPtr;
	Sampler *samplerPtr;

	RayTracer() {}
	~RayTracer() {}

	virtual Color rayTrace(const Ray&, int) = 0;
};


class MonteCarloRayTracer : public RayTracer{
public:
	MonteCarloRayTracer() {}
	Color rayTrace(const Ray &ray, int _depth);

private:
	Color integrateDiffuse_Reflect(const std::vector<Sample, Eigen::aligned_allocator<Vector3R>>&, const MaterialPtr &mPtr) const;
	Color integrateSpecular_Reflect(const std::vector<Sample, Eigen::aligned_allocator<Vector3R>>&, const MaterialPtr &mPtr) const;
	Color integrateDiffuse_Refract(const std::vector<Sample, Eigen::aligned_allocator<Vector3R>>&, const MaterialPtr &mPtr) const;
	Color integrateSpecular_Refract(const std::vector<Sample, Eigen::aligned_allocator<Vector3R>>&, const MaterialPtr &mPtr) const;

	Color rayTrace_Single(const Ray&, int _depth);
	Color RussianRoulette(const Ray&, Real factor);
	Color integrate(const Sample &s, const MaterialPtr &mPtr);
	inline Vector3R reflect(const Vector3R& in, const Vector3R& normal) {
		return in - 2 * in.dot(normal) * normal;
	}
	inline Vector3R refract(const Vector3R &in, const Vector3R &normal, Real n) {
		Real cos_theta1 = in.dot(normal);
		if (abs(cos_theta1) > 1 - Limit::Epsilon) {
			// perpendicular to the surface
			return in;
		}
		if (cos_theta1 > 0) {
			Real sin_theta1 = sqrt(1 - cos_theta1 * cos_theta1);
			Real sin_theta2 = n * sin_theta1;
			if (sin_theta2 > 1) {
				// no refraction
				return in - 2 * in.dot(normal) * normal;
			}
			Real cos_theta2 = sqrt(1 - sin_theta2 * sin_theta2);
			Vector3R H = (in - in.dot(normal) * normal).normalized();
			Vector3R out = cos_theta2 * normal + sin_theta2 * H;
			return out;
		}
		else {
			Real sin_theta1 = sqrt(1 - cos_theta1 * cos_theta1);
			Real sin_theta2 = sin_theta1 / n;
			Real cos_theta2 = sqrt(1 - sin_theta2 * sin_theta2);
			Vector3R H = (in - in.dot(normal) * normal).normalized();
			Vector3R out = -cos_theta2 * normal + sin_theta2 * H;
			return out;
		}
	}
};

