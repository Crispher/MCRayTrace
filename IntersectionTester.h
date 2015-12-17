#pragma once
#include "Scene.h"
#include "stdafx.h"
#include "Sampler.h"

class IntersectionTester
{
public:
	Scene *scenePtr;
    Sampler3D *sampler;

	bool scatterMode = true;
	bool visibilityTestMode = false;
	
	std::default_random_engine gen;
	std::uniform_real_distribution<Real> uniform_01 = std::uniform_real_distribution<Real>(0.0, 1.0);
	std::exponential_distribution<Real> exponential_scatter = std::exponential_distribution<Real>(5e-1);

	IntersectionTester() {};
	~IntersectionTester() {};

	void basicIntersectionTest(const Ray &ray, const F&);
	void basicIntersectionTest(const Ray &ray, const Sphere&);
	bool visible(const Vector3R &pos1, const Vector3R &pos2);

	virtual void intersectionTest(const Ray&, bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr) = 0;

protected:
	void clearCache();
	bool cacheIntersected;
	Real cacheDistance;
	Vector3R cacheNormal;
	MaterialPtr cacheMPtr;
	Real textureFilterR, textureFilterG, textureFilterB;
	Real cacheU, cacheV;
};

class SimpleIntersectionTester : public IntersectionTester {
public:
	SimpleIntersectionTester() {}
	void intersectionTest(const Ray&, bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr);
};
