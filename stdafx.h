#pragma once

#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <list>
#include <algorithm>
#include <random>
#include <thread>
#include <functional>

#include "boost\algorithm\string.hpp"
#include "boost\lexical_cast.hpp"
#include "boost\algorithm\string.hpp"
#include "boost\lexical_cast.hpp"

#include "Eigen/Dense"
#include "Eigen/StdVector"

#define USE_FLOAT
#define NDEBUG

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


