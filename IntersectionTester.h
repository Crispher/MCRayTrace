#pragma once
#include "Scene.h"
#include "stdafx.h"

class IntersectionTester
{
public:
	Scene *scenePtr;

	IntersectionTester() {};
	~IntersectionTester() {};

	void basicIntersectionTest(const Ray &ray, const F&, bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr, Real *extra);
	void basicIntersectionTest(const Ray &ray, const Sphere&, bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr, Real *extra);

	virtual void intersectionTest(const Ray&, bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr) = 0;
};

class SimpleIntersectionTester : public IntersectionTester {
public:
	SimpleIntersectionTester() {}
	void intersectionTest(const Ray&, bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr);
};
