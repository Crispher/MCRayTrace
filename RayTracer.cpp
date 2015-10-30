#include "RayTracer.h"
//#define DEBUG_INFO

Color MonteCarloRayTracer::rayTrace(const Ray& ray, int _depth) {
	if (_depth == 0) {
		return Colors::black;
	}

	bool intersected = false;
	Real distance = Limit::Infinity;
	Vector3R normal;
	MaterialPtr mPtr;
	intersectionTesterPtr->intersectionTest(ray, intersected, distance, normal, mPtr);
	if (!intersected) {
		return Colors::black;
	}

	if (mPtr->name == "AreaLightSource") {
		return Colors::white.filter(mPtr->Ka[0], mPtr->Ka[1], mPtr->Ka[2]);
	}
	// common reflection and refraction.
	Vector3R pos = ray.source + distance * ray.direction;
	Color ans;
	if (_depth < depth) {
		sampleSize = 1;
	}

	bool lookFromOutside = ray.direction.dot(normal) < 0;
	if (lookFromOutside) {
		std::vector<Sample, Eigen::aligned_allocator<Vector3R>> samples_diffuse, samples_specular;

		samples_diffuse = samplerPtr->sample_Diffuse_P(normal, sampleSize);
		int n = samples_diffuse.size();
		for (int i = 0; i < n; i++) {
			Ray _ray(pos, samples_diffuse[i].v);
			samples_diffuse[i].radiance = rayTrace(_ray, _depth - 1);
		}
		ans += integrateDiffuse_Reflect(samples_diffuse, mPtr);

		if (mPtr->Ns > 0) {
			samples_specular = samplerPtr->sample_Specular_P(reflect(ray.direction, normal), normal, mPtr->Ns, sampleSize);
			n = samples_specular.size();
			for (int i = 0; i < n; i++) {
				Ray _ray(pos, samples_specular[i].v);
				samples_specular[i].radiance = rayTrace(_ray, _depth - 1);
			}
			ans += integrateSpecular_Reflect(samples_specular, mPtr);
		}
	}

	if (mPtr->Tr > 0) {
		Real sign = lookFromOutside ? -1 : 1;
		std::vector<Sample, Eigen::aligned_allocator<Vector3R>> samples_diffuse_r, samples_specular_r;

		samples_diffuse_r = samplerPtr->sample_Diffuse_P(sign * normal, sampleSize);
		int n = samples_diffuse_r.size();
		for (int i = 0; i < n; i++) {
			Ray _ray(pos, samples_diffuse_r[i].v);
			samples_diffuse_r[i].radiance = rayTrace(_ray, _depth - 1);
				
		}
		ans += integrateDiffuse_Refract(samples_diffuse_r, mPtr);
			
		if (mPtr->Nst > 0) {
			samples_specular_r = samplerPtr->sample_Specular_P(refract(ray.direction, normal, mPtr->n), sign * normal, mPtr->Nst, sampleSize);
			n = samples_specular_r.size();
			for (int i = 0; i < n; i++) {
				Ray _ray(pos, samples_specular_r[i].v);
				samples_specular_r[i].radiance = rayTrace(_ray, _depth - 1);
			}
			ans += integrateSpecular_Refract(samples_specular_r, mPtr);
		}
	}
	return ans;
}

Color MonteCarloRayTracer::rayTrace_Single(const Ray& ray, int _depth) {
	if (_depth == 0) {
		return Colors::black;
	}

	bool intersected = false;
	Real distance = Limit::Infinity;
	Vector3R normal;
	MaterialPtr mPtr;
	intersectionTesterPtr->intersectionTest(ray, intersected, distance, normal, mPtr);
	if (!intersected) {
		return Colors::black;
	}

	if (mPtr->name == "AreaLightSource") {
		return Colors::white.filter(mPtr->Ka[0], mPtr->Ka[1], mPtr->Ka[2]);
	}
	Vector3R pos = ray.source + distance * ray.direction;
	Color ans;

	// to be altered
	bool lookFromOutside = ray.direction.dot(normal) < 0;
	if (lookFromOutside) {
		Vector3R diffuse = samplerPtr->sample_Diffuse_P(normal);
		Ray _ray(pos, diffuse);
		Color radiance_d = rayTrace(_ray, _depth - 1);
		ans += radiance_d;

		if (mPtr->Ns > 0) {
			Vector3R specular = samplerPtr->sample_Specular_P(reflect(ray.direction, normal), normal, mPtr->Ns);
			Ray _ray(pos, specular);
			Color radiance_s = rayTrace(_ray, _depth - 1);
			ans += radiance_s;
		}
	}

	if (mPtr->Tr > 0) {
		Real sign = lookFromOutside ? -1 : 1;
		
		Vector3R diffuse = samplerPtr->sample_Diffuse_P(sign * normal);
		Ray _ray(pos, diffuse);
		Color radiance_d = rayTrace(_ray, _depth - 1);
		ans += radiance_d;

		if (mPtr->Nst > 0) {
			Vector3R specular = samplerPtr->sample_Specular_P(refract(ray.direction, normal, mPtr->n), sign * normal, mPtr->Nst);
			Ray _ray(pos, specular);
			Color radiance_s = rayTrace(_ray, _depth - 1);
			ans += radiance_s;
		}
	}
	return ans;
	// end

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
