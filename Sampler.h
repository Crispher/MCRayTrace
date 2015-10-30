#pragma once
#include "stdafx.h"
#include "Screen.h"
#include "Object.h"
#include "Scene.h"

struct Sample {
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		Vector3R v;
	Color radiance;
	Real weight;
};

struct Sample2R {
	Sample2R() {}
	Real u, v;
	Sample2R(Real _u, Real _v) : u(_u), v(_v) {}
	Color value;
};

struct PreSample {
	PreSample() {}
	Real x, y, z;
	Real weight;
	PreSample(Real _x, Real _y, Real _z, Real _w) :
		x(_x), y(_y), z(_z), weight(_w) {}
};

struct PreSamplePool {
	PreSamplePool() {}

	std::vector<PreSample> presamples;
	int elemPerGroup;
	int numGroups;
	int cursor;
};

class Sampler {
public:
	Scene *scenePtr;
	std::default_random_engine gen;
	std::uniform_real_distribution<Real> uniform_01 = std::uniform_real_distribution<Real>(0.0, 1.0);

	PreSamplePool presamples_Diffuse;
	std::map<int, PreSamplePool> presamples_Specular;
	PreSamplePool presamples_Diffuse_Single;
	std::map<int, PreSamplePool> presamples_Specular_Single;

	Sampler(){};
	~Sampler(){};

	// the only abstract function
	virtual std::vector<Sample2R> sample_UnitSquare_Uniform(int n) = 0;
	
	// designed for phong model only.
	// sample the hemisphere centered around the vector according to cosine value.
	std::vector<Sample, Eigen::aligned_allocator<Vector3R>> sample_Diffuse(const Vector3R&, int sampleSize);
	// sample the hemisphere centered around the vector according to the nth power of cosine value.
	std::vector<Sample, Eigen::aligned_allocator<Vector3R>> sample_Specular(const Vector3R&, const Vector3R&, int N, int sampleSize);

	// presample utilities, _Single suffix refers to sample pools that have group size 1.
	void generatePreSample_Diffuse(int groupSize, int numGroups);
	void generatePreSample_Diffuse_Single(int numSamples);
	void generatePreSample_Specular(int groupSize, int numGroups, int N);
	void generatePreSample_Specular_Single(int numSamples, int N);

	std::vector<Sample, Eigen::aligned_allocator<Vector3R>> sample_Diffuse_P(const Vector3R&, int sampleSize);
	std::vector<Sample, Eigen::aligned_allocator<Vector3R>> sample_Specular_P(const Vector3R&, const Vector3R&, int N, int sampleSize);
	// get a single sample
	Vector3R sample_Diffuse_P(const Vector3R&);
	Vector3R sample_Specular_P(const Vector3R&, const Vector3R&, int N);
};

class StratifiedSampler : public Sampler {
public:
	StratifiedSampler() {}
	std::vector<Sample2R> sample_UnitSquare_Uniform(int);
};

class LatinCubeSampler : public Sampler {
public:
	LatinCubeSampler() {}
	std::vector<Sample2R> sample_UnitSquare_Uniform(int);
};