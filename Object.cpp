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
	W = image.cols; H = image.rows;
	bump = std::vector<std::vector<Real>>(W);
	for (int i = 0; i < W; i++) {
		bump[i] = std::vector<Real>(H);
	}
	for (int _w = 0; _w < W; _w++) {
		for (int _h = 0; _h < H	; _h++) {
			bump[_w][_h] = image.at<uchar>(H - _h - 1, _w);
			bump[_w][_h] /= 255.0;
		}
	}
}

void Texture::loadTexture(const char* filename) {
	cv::Mat image = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
	W = image.cols; H = image.rows;
	texture = std::vector<std::vector<std::vector<Real>>>(W);
	for (int i = 0; i < W; i++) {
		texture[i] = std::vector<std::vector<Real>>(H, std::vector<Real>(3));
	}
	for (int _w = 0; _w < W; _w++) {
		for (int _h = 0; _h < H; _h++) {
			cv::Vec3b t = image.at<cv::Vec3b>(H - _h - 1, _w);
			texture[_w][_h][0] = t[2] / (Real)255.0;
			texture[_w][_h][1] = t[1] / (Real)255.0;
			texture[_w][_h][2] = t[0] / (Real)255.0;
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
	// to do;
	if (normals.empty()) {
		computeNormals(false);
	}
}

Material::Material(const Material& m) {
	Ka = m.Ka; Kd = m.Kd; Ks = m.Ks;
	Td = m.Td; Ts = m.Ts; 
	//name = m.name;
	Ns = m.Ns;
	Nst = m.Nst;
	n = m.n;
	flag = m.flag;
	texturePtr = m.texturePtr;
}

void Object::loadMtl(const char* filename) {
	std::ifstream in = std::ifstream(filename);
	std::string line;
	bool toggle = false;
	MaterialPtr mPtr = new Material;
	std::string lastMtlName;
	while (std::getline(in, line)) {
		if (line[0] == '#') {
			continue;
		}
		std::vector<std::string> argv;
		boost::split(argv, line, boost::is_any_of("\t "));
		if (argv[0] == "newmtl") {
			if (toggle) {
				materials.insert(std::pair<std::string, MaterialPtr>(lastMtlName, mPtr));
				mPtr = new Material;
			}
			toggle = true;
			lastMtlName = argv[1];
			if (argv[1] == "AreaLightSource") {
				mPtr->setLightsource();
			}
			continue;
		}
		if (argv[0] == "Ka" && toggle) {
			Real x, y, z;
			x = boost::lexical_cast<Real>(argv[1]);
			y = boost::lexical_cast<Real>(argv[2]);
			z = boost::lexical_cast<Real>(argv[3]);
			Real e = (x + y + z) / 3;
			mPtr->Ka = vector < Real > {x, y, z, e};
			continue;
		}
		if (argv[0] == "Kd" && toggle) {
			Real x, y, z;
			x = boost::lexical_cast<Real>(argv[1]);
			y = boost::lexical_cast<Real>(argv[2]);
			z = boost::lexical_cast<Real>(argv[3]);
			Real e = (x + y + z) / 3;
			mPtr->Kd = vector < Real > {x, y, z, e};
			if (e > Limit::Epsilon) {
				mPtr->setRd();
			}
			continue;
		}
		if (argv[0] == "Ks" && toggle) {
			Real x, y, z;
			x = boost::lexical_cast<Real>(argv[1]);
			y = boost::lexical_cast<Real>(argv[2]);
			z = boost::lexical_cast<Real>(argv[3]);
			Real e = (x + y + z) / 3;
			mPtr->Ks = vector < Real > {x, y, z, e};
			if (e > Limit::Epsilon) {
				mPtr->setRs();
			}
			continue;
		}
		if (argv[0] == "Ns" && toggle) {
			mPtr->Ns = boost::lexical_cast<int>(argv[1]);
			continue;
		}
		if (argv[0] == "Tr" && toggle) {
			Real Tr = boost::lexical_cast<Real>(argv[1]);
			if (Tr > 0) {
				mPtr->setTr();
			}
			continue;
		}
		if (argv[0] == "Td" && toggle) {
			Real x, y, z;
			x = boost::lexical_cast<Real>(argv[1]);
			y = boost::lexical_cast<Real>(argv[2]);
			z = boost::lexical_cast<Real>(argv[3]);
			Real e = (x + y + z) / 3;
			mPtr->Td = vector < Real > {x, y, z, e};
			if (e > Limit::Epsilon) {
				mPtr->setTd();
			}
			continue;
		}
		if (argv[0] == "Ts" && toggle) {
			Real x, y, z;
			x = boost::lexical_cast<Real>(argv[1]);
			y = boost::lexical_cast<Real>(argv[2]);
			z = boost::lexical_cast<Real>(argv[3]);
			Real e = (x + y + z) / 3;
			mPtr->Ts = vector < Real > {x, y, z, e};
			if (e > Limit::Epsilon) {
				mPtr->setTs();
			}
			continue;
		}
		if (argv[0] == "Nst" && toggle) {
			mPtr->Nst = boost::lexical_cast<int>(argv[1]);
			continue;
		}
		if (argv[0] == "n" && toggle) {
			mPtr->n = boost::lexical_cast<Real>(argv[1]);
			continue;
		}
		if (argv[0] == "bump" && toggle) {
			mPtr->texturePtr = new Texture();
			mPtr->texturePtr->loadBump(argv[1].c_str());
			mPtr->setBump();
			continue;
		}
		if (argv[0] == "texture" && toggle) {
			mPtr->texturePtr = new Texture();
			mPtr->texturePtr->loadTexture(argv[1].c_str());
			mPtr->setTexture();
			mPtr->setTextureKd();
			continue;
		}
		if (argv[0] == "BRDF" && toggle) {
			// todo;
			continue;
		}
	}
	if (toggle) {
		materials.insert(std::pair<std::string, MaterialPtr>(lastMtlName, mPtr));
	}
}

void Object::printInfo() {
	for (int i = 0; i < vertices.size(); i++) {
		std::cout << "v_" << i << ": " << vertices[i].v << "\n";
	}
	for (int i = 0; i < faces.size(); i++) {
		/*std::cout << "f_" << i << ": " << faces[i].v[0] << "/ " << faces[i].v[1] << "/ " << faces[i].v[2] <<
			"; Material: " << faces[i].materialPtr->name << "\n";*/
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
// compute for faces if !interpolation.
void Object::computeNormals(bool interpolation) {
	int offset = normals.size();
	if (!interpolation) {
		int n = faces.size();
		for (int i = 0; i < n; ++i) {
			Vector3R E1 = vertices[faces[i].v[1]].v - vertices[faces[i].v[0]].v;
			Vector3R E2 = vertices[faces[i].v[2]].v - vertices[faces[i].v[0]].v;
			normals.push_back(VN(E1.cross(E2)));
			faces[i].vn = std::vector<int>(3, offset);
			if (faces[i].vt.empty()) {
				faces[i].vt = std::vector<int>(3, 0);
			}
			++offset;
		}
	}
	int n = normals.size();
	for (int i = 0; i < n; ++i) {
		normals[i].v.normalize();
	}
	std::ofstream os("normals.n");
	for (int i = 0; i < vertices.size(); ++i) {
		os << "v " << vertices[i].v[0] << ' ' << vertices[i].v[1] << ' ' << vertices[i].v[2] << '\n';
	}
	for (int i = 0; i < textures.size(); ++i) {
		os << "vt " << textures[i].v[0] << ' ' << textures[i].v[1] << ' ' << textures[i].v[2] << '\n';
	}
	for (int i = 0; i < normals.size(); ++i) {
		os << "vn " << normals[i].v[0] << ' ' << normals[i].v[1] << ' ' << normals[i].v[2] << '\n';
	}
	for (int i = 0; i < faces.size(); ++i) {
		os << "f " << faces[i].v[0] << '/' << faces[i].vt[0] << '/' << faces[i].vn[0] << ' '
			<< faces[i].v[1] << '/' << faces[i].vt[1] << '/' << faces[i].vn[1] << ' '
			<< faces[i].v[2] << '/' << faces[i].vt[2] << '/' << faces[i].vn[2] << '\n';
	}
	os.close();
}

#pragma endregion


