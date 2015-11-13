#include "Scene.h"
#include "stdafx.h"
#include "Object.h"
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
	direction.normalize();

	up = Vector3R();
	up[0] = ux; up[1] = uy; up[2] = uz;
	up.normalize();

	right = direction.cross(up);

	focolength = foco;
	width = w; height = h;
	step = s;
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
	printf("Constructing scene... %d objects.\n", objects.size());
	std::vector<F> faces_temp_ls, faces_temp_nls;
	
	int n = objects.size();
	int Voffset = 0, Noffset = 0, Toffset = 0;
	for (int i = 0; i < n; i++) {
		vertices.insert(vertices.end(), objects[i].vertices.begin(), objects[i].vertices.end());
		normals.insert(normals.end(), objects[i].normals.begin(), objects[i].normals.end());
		textures.insert(textures.end(), objects[i].textures.begin(), objects[i].textures.end());
		spheres.insert(spheres.end(), objects[i].spheres.begin(), objects[i].spheres.end());
		materials.insert(objects[i].materials.begin(), objects[i].materials.end());
		for (int j = 0; j < objects[i].faces.size(); ++j) {
			F temp = objects[i].faces[j];
			for (int k = 0; k < temp.v.size(); ++k) {
				temp.v[k] += Voffset;
			}
			for (int k = 0; k < temp.vn.size(); ++k) {
				temp.vn[k] += Noffset;
			}
			for (int k = 0; k < temp.vt.size(); ++k) {
				temp.vt[k] += Toffset;
			}
			if (objects[i].faces[j].materialPtr->isLightSource()) {
				faces_temp_ls.push_back(temp);
				++numLightSources;
			}
			else {
				faces_temp_nls.push_back(temp);
			}
		}
		Voffset = vertices.size();
		Noffset = normals.size();
		Toffset = textures.size();
	}
	faces.insert(faces.end(), faces_temp_ls.begin(), faces_temp_ls.end());
	faces.insert(faces.end(), faces_temp_nls.begin(), faces_temp_nls.end());
	printf("Scene construction complete, %d faces\n", faces.size());
}

#pragma region SCENE_RENDER

void Scene::loadObject(const char* filename) {
	Object object;
	object.load(filename);
	objects.push_back(object);
}

#pragma endregion