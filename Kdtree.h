#pragma once
#include "stdafx.h"
#include "Object.h"
#include "Scene.h"
class Ray;
class Material;
class Scene;

class SimpleIntersectionTester : public IntersectionTester{
public:
	Scene* scenePtr;
	SimpleIntersectionTester(Scene* _scenePtr) : scenePtr(_scenePtr) {}
	void intersectionTest(const Ray&,
		bool& out_intersected, Real& out_distance, Vector3R& out_normal,
		bool& out_reflected, Ray& out_reflectingRay,
		bool& out_refracted, Ray& out_refractingRay, Material& material
		);
	void intersectionTest_NoAdditionals(const Ray& ray,
		bool& out_intersected, Real& out_distance);
};

class BBox {
public:
	Vector3R minxyz, maxxyz;
	BBox(Real _xMin, Real _xMax, Real _yMin, Real _yMax, Real _zMin, Real _zMax);
	BBox(const Vector3R& _minxyz, const Vector3R _maxxyz) : minxyz(_minxyz), maxxyz(_maxxyz) {}
	bool contain(const Vector3R& p);
	bool overlap(const Vector3R& v1, const Vector3R& v2, const Vector3R& v3);
	bool overlap(const BBox& bBox);
	bool intersectRay(const Ray& ray);
	void fit(const std::vector<int>& faces);
	void printInfo();
};

class KdtreeNode {
public:
	int splitAxis = -1;
	Real splitAt;
	BBox boundingBox;
	const static int maxShapeCount = 10;
	KdtreeNode *lChild = nullptr, *rChild = nullptr;
	std::vector<int> facesInside;

	KdtreeNode(const BBox& _bBox);
	void split(Scene *scenePtr, int depth);
	bool isLeaf();
};

class Kdtree : public IntersectionTester {
public:
	Scene* scenePtr;
	KdtreeNode *root;
	int maxDepth = 3;
	int maxShapeNumber = 10;

	Kdtree();
	Kdtree(Scene* _scenePtr);
	~Kdtree();
	void intersectionTest(const Ray& ray,
		bool& out_intersected, Real& out_distance, Vector3R& out_normal,
		bool& out_reflected, Ray& out_reflectingRay,
		bool& out_refracted, Ray& out_refractingRay, Material& material);
	void intersectionTest_NoAdditionals(const Ray&, bool&, Real&);
	bool intersectionTest(KdtreeNode *cursor, const Ray& ray,
		bool& out_intersected, Real& out_distance, Vector3R& out_normal,
		bool& out_reflected, Ray& out_reflectingRay,
		bool& out_refracted, Ray& out_refractingRay, Material& material);

	void test(KdtreeNode *cursor);
};

#pragma region UTILITY

inline bool segmentOverlap(Real x1, Real x2, Real x3, Real x4) {
	return (x1 < x3 && x3 < x2) || (x3 < x1 && x1 < x4);
}

inline Real max(Real x, Real y, Real z) {
	Real m = x > y ? x : y;
	m = m > z ? m : z;
	return m;
}

inline Real min(Real x, Real y, Real z) {
	Real m = x < y ? x : y;
	m = m < z ? m : z;
	return m;
}

#pragma endregion