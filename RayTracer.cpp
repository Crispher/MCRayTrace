#include "RayTracer.h"
//#define DEBUG_INFO

Color MonteCarloRayTracer::rayTrace(const Ray& ray, int _depth) {
	if (_depth == 0) {
		return Colors::black;
	}

	bool intersected = false;
	Real distance = Limit::Infinity;
	Vector3R normal;
	MaterialPtr mPtr = nullptr;
	intersectionTesterPtr->intersectionTest(ray, intersected, distance, normal, mPtr);
	if (!intersected) {
		//return Colors::black;
		return Colors::white.filter(0.5, 0.5, 0.5);
	}

	if (mPtr != nullptr && mPtr->isLightSource()) {
		Color ans = Colors::white.filter(mPtr->Ka[0], mPtr->Ka[1], mPtr->Ka[2]);
		return ans;
	}
	// common reflection and refraction.
	Vector3R pos = ray.source + distance * ray.direction;
	Color ans = Colors::black;
	
	bool lookFromOutside = ray.direction.dot(normal) < 0;
	if (lookFromOutside) {
		std::vector<Sample, Eigen::aligned_allocator<Vector3R>> samples_diffuse, samples_specular;

		if (mPtr->isRd()) {
			samples_diffuse = samplerPtr->sample_Diffuse_P(normal, sampleSize);
			int n = samples_diffuse.size();
			for (int i = 0; i < n; i++) {
				Ray _ray(pos, samples_diffuse[i].v);
				samples_diffuse[i].radiance = rayTrace_Single(_ray, _depth - 1);
			}
			ans += integrateDiffuse_Reflect(samples_diffuse, mPtr);
		}
		if (mPtr->isRs()) {
			samples_specular = samplerPtr->sample_Specular_P(reflect(ray.direction, normal), normal, mPtr->Ns, sampleSize);
			int n = samples_specular.size();
			for (int i = 0; i < n; i++) {
				Ray _ray(pos, samples_specular[i].v);
				samples_specular[i].radiance = rayTrace_Single(_ray, _depth - 1);
			}
			ans += integrateSpecular_Reflect(samples_specular, mPtr);
		}
	}

	if (mPtr->isTr()) {
		Real sign = lookFromOutside ? -1 : 1;
		std::vector<Sample, Eigen::aligned_allocator<Vector3R>> samples_diffuse_r, samples_specular_r;

		if (mPtr->isTd()) {
			samples_diffuse_r = samplerPtr->sample_Diffuse_P(sign * normal, sampleSize);
			int n = samples_diffuse_r.size();
			for (int i = 0; i < n; i++) {
				Ray _ray(pos, samples_diffuse_r[i].v);
				samples_diffuse_r[i].radiance = rayTrace_Single(_ray, _depth - 1);

			}
			ans += integrateDiffuse_Refract(samples_diffuse_r, mPtr);
		}
		if (mPtr->isTs()) {
			samples_specular_r = samplerPtr->sample_Specular_P(refract(ray.direction, normal, mPtr->n), sign * normal, mPtr->Nst, sampleSize);
			int n = samples_specular_r.size();
			for (int i = 0; i < n; i++) {
				Ray _ray(pos, samples_specular_r[i].v);
				samples_specular_r[i].radiance = rayTrace_Single(_ray, _depth - 1);
			}
			ans += integrateSpecular_Refract(samples_specular_r, mPtr);
		}
	}
	if (mPtr->isTextured()) {
		delete mPtr;
	}
	return ans;
}

Color MonteCarloRayTracer::rayTrace_Single(const Ray& ray, int _depth) {
	if (_depth == 0) {
		return Colors::black;
		//return RussianRoulette(ray, 0.5);
	}

	bool intersected = false;
	Real distance = Limit::Infinity;
	Vector3R normal;
	MaterialPtr mPtr = nullptr;
	intersectionTesterPtr->intersectionTest(ray, intersected, distance, normal, mPtr);
	if (!intersected) {
		return Colors::black;
		// return Colors::white.filter(0.2, 0.2, 0.2);
	}

	if (mPtr->isLightSource()) {
		Color ans = Colors::white.filter(mPtr->Ka);
		return ans;
	}
	Vector3R pos = ray.source + distance * ray.direction;
	Color ans;

	Sample s = samplerPtr->sample(ray.direction, normal, mPtr);
	Ray _ray(pos, s.v);
	s.radiance = rayTrace_Single(_ray, _depth - 1);
	ans = integrate(s, mPtr);
	if (mPtr->isTextured()) {
		delete mPtr;
	}
	return ans;
}

Color MonteCarloRayTracer::integrateDiffuse_Reflect(const std::vector<Sample, Eigen::aligned_allocator<Vector3R>> &samples, 
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

Color MonteCarloRayTracer::integrateSpecular_Reflect(const std::vector<Sample, Eigen::aligned_allocator<Vector3R>> &samples, 
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

Color MonteCarloRayTracer::integrateDiffuse_Refract(const std::vector<Sample, Eigen::aligned_allocator<Vector3R>>& samples, 
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

Color MonteCarloRayTracer::integrateSpecular_Refract(const std::vector<Sample, Eigen::aligned_allocator<Vector3R>>& samples, 
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

Color MonteCarloRayTracer::integrate(const Sample &s, const MaterialPtr &mPtr) {
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

Color MonteCarloRayTracer::RussianRoulette(const Ray& ray, Real factor) {
	Real rand = samplerPtr->uniform_01(samplerPtr->gen);
	if (rand > factor) {
		return Colors::black;
	}
	else {
		// with prob. p go on
		return rayTrace_Single(ray, 1).scale(1. / factor);
	}
}