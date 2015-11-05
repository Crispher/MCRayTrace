#pragma once
#include "Scene.h"
#include "stdafx.h"

class IntersectionTester
{
public:
	Scene *scenePtr;

	IntersectionTester() {};
	~IntersectionTester() {};

	void basicIntersectionTest(const Ray &ray, const F&, bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr);
	void basicIntersectionTest(const Ray &ray, const Sphere&, bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr);

	virtual void intersectionTest(const Ray&, bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr) = 0;

	static inline Vector3R reflect(const Vector3R& in, const Vector3R& normal) {
		return in - 2 * in.dot(normal) * normal;
	}
	static inline Vector3R refract(const Vector3R &in, const Vector3R &normal, Real n) {
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

class SimpleIntersectionTester : public IntersectionTester {
public:
	SimpleIntersectionTester() {}
	void intersectionTest(const Ray&, bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr);
};
