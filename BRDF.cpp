#include "BRDF.h"

Ward::Ward(const std::vector<Real> &_KdRef, const std::vector<Real> &_KsRef,
	const std::vector<Real> &_KdPerp, const std::vector<Real> &_KsPerp) :
	KdRef(_KdRef), KsRef(_KsRef), KdPerp(_KdPerp), KsPerp(_KsPerp) {

}

Vector3R Ward::sample(const Vector3R &in, const Vector3R &normal, 
	const Vector3R &refDirection, const Vector3R &refPos) {

	Real cosphi, costheta;

	Vector3R H = refDirection.cross(normal);
	Real rv = uniform_01(gen);
	if (rv < KdRef[0]) {
		// generatae random phi according to cosine distri, diffuse.
		cosphi = 2 * rv - 1;
	}
	else {
		cosphi = in.dot(refDirection);
	}
	Real rv2 = uniform_01(gen);
	if (rv < KdPerp[0]) {
		costheta = 2 * rv2 - 1;
	}
	else {
		costheta = in.dot(H);
	}
	Vector3R v = .99 * (H * costheta + refDirection * cosphi).normalized();
	assert(v.squaredNorm() <= 1);
	Vector3R n = -normal * sqrt(1 - v.squaredNorm());
	return (v + n).normalized();
}

Color Ward::integrate(const Color &c) {
	// now a trivial one.
	return c;
}