#pragma once
#include "stdafx.h"
#include "Object.h"
#include "screen.h"

struct Camera {
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	Vector3R position;

	// should be unit vectors 
	Vector3R direction;
	Vector3R up;
	Vector3R right;

	Real focolength;
	Real width, height;
	Real step;

	bool DOF = false;
	Real aperture, focalPlane;

	Camera(){}
	Camera(Real px, Real py, Real pz,
		Real dx, Real dy, Real dz,
		Real ux, Real uy, Real uz,
		Real foco,
		Real width, Real height,
		Real step);
	static Camera load(const char*);
	void setDOF(Real _a, Real _f) {
		DOF = true;
		aperture = _a;
		focalPlane = _f;
	}
	void move(double f, double r, double u) {
		position += f * direction + r * right + u * up;
	}
	void zoomIn(double d) {
		position += d*direction;
	}
	void tiltRight(double d) {
		direction += d*right;
		direction.normalize();
		right = direction.cross(up);
	}
	void tiltUp(double d) {
		direction += d*up;
		direction.normalize();
		up = right.cross(direction);
	}
	void rotate(double d) {
		up += d * right;
		up.normalize();
		right = direction.cross(up);
	}
	void printInfo();
};

class Scene
{
public:
	Scene(){};
	~Scene(){};

	// scene settings
	Color ambientLight = Colors::black;
	int numLightSources = 0;

	//Camera camera;
	std::vector<Object> objects;

	std::vector<F, Eigen::aligned_allocator<Vector3R>> AreaLightSources;
	std::vector<V, Eigen::aligned_allocator<Vector3R>> vertices;
	std::vector<VN, Eigen::aligned_allocator<Vector3R>> normals;
	std::vector<VT, Eigen::aligned_allocator<Vector3R>> textures;
	std::vector<F, Eigen::aligned_allocator<Vector3R>> faces;
	std::vector<Sphere, Eigen::aligned_allocator<Vector3R>> spheres;
	std::map<std::string, MaterialPtr> materials;

	void loadObject(const char*);
	void loadSceneSettings(const char*);
	void constructScene();	
};
