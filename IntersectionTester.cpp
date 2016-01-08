#include "IntersectionTester.h"



#pragma region BASE_INTERSECTION_TESTER

IntersectionTester::~IntersectionTester() {
	delete sampler;
}

void IntersectionTester::basicIntersectionTest(const Ray &ray, const F& face) {
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
	if (t < Limit::Epsilon || t > cacheDistance) {
		return;
	}

	cacheIntersected = true;
	cacheDistance = t;

	if (visibilityTestMode) {
		return;
	}

	Vector3R n =
		scenePtr->normals[face.vn[0]].v * w + scenePtr->normals[face.vn[1]].v * u + scenePtr->normals[face.vn[2]].v * v;
	cacheNormal = n.normalized();
	cacheMPtr = face.materialPtr;

	if (!cacheMPtr->isTextured() && !cacheMPtr->isBumped()) {
		return;
	}
	else {
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
		int tI = std::min((int)(textureCoordinate[0] * cacheMPtr->texturePtr->W), cacheMPtr->texturePtr->W - 2);
		int tJ = std::min((int)(textureCoordinate[1] * cacheMPtr->texturePtr->H), cacheMPtr->texturePtr->H - 2);
		
		if (cacheMPtr->isBumped()) {
			/*Real shiftX = cacheMPtr->texturePtr->bump[tI + 1][tJ] - cacheMPtr->texturePtr->bump[tI][tJ];
			Real shiftY = cacheMPtr->texturePtr->bump[tI][tJ + 1] - cacheMPtr->texturePtr->bump[tI][tJ];
			cacheNormal = (cacheNormal + 10 * (shiftH * shiftX + shiftV * shiftY)).normalized();*/
			Real shiftX = 2 * uniform_01(gen) - 1;
			shiftX = (shiftX / abs(shiftX)) * pow(abs(shiftX), 1.7);
			Vector3R pos = ray.source + cacheDistance * ray.direction;
			Vector3R shift = (pos - Vector3R(0.3, 0.3, 0)).normalized();
			//Vector3R shift = Vector3R(0, 1, 0);
			cacheNormal = 2 * shift * shiftX + sqrt(1 - shiftX * shiftX) * cacheNormal;
			cacheNormal.normalize();
			return;
		}
		textureFilterR = cacheMPtr->texturePtr->texture[tI][tJ][0];
		textureFilterG = cacheMPtr->texturePtr->texture[tI][tJ][1];
		textureFilterB = cacheMPtr->texturePtr->texture[tI][tJ][2];
		return;
	}
}

void IntersectionTester::basicIntersectionTest(const Ray &ray, const Sphere& sphere) {
	
	Real B = 2 * (ray.direction.dot(ray.source - sphere.center));
	Real C = (ray.source - sphere.center).squaredNorm() - sphere.r * sphere.r;
	Real delta;
	if ((delta = (B*B - 4 * C)) < Limit::Epsilon) {
		return;
	}
	Real t1 = (-B - sqrt(delta)) / 2;
	if (t1 > cacheDistance) {
		return;
	}
	if (t1 < Limit::Epsilon) {
		t1 = (-B + sqrt(delta)) / 2;
		if (t1 < Limit::Epsilon || t1 > cacheDistance) {
			return;
		}
	}

	cacheIntersected = true;
	cacheDistance = t1;
	cacheNormal = (ray.source + cacheDistance * ray.direction - sphere.center).normalized();
	cacheMPtr = sphere.materialPtr;
}

bool SimpleIntersectionTester::visible(const Vector3R &pos1, const Vector3R &pos2) {
	visibilityTestMode = true;
	Ray ray = Ray::fromPoints(pos1, pos2);
	Real d = (pos2 - pos1).norm();
	cacheDistance = d;
	cacheIntersected = false;
	cacheNormal = Vector3R(1, 0, 0);
	cacheMPtr = nullptr;

	int n = scenePtr->faces.size();
	for (int i = 0; i < n; i++) {
		basicIntersectionTest(ray, scenePtr->faces[i]);
		if (cacheDistance < d - Limit::Epsilon) {
			visibilityTestMode = false;
			return false;
		}
	}
	n = scenePtr->spheres.size();
	for (int i = 0; i < n; i++) {
		basicIntersectionTest(ray, scenePtr->spheres[i]);
		if (cacheDistance < d - Limit::Epsilon) {
			visibilityTestMode = false;
			return false;
		}
	}
	visibilityTestMode = false;
	return true;
}

void IntersectionTester::clearCache() {
	cacheDistance = Limit::Infinity;
	cacheIntersected = false;
	cacheNormal = Vector3R(1, 0, 0);
	cacheMPtr = nullptr;
	cacheU = 0;
	cacheV = 0;
}

#pragma endregion

#pragma region SIMPLE_TESTER

SimpleIntersectionTester::SimpleIntersectionTester(Scene * _scenePtr) {
	scenePtr = _scenePtr;
	sampler = new Sampler3D();
}

void SimpleIntersectionTester::intersectionTest(const Ray &ray, bool &intersected, Real &distance, Vector3R &normal, MaterialPtr &mPtr) {
	clearCache();
	//scatterMode = false;
	/*if (scatterMode) {
		cacheDistance = exponential_scatter(gen);
	}*/

	int n = scenePtr->faces.size();
	for (int i = 0; i < n; i++) {
		basicIntersectionTest(ray, scenePtr->faces[i]);
	}
	n = scenePtr->spheres.size();
	for (int i = 0; i < n; i++) {
		basicIntersectionTest(ray, scenePtr->spheres[i]);
	}
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
	/*if (scatterMode && !intersected && cacheDistance < 5) {
		normal = sampler->sample_Diffuse_P(-ray.direction);
		intersected = true;
		mPtr = scenePtr->scatterMtl;
		if (mPtr->Kd.size() == 0) {
			printf("bug");
			exit(0);
		}
	}*/
}

#pragma endregion