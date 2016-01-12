#pragma once
#include "Scene.h"
#include "stdafx.h"
#include "Sampler.h"

class IntersectionTester
{
protected:
	Scene *scenePtr;
	Sampler3D *sampler;
	bool visibilityTestMode = false;
	std::default_random_engine gen;

	std::uniform_real_distribution<Real> uniform_01 = std::uniform_real_distribution<Real>(0.0, 1.0);
	std::exponential_distribution<Real> exponential_scatter;


public:
	bool scatterMode = true;
	
	IntersectionTester() {};
	~IntersectionTester();

	void basicIntersectionTest(const Ray &ray, const F&);
	void basicIntersectionTest(const Ray &ray, const Sphere&);
	virtual bool visible(const Vector3R &pos1, const Vector3R &pos2) = 0;

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
	SimpleIntersectionTester(Scene *scenePtr);
	void intersectionTest(const Ray&, bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr);
	bool visible(const Vector3R &pos1, const Vector3R &pos2);
};
