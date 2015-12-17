#include "RayTracer.h"
//#define DEBUG_INFO
RayTracer::RayTracer(Scene *_s, IntersectionTester *_i, Sampler3D *_sm) :
scenePtr(_s), intersectionTesterPtr(_i), samplerPtr(_sm) {
}

Color RayTracer::rayTrace(const Ray& ray, int _depth) {
	if (_depth == 0) {
		/*if (intersectionTesterPtr->visible(Vector3R(0, 0, 0), ray.source)) {
			return Colors::white;
		}*/
		return Colors::black;
		//return RussianRoulette(ray, 0.5);
	}

	bool intersected = false;
	Real distance = Limit::Infinity;
	Vector3R normal;
	MaterialPtr mPtr = nullptr;
	intersectionTesterPtr->intersectionTest(ray, intersected, distance, normal, mPtr);
	if (!intersected) {
		return intersectionTesterPtr->scenePtr->ambientLight;
	}

	if (mPtr->isLightSource()) {
		if (_depth < depth)
			return Colors::black;
		Color ans = Colors::white.filter(mPtr->Kd);
		if (mPtr->isTextured()) {
			delete mPtr;
		}
		return ans;
	}
	Vector3R pos = ray.source + distance * ray.direction;
	Color ans;

	
	RaySample s = samplerPtr->sample(ray.direction, normal, mPtr);
	Ray _ray(pos, s.v);
	s.radiance = rayTrace(_ray, _depth - 1);
	ans = integrate(s, mPtr);
	if (mPtr->isTextured()) {
		delete mPtr;
	}
	ans += directLighting(pos, normal).filter(mPtr->Kd);
	return ans;
}

Color RayTracer::integrateDiffuse_Reflect(const std::vector<RaySample, Eigen::aligned_allocator<Vector3R>> &samples, 
	const MaterialPtr &mPtr) const {
	Real r = 0, g = 0, b = 0;
	int n = samples.size();
	for (int i = 0; i < n; i++) {
		r += samples[i].radiance.red() / samples[i].weight;
		g += samples[i].radiance.green() / samples[i].weight;
		b += samples[i].radiance.blue() / samples[i].weight;
	}
	return Color(r * mPtr->Kd[0] / n, g * mPtr->Kd[1] / n, b * mPtr->Kd[2] / n);
}

Color RayTracer::integrateSpecular_Reflect(const std::vector<RaySample, Eigen::aligned_allocator<Vector3R>> &samples, 
	const MaterialPtr &mPtr) const {
	Real r = 0, g = 0, b = 0;
	int n = samples.size();
	for (int i = 0; i < n; i++) {
		r += samples[i].radiance.red() / samples[i].weight;
		g += samples[i].radiance.green() / samples[i].weight;
		b += samples[i].radiance.blue() / samples[i].weight;
	}
	return Color(r * mPtr->Ks[0] / n, g * mPtr->Ks[1] / n, b * mPtr->Ks[2] / n);
}

Color RayTracer::integrateDiffuse_Refract(const std::vector<RaySample, Eigen::aligned_allocator<Vector3R>>& samples, 
	const MaterialPtr &mPtr) const {
	Real r = 0, g = 0, b = 0;
	int n = samples.size();
	for (int i = 0; i < n; i++) {
		r += samples[i].radiance.red() / samples[i].weight;
		g += samples[i].radiance.green() / samples[i].weight;
		b += samples[i].radiance.blue() / samples[i].weight;
	}
	return Color(r * mPtr->Td[0] / n, g * mPtr->Td[1] / n, b * mPtr->Td[2] / n);
}

Color RayTracer::integrateSpecular_Refract(const std::vector<RaySample, Eigen::aligned_allocator<Vector3R>>& samples, 
	const MaterialPtr &mPtr) const {
	Real r = 0, g = 0, b = 0;
	int n = samples.size();
	for (int i = 0; i < n; i++) {
		r += samples[i].radiance.red() / samples[i].weight;
		g += samples[i].radiance.green() / samples[i].weight;
		b += samples[i].radiance.blue() / samples[i].weight;
	}
	return Color(r * mPtr->Ts[0] / n, g * mPtr->Ts[1] / n, b * mPtr->Ts[2] / n);
}

Color RayTracer::integrate(const RaySample&s, const MaterialPtr &mPtr) {
	switch (s.type) {
	case RE_D:
		return s.radiance.filter(mPtr->Kd, s.weight);
	case RE_S:
		return s.radiance.filter(mPtr->Ks, s.weight);
	case TR_D:
		return s.radiance.filter(mPtr->Td, s.weight);
	case TR_S:
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

Color RayTracer::directLighting(const Vector3R &pos, const Vector3R &normal) {
	Color ans = Colors::black;
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
				if (abs(cos_gamma) > 1 + Limit::Epsilon) {
					std::cout << cos_gamma << " ||\n " << r_pos << " ||\n " << ls_normal << std::endl;
					exit(0);
				}
				
				filter += cos_theta * cos_gamma / r_squared;
			}
		}
		ans += Colors::white.filter(mPtr->Kd).scale(filter / samplePos.size() * area / Constants::Pi);
	}
	return ans;
}