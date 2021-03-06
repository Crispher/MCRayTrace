#include "RayTracer.h"

RayTracer::RayTracer(Scene *_s, std::string _i, int _depth) :
scenePtr(_s), depth(_depth) {
	if (_i == "SimpleIntersectionTester") {
		intersectionTesterPtr = new SimpleIntersectionTester(scenePtr);
	}
	else if (_i == "KdtreeIntersectionTester") {
		intersectionTesterPtr = new Kdtree(scenePtr);
	}
	else {
		printf("Ray tracer initialization failed: no proper intersection tester specified.\n");
		exit(0);
	}
	samplerPtr = new Sampler3D();
}

RayTracer::~RayTracer() {
	delete samplerPtr;
	delete intersectionTesterPtr;
}

Color RayTracer::rayTrace(const Ray& ray) {
	lastRayType = CAMERA;
	return rayTrace(ray, depth);
}
Color RayTracer::rayTrace(const Ray& ray, int _depth) {
	if (_depth == 0) {
		return Colors::black;
	}

	bool intersected = false;
	Real distance = Limit::Infinity;
	Vector3R normal;
	MaterialPtr mPtr = nullptr;
	intersectionTesterPtr->intersectionTest(ray, intersected, distance, normal, mPtr);
	if (!intersected) {
		return scenePtr->ambientLight;
	}

	const bool dl = true;

	if (mPtr->isLightSource()) {
		if (lastRayType == DIFFUSE && dl)
			return Colors::black;
		Color ans = Colors::white.filter(mPtr->Kd);
		if (mPtr->isTextured()) {
			delete mPtr;
		}
		return ans;
	}
	Vector3R pos = ray.source + distance * ray.direction;
	Color ans = Colors::black;

	RaySample s = samplerPtr->sample(ray.direction, normal, mPtr);
	Ray _ray(pos, s.v);
	
	if ((s.type == DIFFUSE) && dl) {
		Real prob, scale;
		ans += directLighting(pos, normal, prob).filter(mPtr->Kd);
		s.radiance = rayTrace(_ray, _depth - 1);
		if (intersectionTesterPtr->scatterMode) {
			scale = exp(-scenePtr->scatterMtl->n * distance) / (1 - prob);
		}
		else {
			scale = 1 / (1 - prob);
		}
		ans += integrate(s, mPtr).scale(scale);
	}
	else {
		s.radiance = rayTrace(_ray, _depth - 1);
		ans += integrate(s, mPtr);
	}
	lastRayType = s.type;
	if (mPtr->isTextured()) {
		delete mPtr;
	}
	return ans;
}

Color RayTracer::integrate(const RaySample&s, const MaterialPtr &mPtr) {
	switch (s.type) {
	case DIFFUSE:
	case SCATTER:
		return s.radiance.filter(mPtr->Kd, s.weight);
	case SPECULAR_REFLECT:
		return s.radiance.filter(mPtr->Ks, s.weight);
	// TRANSMISSION DIFFUSE ?
	case SPECULAR_REFRACT_IN:
	case SPECULAR_REFRACT_OUT:
		return s.radiance.filter(mPtr->Ts, s.weight);
	default:
		return Colors::red;
	}
}

Color RayTracer::RussianRoulette(const Ray& ray, Real factor) {
	Real rand = samplerPtr->getUniform_01();
	if (rand > factor) {
		return Colors::black;
	}
	else {
		// with prob. p go on
		return rayTrace(ray, 1).scale(1. / factor);
	}
}

Color RayTracer::directLighting(const Vector3R &pos, const Vector3R &normal, Real &prob) {
	Color ans = Colors::black;
	prob = 0;
	for (int i = 0; i < scenePtr->numLightSources; i++) {
		MaterialPtr mPtr; Vector3R ls_normal; Real filter = 0; Real area;
		std::vector<Vector3R, Eigen::aligned_allocator<Vector3R>> samplePos = scenePtr->getLightSourceSamples(i, mPtr, ls_normal, area);
		for (int j = 0; j < samplePos.size(); j++) {
			if (intersectionTesterPtr->visible(pos, samplePos[j])) {
				Vector3R r_pos = pos - samplePos[j];
				Real cos_theta = -r_pos.normalized().dot(normal);
				if (cos_theta < 0) {
					continue;
				}
				Real r_squared = r_pos.squaredNorm();
				//Real cos_gamma = std::max(0.0, r_pos.normalized().dot(ls_normal));
				Real cos_gamma = abs(r_pos.normalized().dot(ls_normal));
				
				filter += cos_theta * cos_gamma / r_squared;
			}
		}
		ans += Colors::white.filter(mPtr->Kd).scale(filter / samplePos.size() * area / Constants::Pi);
		prob += filter * area / Constants::Pi / samplePos.size();
	}
	return ans;
}