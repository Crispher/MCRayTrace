#pragma once
#include "Screen.h"
#include "stdafx.h"


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
	VN(const Vector3R& v) { this->v = v; }
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

class Material
{
public:
	Material(){};
	~Material(){};
	Material(const Material&);
	//std::string name;
	vector<Real> Ka, Kd, Ks, Td, Ts;
	int Ns = -1;

	Real n = 1; // relative refraction rate
	int Nst = -1;
	Texture *texturePtr;
	unsigned short flag = 0; // 1 rd | 2 rs | 4 tr | 8 td | 16 ts | 32 texture | 64 brdf | channel: 128 bump | 256 Kd | Ks | Td | Ts | islightsource | fresnel
	inline void setRd() { flag |= 1; }
	inline void setRs() { flag |= 2; }
	inline void setTr() { flag |= 4; }
	inline void setTd() { flag |= 8; }
	inline void setTs() { flag |= 16; }
	inline void setTexture() { flag |= 32; }
	inline void setBRDF() { flag |= 64; }
	inline void setBump() { flag |= 128; }
	inline void setTextureKd() { flag |= 256; }
	inline void setTextureKs() { flag |= 512; }
	inline void setTextureTd() { flag |= 1024; }
	inline void setTextureTs() { flag |= 2048; }
	inline void setLightsource() { flag |= 4096; }
	inline void setFresnel() { flag |= 8192; }
	bool isRd() { return flag & 1; }
	bool isRs() { return flag & 2; }
	bool isTr() { return flag & 4; }
	bool isTd() { return flag & 8; }
	bool isTs() { return flag & 16; }
	bool isTextured() { return flag & 32; }
	bool isBRDF() { return flag & 64; }
	bool isBumped() { return flag & 128; }
	bool isTextureKd() { return flag & 256; }
	bool isTextureKs() { return flag & 512; }
	bool isTextureTd() { return flag & 1024; }
	bool isTextureTs() { return flag & 2048; }
	bool isLightSource() { return flag & 4096; }
	bool isFresnel() { return flag & 8192; }
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
	void relocate(Real minx, Real minY, Real minz);
	void computeNormals(bool interpolation);
	void scale(Real ratio);

	// material information:
	std::vector<V, Eigen::aligned_allocator<Vector3R>> vertices;
	std::vector<VN, Eigen::aligned_allocator<Vector3R>> normals;
	std::vector<VT, Eigen::aligned_allocator<Vector3R>> textures;
	std::vector<F, Eigen::aligned_allocator<Vector3R>> faces;
	std::vector<Sphere, Eigen::aligned_allocator<Vector3R>> spheres;
	std::map<std::string, MaterialPtr> materials;

};