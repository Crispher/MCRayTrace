#pragma once
#include "stdafx.h"
#include "Screen.h"
#include "Scene.h"
#include "IntersectionTester.h"
#include "Sampler.h"

class RayTracer {
public:
	RayTracer(Scene*, IntersectionTester*, int _depth);
	~RayTracer();
	Color rayTrace(const Ray&, int _depth);

	int depth;

private:
	Color integrateDiffuse_Reflect(const std::vector<RaySample, Eigen::aligned_allocator<Vector3R>>&, const MaterialPtr &mPtr) const;
	Color integrateSpecular_Reflect(const std::vector<RaySample, Eigen::aligned_allocator<Vector3R>>&, const MaterialPtr &mPtr) const;
	Color integrateDiffuse_Refract(const std::vector<RaySample, Eigen::aligned_allocator<Vector3R>>&, const MaterialPtr &mPtr) const;
	Color integrateSpecular_Refract(const std::vector<RaySample, Eigen::aligned_allocator<Vector3R>>&, const MaterialPtr &mPtr) const;

	
	Color RussianRoulette(const Ray&, Real factor);
	Color integrate(const RaySample &s, const MaterialPtr &mPtr);
	Color directLighting(const Vector3R&, const Vector3R &normal, Real &prob);

	Scene *scenePtr;
	IntersectionTester *intersectionTesterPtr;
	Sampler3D *samplerPtr;
};

