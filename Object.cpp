#include "Object.h"

#pragma region IO

F::F(std::string s1, std::string s2, std::string s3) {
	std::vector<std::string> argv1, argv2, argv3;
	boost::split(argv1, s1, boost::is_any_of("/"));
	boost::split(argv2, s2, boost::is_any_of("/"));
	boost::split(argv3, s3, boost::is_any_of("/"));
	switch (argv1.size()) {
	case 3:
		vn = std::vector<int>(3);
		vn[0] = boost::lexical_cast<int>(argv1[2]) - 1;
		vn[1] = boost::lexical_cast<int>(argv2[2]) - 1;
		vn[2] = boost::lexical_cast<int>(argv3[2]) - 1;
	case 2:
		vt = std::vector<int>(3);
		vt[0] = boost::lexical_cast<int>(argv1[1]) - 1;
		vt[1] = boost::lexical_cast<int>(argv2[1]) - 1;
		vt[2] = boost::lexical_cast<int>(argv3[1]) - 1;
	case 1:
		v = std::vector<int>(3);
		v[0] = boost::lexical_cast<int>(argv1[0]) - 1;
		v[1] = boost::lexical_cast<int>(argv2[0]) - 1;
		v[2] = boost::lexical_cast<int>(argv3[0]) - 1;
	}
}

void Texture::loadBump(const char* filename) {
	cv::Mat image = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
	bumpW = image.cols, bumpH = image.rows;
	bump = std::vector<std::vector<Real>>(bumpW);
	for (int i = 0; i < bumpW; i++) {
		bump[i] = std::vector<Real>(bumpH);
	}
	for (int _w = 0; _w < bumpW; _w++) {
		for (int _h = 0; _h < bumpH	; _h++) {
			bump[_w][_h] = image.at<uchar>(bumpH - _h - 1, _w);
			bump[_w][_h] /= 255.0;
		}
	}
}

void Object::load(const char* filename) {
	std::ifstream in = std::ifstream(filename);
	std::string line;
	std::string currentMaterialName = "Default Material";
	while (std::getline(in, line)) {
		if (line[0] == '#') {
			continue;
		}
		else {
			std::vector<std::string> argv;
			boost::split(argv, line, boost::is_any_of("\t "));
			if (argv[0] == "v") {
				Real x, y, z, w;
				x = boost::lexical_cast<Real>(argv[1]);
				y = boost::lexical_cast<Real>(argv[2]);
				z = boost::lexical_cast<Real>(argv[3]);
				w = argv.size() > 4 ? boost::lexical_cast<Real>(argv[4]) : 1;
				vertices.emplace_back(x, y, z, w);
				continue;
			}
			if (argv[0] == "vt") {
				Real x, y, z, w;
				x = boost::lexical_cast<Real>(argv[1]);
				y = boost::lexical_cast<Real>(argv[2]);
				z = boost::lexical_cast<Real>(argv[3]);
				w = argv.size() > 4 ? boost::lexical_cast<Real>(argv[4]) : 1;
				textures.emplace_back(x, y, z, w);
				continue;
			}
			if (argv[0] == "vn") {
				Real x, y, z;
				x = boost::lexical_cast<Real>(argv[1]);
				y = boost::lexical_cast<Real>(argv[2]);
				z = boost::lexical_cast<Real>(argv[3]);
				normals.emplace_back(x, y, z);
				continue;
			}
			if (argv[0] == "f") {
				for (int i = 2; i + 1 < argv.size(); i++) {
					F f = F(argv[1], argv[i], argv[i + 1]);
					f.materialPtr = materials[currentMaterialName];
					faces.push_back(f);
				}
				continue;
			}
			if (argv[0] == "sphere") {
				Real x, y, z, w;
				x = boost::lexical_cast<Real>(argv[1]);
				y = boost::lexical_cast<Real>(argv[2]);
				z = boost::lexical_cast<Real>(argv[3]);
				w = boost::lexical_cast<Real>(argv[4]);
				spheres.emplace_back(x, y, z, w, materials[currentMaterialName]);
				continue;
			}
			if (argv[0] == "mtllib") {
				loadMtl(argv[1].c_str());
				continue;
			}
			if (argv[0] == "usemtl") {
				currentMaterialName = argv[1];
				continue;
			}
		}
	}
}

void Object::loadMtl(const char* filename) {
	std::ifstream in = std::ifstream(filename);
	std::string line;
	bool toggle = false;
	MaterialPtr mPtr = new Material;
	while (std::getline(in, line)) {
		if (line[0] == '#') {
			continue;
		}
		std::vector<std::string> argv;
		boost::split(argv, line, boost::is_any_of("\t "));
		if (argv[0] == "newmtl") {
			if (toggle) {
				materials.insert(std::pair<std::string, MaterialPtr>(mPtr->name, mPtr));
				mPtr = new Material;
			}
			toggle = true;
			mPtr->name = argv[1];
			continue;
		}
		if (argv[0] == "Ka" && toggle) {
			Real x, y, z;
			x = boost::lexical_cast<Real>(argv[1]);
			y = boost::lexical_cast<Real>(argv[2]);
			z = boost::lexical_cast<Real>(argv[3]);
			mPtr->Ka = vector < Real > {x, y, z};
			continue;
		}
		if (argv[0] == "Kd" && toggle) {
			Real x, y, z;
			x = boost::lexical_cast<Real>(argv[1]);
			y = boost::lexical_cast<Real>(argv[2]);
			z = boost::lexical_cast<Real>(argv[3]);
			mPtr->Kd = vector < Real > {x, y, z};
			continue;
		}
		if (argv[0] == "Ks" && toggle) {
			Real x, y, z;
			x = boost::lexical_cast<Real>(argv[1]);
			y = boost::lexical_cast<Real>(argv[2]);
			z = boost::lexical_cast<Real>(argv[3]);
			mPtr->Ks = vector < Real > {x, y, z};
			continue;
		}
		if (argv[0] == "Ns" && toggle) {
			mPtr->Ns = boost::lexical_cast<Real>(argv[1]);
			continue;
		}
		if (argv[0] == "Tr" && toggle) {
			mPtr->Tr = boost::lexical_cast<Real>(argv[1]);
			continue;
		}
		if (argv[0] == "Td" && toggle) {
			Real x, y, z;
			x = boost::lexical_cast<Real>(argv[1]);
			y = boost::lexical_cast<Real>(argv[2]);
			z = boost::lexical_cast<Real>(argv[3]);
			mPtr->Td = vector < Real > {x, y, z};
			continue;
		}
		if (argv[0] == "Ts" && toggle) {
			Real x, y, z;
			x = boost::lexical_cast<Real>(argv[1]);
			y = boost::lexical_cast<Real>(argv[2]);
			z = boost::lexical_cast<Real>(argv[3]);
			mPtr->Ts = vector < Real > {x, y, z};
			continue;
		}
		if (argv[0] == "Nst" && toggle) {
			mPtr->Nst = boost::lexical_cast<Real>(argv[1]);
			continue;
		}
		if (argv[0] == "n" && toggle) {
			mPtr->n = boost::lexical_cast<Real>(argv[1]);
			continue;
		}
		if (argv[0] == "bump" && toggle) {
			mPtr->textureMode = BUMP;
			mPtr->texturePtr = new Texture();
			mPtr->texturePtr->loadBump(argv[1].c_str());
			continue;
		}
		if (argv[0] == "BRDF" && toggle) {
			mPtr->textureMode = BRDF_DEFINED;
			Real
				r1 = boost::lexical_cast<Real>(argv[1]),
				r2 = boost::lexical_cast<Real>(argv[2]),
				r3 = boost::lexical_cast<Real>(argv[3]),
				r4 = boost::lexical_cast<Real>(argv[4]),
				r5 = boost::lexical_cast<Real>(argv[5]),
				r6 = boost::lexical_cast<Real>(argv[6]),
				r7 = boost::lexical_cast<Real>(argv[7]),
				r8 = boost::lexical_cast<Real>(argv[8]),
				r9 = boost::lexical_cast<Real>(argv[9]),
				r10 = boost::lexical_cast<Real>(argv[10]),
				r11 = boost::lexical_cast<Real>(argv[11]),
				r12 = boost::lexical_cast<Real>(argv[12]);
			std::vector < Real >
				kdr{ r1, r2, r3 },
				ksr{ r4, r5, r6 },
				kdp{ r7, r8, r9 },
				ksp{ r10, r11, r12 };
			mPtr->BRDFPtr = new Ward(kdr, ksr, kdp, ksp);
			continue;
		}
	}
	if (toggle) {
		materials.insert(std::pair<std::string, MaterialPtr>(mPtr->name, mPtr));
	}
}

void Object::printInfo() {
	for (int i = 0; i < vertices.size(); i++) {
		std::cout << "v_" << i << ": " << vertices[i].v << "\n";
	}
	for (int i = 0; i < faces.size(); i++) {
		std::cout << "f_" << i << ": " << faces[i].v[0] << "/ " << faces[i].v[1] << "/ " << faces[i].v[2] <<
			"; Material: " << faces[i].materialPtr->name << "\n";
	}
	for (auto i : materials) {
		std::cout << "mtl_" << i.first << ":\n" <<
			"\tKa: " << i.second->Ka[0] << ", " << i.second->Ka[1] << ", " << i.second->Ka[2] << "\n" <<
			"\tKd: " << i.second->Kd[0] << ", " << i.second->Kd[1] << ", " << i.second->Kd[2] << "\n" <<
			"\tKs: " << i.second->Ks[0] << ", " << i.second->Ks[1] << ", " << i.second->Ks[2] << "\n";
	}
}

#pragma endregion

#pragma region GEOMETRY_TRANSFORMATIONS

void Object::translate(const Vector3R& v) {
	for (auto iter = vertices.begin(); iter != vertices.end(); iter++) {
		iter->v += v;
	}
}

void Object::rotate(int axis, Real angle) {
	int axis1, axis2;
	Real c = cos(angle), s = sin(angle), _s = -s;
	switch (axis) {
	case 0:
		axis1 = 1; axis2 = 2;
		break;
	case 1:
		axis1 = 0; axis2 = 2;
		break;
	case 2:
		axis1 = 0; axis2 = 1;
		break;
	}
	for (auto iter = vertices.begin(); iter != vertices.end(); iter++) {
		Real v1 = iter->v[axis1], v2 = iter->v[axis2];
		iter->v[axis1] = c*v1 + _s*v2;
		iter->v[axis2] = s*v1 + c*v2;
	}
	for (auto iter = normals.begin(); iter != normals.end(); iter++) {
		Real v1 = iter->v[axis1], v2 = iter->v[axis2];
		iter->v[axis1] = c*v1 + _s*v2;
		iter->v[axis2] = s*v1 + c*v2;
	}
}

void Object::transform(const Matrix3R& m) {
	for (auto iter = vertices.begin(); iter != vertices.end(); iter++) {
		// todo
	}
	for (auto iter = normals.begin(); iter != normals.end(); iter++) {
		// todo
	}
}

#pragma endregion


