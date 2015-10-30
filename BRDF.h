#pragma once
#include "stdafx.h"
#include "screen.h"

// base class
class BRDF
{
public:
	BRDF() {};
	~BRDF() {};

	virtual Vector3R sample(const Vector3R &in, const Vector3R &normal, const Vector3R &refDirection, const Vector3R &refPos) = 0;
	virtual Color integrate(const Color& c) = 0;
};

class Ward : public BRDF {
public:
	std::vector<Real> KdRef, KsRef, KdPerp, KsPerp;
	std::default_random_engine gen;
	std::uniform_real_distribution<Real> uniform_01 = std::uniform_real_distribution<Real>(0.0, 1.0);

	Ward() {}
	Ward(const std::vector<Real>&, const std::vector<Real>&, const std::vector<Real>&, const std::vector<Real>&);

	Vector3R sample(const Vector3R &in, const Vector3R &normal, const Vector3R &refDirection, const Vector3R &refPos);
	Color integrate(const Color &c);
};