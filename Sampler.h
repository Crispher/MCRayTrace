#pragma once
#include "stdafx.h"
#include "Screen.h"
#include "Object.h"
#include "Scene.h"

enum RayType { CAMERA, DIFFUSE, SPECULAR_REFLECT, SPECULAR_REFRACT_IN, SPECULAR_REFRACT_OUT, SCATTER};

struct RaySample{
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	Vector3R v;
	Color radiance;
	Real weight;
	RayType type = DIFFUSE;
};

struct Sample2D {
	Sample2D() {}
	Real u, v;
	Sample2D(Real _u, Real _v) : u(_u), v(_v) {}
	Color value;
};

struct PreSample3D {
	PreSample3D() {}
	Real x, y, z;
	Real weight;
	PreSample3D(Real _x, Real _y, Real _z, Real _w) :
		x(_x), y(_y), z(_z), weight(_w) {}
};

struct PreSamplePool3D {
	PreSamplePool3D() {}
	std::vector<PreSample3D> presamples;
	int elementCount;
	int cursor;
};

class Sampler2D {
public:
	Sampler2D() {}
	~Sampler2D() {}
	virtual std::vector<Sample2D> sample_UnitSquare_Uniform(int) = 0;
	std::vector<Sample2D> sampleTriangle(int n);
protected:
	std::default_random_engine gen;
	std::uniform_real_distribution<Real> uniform_01 = std::uniform_real_distribution<Real>(0.0, 1.0);
};

class Sampler3D {
public:
    Sampler3D();
	~Sampler3D(){};

	// return a random real in [0,1);
	Real getUniform_01();

	// the only abstract function
	void generatePreSamples_Diffuse(int numSamples);
	void generatePreSamples_Specular(int numSamples, int N);

	// get a single sample
	Vector3R sample_Diffuse_P(const Vector3R&);
	Vector3R sample_Specular_P(const Vector3R&, const Vector3R&, int N);

	// get a single sample
	RaySample sample(const Vector3R &in, const Vector3R &normal, const MaterialPtr &mPtr);

private:
	std::default_random_engine gen;
	std::uniform_real_distribution<Real> uniform_01 = std::uniform_real_distribution<Real>(0.0, 1.0);
	PreSamplePool3D presamples_Diffuse;
	std::map<int, PreSamplePool3D> presamples_Specular;
	PreSamplePool3D presamples_Diffuse_Single;
	std::map<int, PreSamplePool3D> presamples_Specular_Single;

	Sample2D sampleUnitSquare_Uniform();
};

class StratifiedSampler : public Sampler2D {
public:
	StratifiedSampler() {}
	std::vector<Sample2D> sample_UnitSquare_Uniform(int);
};

class LatinCubeSampler : public Sampler2D {
public:
	LatinCubeSampler() {}
	std::vector<Sample2D> sample_UnitSquare_Uniform(int);
};