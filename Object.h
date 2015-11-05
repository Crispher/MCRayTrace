#pragma once
#include "Screen.h"
#include "stdafx.h"
#include "BRDF.h"

/* redefine object in neater way */
/* convention: use 'v' to denote coordinates and
s */

class V {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		Vector3R v;
	V(Real x, Real y, Real z, Real w = 1) {
		v = Vector3R(x, y, z);
	}
};

class VN {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		Vector3R v;
	VN(Real x, Real y, Real z, Real w = 1) {
		v = Vector3R(x, y, z);
	}
};

class VT {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		Vector3R v;
	VT(Real x, Real y, Real z, Real w = 1) {
		v = Vector3R(x, y, z);
	}
};

class Material;

class F {
public:
	std::vector<int> v;
	std::vector<int> vt;
	std::vector<int> vn;
	Material *materialPtr;
	F(std::string, std::string, std::string);
};

class Sphere {
public:
	Vector3R center;
	Real r;
	Material *materialPtr;
	Sphere(Real x, Real y, Real z, Real _r, Material *_m) : center(Vector3R(x, y, z)), r(_r), materialPtr(_m) {}
};

class Ray {
public:
	Vector3R source;
	Vector3R direction;
	Ray(){}
	Ray(const Vector3R& s_, const Vector3R& d_) {
		source = s_;
		direction = d_;
	}
	static Ray fromPoints(const Vector3R& s_, const Vector3R& p_) {
		Ray ray;
		ray.source = s_;
		ray.direction = (p_ - s_).normalized();
		return ray;
	}
};

class Texture {
public:
	Texture() {}
	std::vector<std::vector<Real>> bump;
	std::vector<std::vector<std::vector<Real>>> texture;
	void loadBump(const char*);
	void loadTexture(const char*);
	int H, W;
};

enum TextureMode { NO_TEXTURE, BUMP, TEXTURE, BRDF_DEFINED };

class Material
{
public:
	Material(){};
	~Material(){};
	std::string name;
	vector<Real> Ka, Kd, Ks, Td, Ts;
	Real Ns = -1;
	Real Tr = -1;
	Real n = 1; // relative refraction rate
	Real Nst = -1;
	TextureMode textureMode = NO_TEXTURE;
	Texture *texturePtr;
	BRDF *BRDFPtr;
};

typedef Material *MaterialPtr;


class Object
{
public:
	Object(){};
	~Object(){};

	// io:
	void load(const char* filename);
	void loadMtl(const char* filename);
	void printInfo();

	// geometry transformations:
	void translate(const Vector3R& v);
	void rotate(int axis, Real angle);
	void transform(const Matrix3R& m);

	// material information:

	std::vector<V, Eigen::aligned_allocator<Vector3R>> vertices;
	std::vector<VN, Eigen::aligned_allocator<Vector3R>> normals;
	std::vector<VT, Eigen::aligned_allocator<Vector3R>> textures;
	std::vector<F, Eigen::aligned_allocator<Vector3R>> faces;
	std::vector<Sphere, Eigen::aligned_allocator<Vector3R>> spheres;
	std::map<std::string, MaterialPtr> materials;

};