#include "IntersectionTester.h"

#pragma region BASE_INTERSECTION_TESTER

void IntersectionTester::basicIntersectionTest(const Ray &ray, const F& face,
	bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr, Real *extra) {

	const Vector3R &p0 = scenePtr->vertices[face.v[0]].v, &p1 = scenePtr->vertices[face.v[1]].v, &p2 = scenePtr->vertices[face.v[2]].v;

	Vector3R E1 = p1 - p0;
	Vector3R E2 = p2 - p0;
	Vector3R T = ray.source - p0;
	Vector3R P = ray.direction.cross(E2);

	Real det = P.dot(E1);
	if (abs(det) <= Limit::Epsilon) {
		return;
	}
	Real u = P.dot(T) / det;
	if (u > 1 + Limit::Epsilon || u < -Limit::Epsilon) {
		return;
	}
	Vector3R Q = T.cross(E1);
	Real v = Q.dot(ray.direction) / det, w;
	if (v < -Limit::Epsilon || v > 1 + Limit::Epsilon || (w = (1 - u - v)) < -Limit::Epsilon) {
		return;
	}
	Real t = Q.dot(E2) / det;
	if (t < Limit::Epsilon || t > distance) {
		return;
	}

	intersected = true;
	distance = t;
	Vector3R n =
		scenePtr->normals[face.vn[0]].v * w + scenePtr->normals[face.vn[1]].v * u + scenePtr->normals[face.vn[2]].v * v;
	normal = n.normalized();
	mPtr = face.materialPtr;
	switch (mPtr->textureMode)
	{
	case NO_TEXTURE:
		return;
	case BUMP: {
		// compute normal shift
		Vector3R &t0 = scenePtr->textures[face.vt[0]].v, &t1 = scenePtr->textures[face.vt[1]].v, &t2 = scenePtr->textures[face.vt[2]].v;
		Vector3R T1 = t1 - t0, T2 = t2 - t0;
		Real v11 = E1.squaredNorm(), v12 = E1.dot(E2), v22 = E2.squaredNorm();
		Real det2 = v11 * v22 - v12 * v12;
		Real k11 = (T2[0] * v22 - T1[0] * v12) / det2;
		Real k12 = (v11 * T2[0] - v12 * T1[0]) / det2;
		Real k21 = (T2[1] * v22 - T1[1] * v12) / det2;
		Real k22 = (v11 * T2[1] - v12 * T1[1]) / det2;
		Vector3R shiftH = k11 * E1 + k12 * E2;
		Vector3R shiftV = k21 * E1 + k22 * E2;

		Vector3R textureCoordinate = t0 * w + t1 * u + t2 * v;
		int tI = std::min((int)(textureCoordinate[0] * mPtr->texturePtr->bumpW), mPtr->texturePtr->bumpW - 2);
		int tJ = std::min((int)(textureCoordinate[1] * mPtr->texturePtr->bumpH), mPtr->texturePtr->bumpH - 2);
		Real shiftX = mPtr->texturePtr->bump[tI + 1][tJ] - mPtr->texturePtr->bump[tI][tJ];
		Real shiftY = mPtr->texturePtr->bump[tI][tJ + 1] - mPtr->texturePtr->bump[tI][tJ];
		normal = (normal + 10 * (shiftH * shiftX + shiftV * shiftY)).normalized();
	}
		return;
	case TEXTURE:
		//todo;
		return;
	case BRDF_DEFINED:
		//todo;
		return;
	default:
		return;
	}
}

void IntersectionTester::basicIntersectionTest(const Ray &ray, const Sphere& sphere,
	bool &intersected, Real &distance, Vector3R &normal, MaterialPtr& mPtr, Real *extra) {
	
	Real B = 2 * (ray.direction.dot(ray.source - sphere.center));
	Real C = (ray.source - sphere.center).squaredNorm() - sphere.r * sphere.r;
	Real delta;
	if ((delta = (B*B - 4 * C)) < Limit::Epsilon) {
		return;
	}
	Real t1 = (-B - sqrt(delta)) / 2;
	if (t1 > distance) {
		return;
	}
	if (t1 < Limit::Epsilon) {
		t1 = (-B + sqrt(delta)) / 2;
		if (t1 < Limit::Epsilon || t1 > distance) {
			return;
		}
	}

	intersected = true;
	distance = t1;
	normal = (ray.source + distance * ray.direction - sphere.center).normalized();
	mPtr = sphere.materialPtr;
}

#pragma endregion

#pragma region SIMPLE_TESTER

void SimpleIntersectionTester::intersectionTest(const Ray &ray, bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr) {
	distance = Limit::Infinity;
	intersected = false;
	Real *extraPtr;

	int n = scenePtr->faces.size();
	for (int i = 0; i < n; i++) {
		basicIntersectionTest(ray, scenePtr->faces[i], intersected, distance, normal, mPtr, extraPtr);
	}
	n = scenePtr->spheres.size();
	for (int i = 0; i < n; i++) {
		basicIntersectionTest(ray, scenePtr->spheres[i], intersected, distance, normal, mPtr, extraPtr);
	}
}

#pragma endregion