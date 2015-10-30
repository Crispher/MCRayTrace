#include "Scene.h"
#include "stdafx.h"
#include "Object.h"
using namespace boost::numeric;
//#define DEBUG_LOG
#define SHOW_PROGRESS
#pragma region CAMERA

Camera::Camera(Real px, Real py, Real pz,
	Real dx, Real dy, Real dz,
	Real ux, Real uy, Real uz,
	Real foco,
	Real w, Real h,
	Real s) {

	position = Vector3R();
	position[0] = px; position[1] = py; position[2] = pz;

	direction = Vector3R();
	direction[0] = dx; direction[1] = dy; direction[2] = dz;

	up = Vector3R();
	up[0] = ux; up[1] = uy; up[2] = uz;
	up.normalize();

	right = direction.cross(up);

	focolength = foco;
	width = w; height = h;
	step = s;
}

Camera Camera::load(const char* filename) {
	std::ifstream in = std::ifstream(filename);
	Real x, y, z, dx, dy, dz, ux, uy, uz, foco, w, h, step;
	in >> x >> y >> z >> dx >> dy >> dz >> ux >> uy >> uz >> foco >> w >> h >> step;
	return Camera(x, y, z, dx, dy, dz, ux, uy, uz, foco, w, h, step);
}

void Camera::printInfo() {
	std::cout << "Located at: " << position << "\n";
	std::cout << "direction: " << direction << "\n" <<
		"up: " << up << "\n" <<
		"right" << right << "\n" <<
		"focolength: " << focolength << "\n" <<
		"width, height: " << width << ", " << height << "\n" <<
		"step: " << step << "\n";
}

#pragma endregion

void Scene::constructScene() {
	vertices = std::vector<V, Eigen::aligned_allocator<Vector3R>>(objects[0].vertices);
	normals = std::vector<VN, Eigen::aligned_allocator<Vector3R>>(objects[0].normals);
	textures = std::vector<VT, Eigen::aligned_allocator<Vector3R>>(objects[0].textures);
	faces = std::vector<F, Eigen::aligned_allocator<Vector3R>>(objects[0].faces);
	spheres = std::vector<Sphere, Eigen::aligned_allocator<Vector3R>>(objects[0].spheres);
	materials = std::map<std::string, MaterialPtr>(objects[0].materials);
}

#pragma region SCENE_RENDER

void Scene::loadObject(const char* filename) {
	Object object;
	object.load(filename);
	objects.push_back(object);
}

void Scene::loadSceneSettings(const char *filename) {
	std::ifstream in = std::ifstream(filename);
	std::string line;
	while (std::getline(in, line)) {
		std::vector<std::string> argv;
		boost::split(argv, line, boost::is_any_of("\t "));
		
		if (argv[0] == "lightsource") {
			// todo;
		}
		if (argv[0] == "object") {
			loadObject(argv[1].c_str());
		}
	}
	constructScene();
}

#pragma endregion