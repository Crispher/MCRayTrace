#pragma once

#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <queue>
#include <string>
#include <list>
#include <algorithm>
#include <random>
#include <thread>
#include <functional>
#include <sstream>

#include "boost\algorithm\string.hpp"
#include "boost\lexical_cast.hpp"
#include "boost\algorithm\string.hpp"
#include "boost\lexical_cast.hpp"

#include "Eigen/Dense"
#include "Eigen/StdVector"

#ifdef USE_FLOAT
typedef float Real;
typedef Eigen::Vector3f Vector3R;
typedef Eigen::Matrix3f Matrix3R;
#else 
typedef double Real;
typedef Eigen::Vector3d Vector3R;
typedef Eigen::Matrix3d Matrix3R;
#endif

namespace Limit {
	const int Infinity = 65535;
#ifdef USE_FLOAT
	const Real Epsilon = 1e-6;
#else
	const Real Epsilon = 1e-7;
#endif
}

namespace Constants {
	const Real Pi = 3.141592653589793238;
}

inline Vector3R reflect(const Vector3R& in, const Vector3R& normal) {
	return in - 2 * in.dot(normal) * normal;
}

inline Vector3R refract(const Vector3R &in, const Vector3R &normal, Real n) {
	Real cos_theta1 = in.dot(normal);
	if (abs(cos_theta1) > 1 - Limit::Epsilon) {
		// perpendicular to the surface
		return in;
	}
	if (cos_theta1 > 0) {
		Real sin_theta1 = sqrt(1 - cos_theta1 * cos_theta1);
		Real sin_theta2 = n * sin_theta1;
		if (sin_theta2 > 1) {
			// no refraction
			return in - 2 * in.dot(normal) * normal;
		}
		Real cos_theta2 = sqrt(1 - sin_theta2 * sin_theta2);
		Vector3R H = (in - in.dot(normal) * normal).normalized();
		Vector3R out = cos_theta2 * normal + sin_theta2 * H;
		return out;
	}
	else {
		Real sin_theta1 = sqrt(1 - cos_theta1 * cos_theta1);
		Real sin_theta2 = sin_theta1 / n;
		Real cos_theta2 = sqrt(1 - sin_theta2 * sin_theta2);
		Vector3R H = (in - in.dot(normal) * normal).normalized();
		Vector3R out = -cos_theta2 * normal + sin_theta2 * H;
		return out;
	}
}
