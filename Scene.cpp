#include "Scene.h"
#include "stdafx.h"
#include "Object.h"
#include "Sampler.h"
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

Scene::Scene(Sampler2D *_s) : samplerPtr(_s) {}

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

	// put lightsources at the beginning of the array
	faces.insert(faces.end(), faces_temp_ls.begin(), faces_temp_ls.end());
	faces.insert(faces.end(), faces_temp_nls.begin(), faces_temp_nls.end());

	printf("looking for scattering mtl\n");
	auto iter = objects[0].materials.find("GlobalScatter");
	if (iter == objects[0].materials.end()) {
		printf("No scattering media.\n");
		scatterMtl = nullptr;
	}
	else {
		scatterMtl = objects[0].materials["GlobalScatter"];
		scatterMtl->setScatter();
	}
	printf("Scene construction complete, %d faces\n", faces.size());
	objects.clear();
}

#pragma region SCENE_RENDER

void Scene::loadObject(const char* filename) {
	Object object;
	object.load(filename);
	objects.push_back(object);
}

std::vector<Vector3R, Eigen::aligned_allocator<Vector3R>> Scene::getLightSourceSamples(int index, MaterialPtr &mPtr, Vector3R &normal, Real &area) {
	std::vector<Sample2D> samples = samplerPtr->sampleTriangle(1);
	std::vector<Vector3R, Eigen::aligned_allocator<Vector3R>> samplePos(samples.size());
	for (int i = 0; i < samples.size(); i++) {
		samplePos[i] = (1 - samples[i].u - samples[i].v) * vertices[faces[index].v[0]].v + 
			samples[i].u * vertices[faces[index].v[1]].v + 
			samples[i].v * vertices[faces[index].v[2]].v;
	}
	normal = normals[faces[index].vn[0]].v;
	mPtr = faces[index].materialPtr;
	Vector3R e1 = vertices[faces[index].v[1]].v - vertices[faces[index].v[0]].v,
		e2 = vertices[faces[index].v[2]].v - vertices[faces[index].v[0]].v;
	area = 0.5 * e1.cross(e2).norm();
	return samplePos;
}

#pragma endregion