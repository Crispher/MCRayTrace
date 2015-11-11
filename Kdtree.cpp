#include "Kdtree.h"

#pragma region SIMPLE_INTERSECTION_TESTER

void SimpleIntersectionTester::intersectionTest(const Ray& ray,
	bool& out_intersected, Real& out_distance, Vector3R& out_normal,
	bool& out_reflected, Ray& out_reflectingRay,
	bool& out_refracted, Ray& out_refractingRay, Material& material) {
	out_intersected = false;
	int n = scenePtr->faces.size();
	for (int i = 0; i < n; i++) {
		scenePtr->basicIntersectionTest(ray, scenePtr->faces[i], out_intersected, out_distance, out_normal,
			out_reflected, out_reflectingRay, out_refracted, out_refractingRay, material);
	}
	n = scenePtr->spheres.size();
	for (int i = 0; i < n; i++) {
		scenePtr->basicIntersectionTest(ray, scenePtr->spheres[i], out_intersected, out_distance, out_normal,
			out_reflected, out_reflectingRay, out_refracted, out_refractingRay, material);
	}
}

void SimpleIntersectionTester::intersectionTest_NoAdditionals(const Ray& ray, bool& out_intersected, Real& out_distance) {
	out_intersected = false;
	int n = scenePtr->faces.size();
	for (int i = 0; i < n; i++) {
		scenePtr->basicIntersectionTest_NoAdditionals(ray, scenePtr->faces[i], out_intersected, out_distance);
	}
	n = scenePtr->spheres.size();
	for (int i = 0; i < n; i++) {
		scenePtr->basicIntersectionTest_NoAdditionals(ray, scenePtr->spheres[i], out_intersected, out_distance);
	}
}

#pragma endregion

#pragma region BBOX

BBox::BBox(Real _xMin, Real _xMax, Real _yMin, Real _yMax, Real _zMin, Real _zMax) :
minxyz(Vector3R(_xMin, _yMin, _zMin)), maxxyz(Vector3R(_xMax, _yMax, _zMax)) {}

bool BBox::contain(const Vector3R& p) {
	return (minxyz[0] - Limit::Epsilon < p[0] && p[0] < maxxyz[0] + Limit::Epsilon) &&
		(minxyz[1] - Limit::Epsilon < p[1] && p[1] < maxxyz[1] + Limit::Epsilon) &&
		(maxxyz[2] - Limit::Epsilon < p[2] && p[2] < maxxyz[2] + Limit::Epsilon);
}

void BBox::fit(const std::vector<int>& f) {

}

bool BBox::overlap(const BBox& bBox) {
	return segmentOverlap(minxyz[0], maxxyz[0], bBox.minxyz[0], bBox.maxxyz[0]) &&
		segmentOverlap(minxyz[1], maxxyz[1], bBox.minxyz[1], bBox.maxxyz[1]) &&
		segmentOverlap(minxyz[2], maxxyz[2], bBox.minxyz[2], bBox.maxxyz[2]);
}

bool BBox::overlap(const Vector3R& v1, const Vector3R& v2, const Vector3R& v3) {
	BBox b(min(v1[0], v2[0], v3[0]), max(v1[0], v2[0], v3[0]),
		min(v1[1], v2[1], v3[1]), max(v1[1], v2[1], v3[1]),
		min(v1[2], v2[2], v3[2]), max(v1[2], v2[2], v3[2]));
	return overlap(b);
}

bool BBox::intersectRay(const Ray& ray) {
	// todo
	return true;
}

void BBox::printInfo() {
	printf("(%f, %f, %f), (%f, %f, %f)\n", minxyz[0], minxyz[1], minxyz[2], maxxyz[0], maxxyz[1], maxxyz[2]);
}

#pragma endregion

#pragma region KDTREE

KdtreeNode::KdtreeNode(const BBox& _bBox) : boundingBox(_bBox) {}

bool KdtreeNode::isLeaf() {
	return splitAxis == -1;
}

void KdtreeNode::split(Scene *scenePtr, int depth) {
	if (depth == 0) {
		return;
	}
	Real dx = boundingBox.maxxyz[0] - boundingBox.minxyz[0];
	Real dy = boundingBox.maxxyz[1] - boundingBox.minxyz[1];
	Real dz = boundingBox.maxxyz[2] - boundingBox.minxyz[2];

	if (dx > dy && dx > dz) {
		splitAxis = 0;
	}
	else {
		splitAxis = (dy > dz) ? 1 : 2;
	}

	Vector3R split1 = boundingBox.maxxyz;
	Vector3R split2 = boundingBox.minxyz;
	splitAt = (split1[splitAxis] + split2[splitAxis]) / 2;
	split1[splitAxis] = splitAt;
	split2[splitAxis] = splitAt;
	printf("split along %d, at %f\n", splitAxis, splitAt);
	lChild = new KdtreeNode(BBox(boundingBox.minxyz, split1));
	rChild = new KdtreeNode(BBox(split2, boundingBox.maxxyz));

	for (int f : facesInside) {
		Vector3R &v1 = scenePtr->vertices[scenePtr->faces[f].v[0]].v, 
			&v2 = scenePtr->vertices[scenePtr->faces[f].v[1]].v,
			&v3 = scenePtr->vertices[scenePtr->faces[f].v[2]].v;
		if (lChild->boundingBox.overlap(v1, v2, v3)) {
			lChild->facesInside.push_back(f);
		}
		if (rChild->boundingBox.overlap(v1, v2, v3)) {
			rChild->facesInside.push_back(f);
		}
	}
	facesInside.clear();
	if (lChild->facesInside.size() > maxShapeCount) {
		lChild->split(scenePtr, depth - 1);
	}
	if (rChild->facesInside.size() > maxShapeCount) {
		rChild->split(scenePtr, depth - 1);
	}
}

Kdtree::Kdtree()
{
}


Kdtree::~Kdtree()
{
}

Kdtree::Kdtree(Scene *_scenePtr) {
	scenePtr = _scenePtr;
	root = new KdtreeNode(BBox(-2, 2, -2, 2, -2, 2));
	root->facesInside = vector<int>(scenePtr->faces.size());
	int n = root->facesInside.size();
	for (int i = 0; i < n; i++) {
		root->facesInside[i] = i;
	}
	root->split(scenePtr, 5);
}

void Kdtree::test(KdtreeNode *cursor) {
	if (cursor->isLeaf()) {
		for (int f : cursor->facesInside) {
			printf("%d ", f);
		}
		cursor->boundingBox.printInfo();
		printf("\n");
		return;
	}
	test(cursor->lChild);
	test(cursor->rChild);
}

void Kdtree::intersectionTest(const Ray& ray,
	bool& out_intersected, Real& out_distance, Vector3R& out_normal,
	bool& out_reflected, Ray& out_reflectingRay,
	bool& out_refracted, Ray& out_refractingRay, Material& material) {
	intersectionTest(root, ray, 
		out_intersected, out_distance, out_normal,
		out_reflected, out_reflectingRay,
		out_refracted, out_refractingRay, material);
}

bool Kdtree::intersectionTest(KdtreeNode *cursor, const Ray& ray,
	bool& out_intersected, Real& out_distance, Vector3R& out_normal,
	bool& out_reflected, Ray& out_reflectingRay,
	bool& out_refracted, Ray& out_refractingRay, Material& material)  {
	// if ray do not intersect the bounding box, return false directly.
	if (!cursor->boundingBox.intersectRay(ray)) {
		return false;
	}

	if (cursor->isLeaf()) {
		for (int f : cursor->facesInside) {
			scenePtr->basicIntersectionTest(ray, scenePtr->faces[f],
				out_intersected, out_distance, out_normal,
				out_reflected, out_reflectingRay,
				out_refracted, out_refractingRay,
				material);
		}
	}
	else {
		KdtreeNode *near, *far;
		near = ray.source[cursor->splitAxis] < cursor->splitAt ? cursor->lChild : cursor->rChild;
		far = ray.source[cursor->splitAxis] < cursor->splitAt ? cursor->rChild : cursor->lChild;

		bool decided = intersectionTest(near, ray, out_intersected, out_distance, out_normal,
			out_reflected, out_reflectingRay, out_refracted, out_refractingRay, material);
		if (decided) {
			return true;
		}
		else {
			// bounding box check should be put here. todo
			intersectionTest(far, ray, out_intersected, out_distance, out_normal,
				out_reflected, out_reflectingRay, out_refracted, out_refractingRay, material);
		}		
	}
	if (!out_intersected) {
		return false;
	}
	else {
		Vector3R intersection = ray.source + out_distance * ray.direction;
		if (cursor->boundingBox.contain(intersection)) {
			return true;
		}
	}
	return false;
}

void Kdtree::intersectionTest_NoAdditionals(const Ray& ray, bool& out_intersected, Real& out_distance) {

}

#pragma endregion