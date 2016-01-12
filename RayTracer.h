#pragma once
#include "stdafx.h"
#include "Screen.h"
#include "Scene.h"
#include "IntersectionTester.h"
#include "Kdtree.h"
#include "Sampler.h"



class RayTracer {
public:
	RayTracer(Scene*, std::string intersectionTester, int _depth);
	~RayTracer();
	Color rayTrace(const Ray&);
	Color rayTrace(const Ray&, int _depth);

private:
	Color RussianRoulette(const Ray&, Real factor);
	Color integrate(const RaySample &s, const MaterialPtr &mPtr);
	Color directLighting(const Vector3R&, const Vector3R &normal, Real &prob);

	Scene *scenePtr;
	IntersectionTester *intersectionTesterPtr;
	Sampler3D *samplerPtr;
	int depth;

	RayType lastRayType;
};