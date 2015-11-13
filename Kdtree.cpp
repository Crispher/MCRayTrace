#include "Kdtree.h"


#pragma region BBOX

BBox::BBox(Real _xMin, Real _xMax, Real _yMin, Real _yMax, Real _zMin, Real _zMax) :
minxyz(Vector3R(_xMin, _yMin, _zMin)), maxxyz(Vector3R(_xMax, _yMax, _zMax)) {}

bool BBox::contain(const Vector3R& p) {
	return 
		(minxyz[0] - Limit::Epsilon < p[0]) && 
		(p[0] < maxxyz[0] + Limit::Epsilon) &&
		(minxyz[1] - Limit::Epsilon < p[1]) &&
		(p[1] < maxxyz[1] + Limit::Epsilon) &&
		(maxxyz[2] - Limit::Epsilon < p[2]) && 
		(p[2] < maxxyz[2] + Limit::Epsilon);
}

void BBox::fit(const std::vector<int>& f) {

}

bool BBox::overlap(const BBox& bBox) {
	return segmentOverlap(minxyz[0], maxxyz[0], bBox.minxyz[0], bBox.maxxyz[0]) &&
		segmentOverlap(minxyz[1], maxxyz[1], bBox.minxyz[1], bBox.maxxyz[1]) &&
		segmentOverlap(minxyz[2], maxxyz[2], bBox.minxyz[2], bBox.maxxyz[2]);
}

bool BBox::overlap(const Vector3R& v1, const Vector3R& v2, const Vector3R& v3) {
	// todo : naive
	BBox b(min(v1[0], v2[0], v3[0]) - Limit::Epsilon, max(v1[0], v2[0], v3[0]) + Limit::Epsilon,
		min(v1[1], v2[1], v3[1]) - Limit::Epsilon, max(v1[1], v2[1], v3[1]) + Limit::Epsilon,
		min(v1[2], v2[2], v3[2]) - Limit::Epsilon, max(v1[2], v2[2], v3[2]) + Limit::Epsilon);
	return overlap(b);
}

bool BBox::intersectRay(const Ray& r) {
	// todo
	Real tmin, tmax, tymin, tymax, tzmin, tzmax; 
	if (r.direction[0] >= 0) {
		tmin = (minxyz[0] - r.source[0]) / r.direction[0]; 
		tmax = (maxxyz[0] - r.source[0]) / r.direction[0]; 
	}
	else { 
		tmin = (maxxyz[0] - r.source[0]) / r.direction[0]; 
		tmax = (minxyz[0] - r.source[0]) / r.direction[0]; 
	} 
	if (r.direction[1] >= 0) { 
		tymin = (minxyz[1] - r.source[1]) / r.direction[1]; 
		tymax = (maxxyz[1] - r.source[1]) / r.direction[1]; 
	}
	else {
		tymin = (maxxyz[1] - r.source[1]) / r.direction[1];
		tymax = (minxyz[1] - r.source[1]) / r.direction[1]; 
	} 
	if ((tmin > tymax) || (tymin > tmax)) 
		return false;

	if (tymin > tmin) 
		tmin = tymin; 
	if (tymax < tmax) 
		tmax = tymax; 
	
	if (r.direction[2] >= 0) {
		tzmin = (minxyz[2] - r.source[2]) / r.direction[2];
		tzmax = (maxxyz[2] - r.source[2]) / r.direction[2]; 
	}
	else {
		tzmin = (maxxyz[2] - r.source[2]) / r.direction[2]; 
		tzmax = (minxyz[2] - r.source[2]) / r.direction[2]; 
	} 

	if ((tmin > tzmax) || (tzmin > tmax)) 
		return false; 
	if (tzmin > tmin) 
		tmin = tzmin; 
	if (tzmax < tmax) 
		tmax = tzmax; 
	//return ((tmin < t1) && (tmax > 0));
	return (tmax > 0) && (tmin < tmax);
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
	//printf("split along %d, at %f\n", splitAxis, splitAt);
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

Kdtree::Kdtree(Scene *_scenePtr) {
	scenePtr = _scenePtr;
	root = new KdtreeNode(BBox(-2, 2, -2, 2, -2, 2));
	root->facesInside = vector<int>(scenePtr->faces.size());
	int n = root->facesInside.size();
	for (int i = 0; i < n; i++) {
		root->facesInside[i] = i;
	}
	root->split(scenePtr, maxDepth);
	//test(root);
}

void Kdtree::test(KdtreeNode *cursor) {
	if (cursor->isLeaf()) {
		for (int f : cursor->facesInside) {
			printf("%d ", f);
		}
		cursor->boundingBox.printInfo();
		printf("\n");
		printf("%d faces inside", cursor->facesInside.size());
		return;
	}
	test(cursor->lChild);
	test(cursor->rChild);
}

//#define LOG

void Kdtree::intersectionTest(const Ray& ray,
	bool& intersected, Real& distance, Vector3R& normal, MaterialPtr &mPtr) {
	debug_testcount = 0;
#ifdef LOG
	printf("intersection test: ray(%f, %f, %f -> %f, %f, %f)\n", ray.source[0], ray.source[1], ray.source[2],
		ray.direction[0], ray.direction[1], ray.direction[2]);
#endif
	// todo;
	clearCache();
	intersectionTest(root, ray);
#ifdef LOG
	//printf(" :%d: ", debug_testcount);
	printf("test result %d\n", cacheIntersected);
	Vector3R pos = ray.source + cacheDistance * ray.direction;
	printf("intersection at (%f, %f, %f), distance = %f\n", pos[0], pos[1], pos[2], cacheDistance);
#endif
	intersected = cacheIntersected;
	distance = cacheDistance;
	normal = cacheNormal;
	mPtr = cacheMPtr;
	if (cacheMPtr != nullptr) {
		// memory leak fixed
		if (mPtr->isTextureKd()) {
			mPtr = new Material(*cacheMPtr);
			mPtr->Kd[0] *= textureFilterR;
			mPtr->Kd[1] *= textureFilterG;
			mPtr->Kd[2] *= textureFilterB;
		}
	}
	//printf(": %d :", debug_testcount);
}

bool Kdtree::intersectionTest(KdtreeNode *cursor, const Ray& ray)  {
	// if ray do not intersect the bounding box, return false directly.
#ifdef LOG
	printf("\n\n KDNODE: intersection test: ray(%f, %f, %f -> %f, %f, %f)\n", ray.source[0], ray.source[1], ray.source[2],
		ray.direction[0], ray.direction[1], ray.direction[2]);
	cursor->boundingBox.printInfo();
#endif
	if (!cursor->boundingBox.intersectRay(ray)) {
		return false;
	}

	if (cursor->isLeaf()) {
#ifdef LOG
		printf("leaf: ");
#endif
		for (int f : cursor->facesInside) {
			basicIntersectionTest(ray, scenePtr->faces[f]);
			debug_testcount++;
#ifdef LOG
			printf("%d ", f);
#endif
		}
	}
	
	else {
		KdtreeNode *near, *far;
		near = ray.source[cursor->splitAxis] < cursor->splitAt ? cursor->lChild : cursor->rChild;
		far = ray.source[cursor->splitAxis] < cursor->splitAt ? cursor->rChild : cursor->lChild;

		bool decided = intersectionTest(near, ray);
		if (decided) {
			return true;
		}
		else {
			// bounding box check should be put here. todo // needless ?
			intersectionTest(far, ray);
		}		
	}

	if (cacheIntersected) {
		Vector3R intersection = ray.source + cacheDistance * ray.direction;
		// cursor->boundingBox.printInfo();
		if (cursor->boundingBox.contain(intersection)) {
			//printf(" |A| ");
			return true;
		}
	}
	return false;
}
#pragma endregion