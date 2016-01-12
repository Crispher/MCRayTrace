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
				z = boost::lexical_cast<Real>(0);
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
	Kd = m.Kd; Ks = m.Ks;
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
			//mPtr->Ka = vector < Real > {x, y, z, e};
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
			for (int i = 2; i < argv.size(); ++i) {
				if (argv[i] == "Kd") {
					mPtr->setTextureKd();
				}
				if (argv[i] == "Ks") {
					mPtr->setTextureKs();
				}
				if (argv[i] == "Td") {
					mPtr->setTextureTd();
				}
				if (argv[i] == "Ts") {
					mPtr->setTextureTs();
				}
			}
			continue;
		}
		if (argv[0] == "BRDF" && toggle) {
			// todo;
			continue;
		}
		if (argv[0] == "scatter" && toggle) {
			mPtr->setScatter();
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
	printf("Rotating...\n");
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
	for (int i = 0; i < vertices.size(); i++) {
		Real v1 = vertices[i].v[axis1], v2 = vertices[i].v[axis2];
		vertices[i].v[axis1] = c*v1 + _s*v2;
		vertices[i].v[axis2] = s*v1 + c*v2;
	}
	for (int i = 0; i < normals.size(); i++) {
		Real v1 = normals[i].v[axis1], v2 = normals[i].v[axis2];
		normals[i].v[axis1] = c*v1 + _s*v2;
		normals[i].v[axis2] = s*v1 + c*v2;
	}
	printf("Rotation complete\n");
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

void Object::relocate(Real minx, Real miny, Real minz) {
	printf("Relocating...\n");
	Real mx = Limit::Infinity, my = Limit::Infinity, mz = Limit::Infinity;
	for (int i = 0; i < vertices.size(); ++i) {
		mx = std::min(vertices[i].v[0], mx);
		my = std::min(vertices[i].v[1], my);
		mz = std::min(vertices[i].v[2], mz);
	}
	Real dx = minx - mx, dy = miny - my, dz = minz - mz;
	for (int i = 0; i < vertices.size(); ++i) {
		vertices[i].v[0] += dx;
		vertices[i].v[1] += dy;
		vertices[i].v[2] += dz;
	}
	printf("Relocation complete.\n");
}
void Object::scale(Real ratio) {
	for (int i = 0; i < vertices.size(); ++i) {
		vertices[i].v *= ratio;
	}
}

bool collapse(F f) {
	return f.v[0] == f.v[1] || f.v[0] == f.v[2] || f.v[1] == f.v[2];
}

struct Edge {
	int v0, v1;
	double length = 0;
	Edge(int _0, int _1) { v0 = std::min(_0, _1); v1 = std::max(_0, _1); }
};
typedef Edge* EdgePtr;

struct Comparator {
	Comparator() {}
	bool operator() (const EdgePtr &lhs, const EdgePtr &rhs) const {
		return lhs->length > rhs->length;
	}
};
/*
void Object::meshSimplify(int n) {
	std::vector<std::vector<int>> facesAdjToVertex(vertices.size());
	std::vector<bool> faceRemain(true, faces.size());
	for (int i = 0; i < faces.size(); i++) {
		for (int j = 0; j < 3; j++) {
			facesAdjToVertex[faces[i].v[j]].push_back(i);
		}
	}
	// now we can access faces through vertices

	std::set<Edge> edgeSet;
	for (int i = 0; i < faces.size(); i++) {
		edgeSet.insert(Edge(faces[i].v[0], faces[i].v[1]));
		edgeSet.insert(Edge(faces[i].v[0], faces[i].v[2]));
		edgeSet.insert(Edge(faces[i].v[1], faces[i].v[2]));
	}
	std::vector<Edge*> edgeList;
	for (auto iter = edgeSet.begin(); iter != edgeSet.end(); iter++) {
		edgeList.push_back(new Edge(*iter));
	}
	edgeSet.clear();
	// now we have a list of edges

	std::vector<std::vector<Edge*>> edgesAdjToVertex(vertices.size());
	for (int i = 0; i < edgeList.size(); i++) {
		edgesAdjToVertex[edgeList[i]->v0].push_back(edgeList[i]);
		edgesAdjToVertex[edgeList[i]->v1].push_back(edgeList[i]);
	}
	// now we can access edges through vertices

	for (int i = 0; i < edgeList.size(); i++) {
		edgeList[i]->length = (vertices[edgeList[i]->v0].v - vertices[edgeList[i]->v1].v).norm();
	}
	// now we have length of each edge

	Comparator comp();
	
	// now we have a priority queue

	// ready for loop
}

*/

//
//void Object::meshSimplify(int n) {
//	std::set<Edge> edges;
//	for (int i = 0; i < faces.size(); i++) {
//		edges.insert(Edge(faces[i].v[0], faces[i].v[1]));
//		edges.insert(Edge(faces[i].v[0], faces[i].v[2]));
//		edges.insert(Edge(faces[i].v[1], faces[i].v[2]));
//	}
//	std::vector<std::vector<int>> facesAdjVertex(vertices.size());
//	std::vector<bool> faceRemain(true, faces.size());
//	for (int i = 0; i < faces.size(); i++) {
//		for (int j = 0; j < 3; j++) {
//			facesAdjVertex[faces[i].v[j]].push_back(i);
//		}
//	}
//	int facesRemain = faces.size();
//
//	printf("%d faces\n", faces.size());
//	while (faces.size() > n) {
//		if (faces.size() % 1000 < 1)
//			printf("%d faces remain.\n", faces.size());
//		Edge min = *edges.begin();
//		Real dmin = (vertices[min.v0].v - vertices[min.v1].v).norm();
//		for (auto iter = edges.begin(); iter != edges.end(); iter++) {
//			Edge temp = *iter;
//			if ((vertices[min.v0].v - vertices[min.v1].v).norm() < dmin) {
//				min = temp;
//				dmin = (vertices[min.v0].v - vertices[min.v1].v).norm();
//			}
//		}
//		vertices[min.v0].v = (vertices[min.v1].v = 0.5 * (vertices[min.v0].v + vertices[min.v1].v));
//
//		for (int f : facesAdjVertex[min.v1]) {
//			for (int i = 0; i < 3; i++) {
//				if (faces[f].v[i] == min.v0) {
//					faceRemain[f] = false;
//					facesRemain--;
//				}
//				if (faces[f].v[i] == min.v1) {
//					faces[f].v[i] = min.v0;
//				}
//			}
//		}
//
//		std::set<Edge> toDelete;
//		for (auto iter = edges.begin(); iter != edges.end(); iter++) {
//			if (iter->v0 == min.v1 || iter->v1 == min.v1) {
//				toDelete.insert(*iter);
//			}
//		}
//		for (auto iter = toDelete.begin(); iter != toDelete.end(); iter++) {
//			edges.erase(*iter);
//		}
//
//		auto rem = std::remove_if(faces.begin(), faces.end(), collapse);
//		faces.erase(rem, faces.end());
//	}
//
//	std::stringstream ss;
//	ss << "images/" << n << "_simplifed.obj";
//	std::string s;
//	ss >> s;
//	std::ofstream os(s.c_str());
//	for (int i = 0; i < vertices.size(); ++i) {
//		os << "v " << vertices[i].v[0] << ' ' << vertices[i].v[1] << ' ' << vertices[i].v[2] << '\n';
//	}
//	for (int i = 0; i < faces.size(); ++i) {
//		os << "f " << faces[i].v[0]+1 << ' ' << faces[i].v[1]+1 << ' ' << faces[i].v[2]+1 << '\n';
//	}
//	os.close();
//}

#pragma endregion


