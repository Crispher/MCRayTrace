#pragma once
#include "stdafx.h"

class Scene;
class Sphere;
class F;

class SphereBody {
public:
	SphereBody(Scene*, int);

	Scene *scenePtr;
	int sphereId;

	Real mass;
	Real radius;
	Vector3R force;
	Vector3R position;
	Vector3R velocity;

	void stepPosition(Real dt, Real damping);
	void applyForce(const Vector3R&);
};

class TriangleBody {
public:
	TriangleBody(F*, Scene*);
	std::vector<Vector3R> vertices;
};

class Physics
{
public:
	Vector3R gravity;
	Real collision_damping;

	Physics(Scene*);
	~Physics();

	void step(Real dt);

	void reset();
	std::vector<SphereBody*> spheres;
	std::vector<TriangleBody*> triangles;
};