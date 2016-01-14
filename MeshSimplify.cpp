#include "Object.h"
bool collapse(F f) {
	return f.v[0] == f.v[1] || f.v[0] == f.v[2] || f.v[1] == f.v[2];
}

struct Edge {
	int v0 = 0, v1 = 0;
	Edge(int _0, int _1) { v0 = std::min(_0, _1); v1 = std::max(_0, _1); }
};

bool operator < (const Edge &lhs, const Edge &rhs) {
	return (lhs.v0 < rhs.v0) || ((lhs.v0 == rhs.v0) && (lhs.v1 < rhs.v1));
}

// wrapper class
struct EdgePtr {
	Edge *p;
	double length = 0;
	EdgePtr(const Edge& e) { p = new Edge(e); }
	EdgePtr(const EdgePtr& ep) { p = ep.p; length = ep.length; }
};

bool operator < (const EdgePtr &lhs, const EdgePtr &rhs) {
	return (lhs.length < rhs.length) ||
		(lhs.length == rhs.length && *(lhs.p) < *(rhs.p));
}


void Object::meshSimplify(int n, bool preserveBoundary) {
	printf("Simplifying...");
	vector<double> minInnerProductOfVertex(vertices.size(), 1);
	if (preserveBoundary) {
		computeNormals(false);
		vector<vector<Vector3R>> normalsOfVertex(vertices.size());
		for (int i = 0; i < faces.size(); i++) {
			for (int j = 0; j < 3; j++) {
				normalsOfVertex[faces[i].v[j]].push_back(normals[faces[i].vn[j]].v);
			}
		}
		for (int i = 0; i < vertices.size(); i++) {
			for (int j = 0; j < normalsOfVertex[i].size(); j++) {
				for (int k = j + 1; k < normalsOfVertex[i].size(); k++) {
					minInnerProductOfVertex[i] = std::min(minInnerProductOfVertex[i],
						abs(normalsOfVertex[i][j].dot(normalsOfVertex[i][k])));
				}
			}
		}
	}

	std::vector<std::set<int>> facesAdjToVertex(vertices.size());
	for (int i = 0; i < faces.size(); i++) {
		for (int j = 0; j < 3; j++) {
			facesAdjToVertex[faces[i].v[j]].insert(i);
		}
	}

	// now we can access faces through vertices
	std::set<Edge> edgeSet;
	for (int i = 0; i < faces.size(); i++) {
		edgeSet.insert(Edge(faces[i].v[0], faces[i].v[1]));
		edgeSet.insert(Edge(faces[i].v[0], faces[i].v[2]));
		edgeSet.insert(Edge(faces[i].v[1], faces[i].v[2]));
	}

	std::set<EdgePtr> edgePtrSet;
	for (auto iter = edgeSet.begin(); iter != edgeSet.end(); iter++) {
		EdgePtr ep = EdgePtr(*iter);
		ep.length = (vertices[ep.p->v0].v - vertices[ep.p->v1].v).norm();
		edgePtrSet.insert(ep);
	}
	edgeSet.clear();
	// now we have a set of edgesptrs with their length

	std::vector<std::set<EdgePtr>> edgesAdjToVertex(vertices.size());
	for (auto iter = edgePtrSet.begin(); iter != edgePtrSet.end(); iter++) {
		edgesAdjToVertex[iter->p->v0].insert(*iter);
		edgesAdjToVertex[iter->p->v1].insert(*iter);
	}
	// now we can access edges through vertices
	
	std::vector<bool> faceAlive(faces.size(), true);
	int numFaces = faces.size();
	
	// ready for loop
	while (numFaces > n) {
		
		EdgePtr min(*edgePtrSet.begin());

		edgePtrSet.erase(min);
		
		edgesAdjToVertex[min.p->v0].erase(min);
		edgesAdjToVertex[min.p->v1].erase(min);
		if (preserveBoundary) {
			if (minInnerProductOfVertex[min.p->v0] < 0.5) {
				if (minInnerProductOfVertex[min.p->v1] < 0.5) {
					vertices[min.p->v0].v = 0.5 * (vertices[min.p->v0].v + vertices[min.p->v1].v);
				}
				else {
					;
				}
			}
			else {
				if (minInnerProductOfVertex[min.p->v1] > 0.5) {
					vertices[min.p->v0].v = 0.5 * (vertices[min.p->v0].v + vertices[min.p->v1].v);
				}
				else {
					vertices[min.p->v0].v = vertices[min.p->v1].v;
					minInnerProductOfVertex[min.p->v0] = minInnerProductOfVertex[min.p->v1];
				}
			}
		}
		else {
			vertices[min.p->v0].v = 0.5 * (vertices[min.p->v0].v + vertices[min.p->v1].v);
		}
		
		std::set<int> deadfaces;
		for (int f : facesAdjToVertex[min.p->v1]) {
			for (int i = 0; i < 3; i++) {
				if (faces[f].v[i] == min.p->v1) {
					faces[f].v[i] = min.p->v0;
				}
				else if (faces[f].v[i] == min.p->v0) {
					faceAlive[f] = false;
					deadfaces.insert(f);
					numFaces--;
				}
			}
			if (faceAlive[f]) {
				facesAdjToVertex[min.p->v0].insert(f);
			}
		}
		for (int f : deadfaces) {
			for (int v : faces[f].v) {
				facesAdjToVertex[v].erase(f);
			}
		}
		deadfaces.clear();
		// maintaing faces

		std::set<int> connectVertices;
		for (EdgePtr e : edgesAdjToVertex[min.p->v1]) {
			if (e.p->v0 == min.p->v1) {
				connectVertices.insert(e.p->v1);
				edgesAdjToVertex[e.p->v1].erase(e);
			}
			else if (e.p->v1 == min.p->v1) {
				connectVertices.insert(e.p->v0);
				edgesAdjToVertex[e.p->v0].erase(e);
			}
			edgePtrSet.erase(e);
		}
		for (EdgePtr e : edgesAdjToVertex[min.p->v0]) {
			if (e.p->v0 == min.p->v0) {
				connectVertices.insert(e.p->v1);
				edgesAdjToVertex[e.p->v1].erase(e);
			}
			else if (e.p->v1 == min.p->v0) {
				connectVertices.insert(e.p->v0);
				edgesAdjToVertex[e.p->v0].erase(e);
			}
			edgePtrSet.erase(e);
		}
		edgesAdjToVertex[min.p->v0].clear();
		edgesAdjToVertex[min.p->v1].clear();
		// find all connect vertices

		for (int v : connectVertices) {
			Edge e(min.p->v0, v);
			EdgePtr ep(e);
			ep.length = (vertices[min.p->v0].v - vertices[v].v).norm();
			edgesAdjToVertex[v].insert(ep);
			edgesAdjToVertex[min.p->v0].insert(ep);
			edgePtrSet.insert(ep);
		}
	}

	printf("Complete.\n");
	std::ofstream os("images/simplified.obj");
	for (int i = 0; i < vertices.size(); ++i) {
		os << "v " << vertices[i].v[0] << ' ' << vertices[i].v[1] << ' ' << vertices[i].v[2] << '\n';
	}
	for (int i = 0; i < faces.size(); ++i) {
		if (faceAlive[i]) {
			os << "f " << faces[i].v[0] + 1 << ' ' << faces[i].v[1] + 1 << ' ' << faces[i].v[2] + 1 << '\n';
		}
	}
	os.close();
}


