#include "physics.h"
#include "Object.h"
#include "Scene.h"

bool collides(SphereBody& body1, TriangleBody& body2, Real collision_damping);
bool collides(SphereBody& body1, SphereBody& body2, Real collision_damping);

Physics::Physics(Scene* scenePtr) {
	for (int i = 0; i < scenePtr->spheres.size(); i++) {
		spheres.push_back(new SphereBody(scenePtr, i));
	}
	for (int i = 0; i < scenePtr->faces.size(); i++) {
		triangles.push_back(new TriangleBody(&scenePtr->faces[i], scenePtr));
	}
	collision_damping = 0;
	gravity = Vector3R(0, 0, -2);
}

Physics::~Physics() {

}

SphereBody::SphereBody(Scene *_s, int _id) : scenePtr(_s), sphereId(_id) {
	radius = _s->spheres[_id].r;
	position = _s->spheres[_id].center;
	mass = 1;
	velocity = Vector3R(0, 0, 0);
}

TriangleBody::TriangleBody(F* _f, Scene* _s) {
	for (int i = 0; i < 3; i++) {
		vertices.push_back(_s->vertices[_f->v[i]].v);
	}
}

void Physics::step(Real dt)
{
	for (int i = 0; i < spheres.size(); i++) {
		for (int j = 0; j < triangles.size(); j++) {
			collides(*spheres[i], *triangles[j], collision_damping);
		}

		for (int j = i + 1; j < spheres.size(); j++) {
			collides(*spheres[i], *spheres[j], collision_damping);
		}
	}
	for (int i = 0; i < spheres.size(); i++) {
		spheres[i]->applyForce(gravity);
		spheres[i]->stepPosition(dt, 0);
	}
}

bool collides(SphereBody& body1, SphereBody& body2, Real collision_damping)
{
	Vector3R r_12 = body1.position - body2.position;
	Vector3R v_12 = body1.velocity - body2.velocity;
	if (r_12.dot(v_12) > 0) {
		// moving against each other
		return false;
	}
	// moving towards each other
	if (r_12.norm() < body1.radius + body2.radius) {
		// they contacts
		Vector3R d = -r_12.normalized();
		Vector3R v_21_pp = 2 * d * body1.mass / (body1.mass + body2.mass) * d.dot(v_12);
		Vector3R u_2 = body2.velocity + v_21_pp;
		body1.velocity = (body1.mass * body1.velocity + body2.mass * body2.velocity - body2.mass * u_2) / body1.mass;
		body2.velocity = u_2;
		body1.velocity *= (1 - collision_damping);
		body2.velocity *= (1 - collision_damping);
		return true;
	}

	return false;
}

bool collides(SphereBody& body1, TriangleBody& body2, Real collision_damping)
{
	// TODO detect collision. If there is one, update velocity
	Vector3R e1 = body2.vertices[2] - body2.vertices[0],
		e2 = body2.vertices[1] - body2.vertices[0];
	Vector3R normal = e1.cross(e2).normalized();
	Vector3R r_pos = body1.position - body2.vertices[0];
	if (r_pos.dot(normal) < 0) {
		normal = -normal;
	}
	Real distance = abs(r_pos.dot(normal));
	if (body1.velocity.dot(normal) > 0) {
		return false;
	}
	//Vector3R proj = body1.position - distance * normal;
	//Vector3R p1 = body2.vertices[0] - proj;
	//Vector3R p2 = body2.vertices[1] - proj;
	//Vector3R p3 = body2.vertices[2] - proj;
	//Vector3R c12 = p1.cross(p2);
	//Vector3R c23 = p2.cross(p3);
	//Vector3R c31 = p3.cross(p1);
	//Real d2 = c12.dot(c23);
	//Real d3 = c23.dot(c31);
	//Real d1 = c31.dot(c12);
	//if (d2 * d3 < 0 || d1 * d3 < 0) {
	//	return false; //reject if projection is not inside the triangle.
	//}
	if (distance < body1.radius) {
		body1.velocity = reflect(body1.velocity, normal);
		body1.velocity *= (1 - collision_damping);
		return true;
	}
	return false;
}

void SphereBody::stepPosition(Real dt, Real motion_damping) {
	velocity += force / mass * dt;
	position += velocity * dt;
	std::cout << position << std::endl;
	scenePtr->spheres[sphereId].center = position;
	force = Vector3R(0, 0, 0);
	return;
}

void SphereBody::applyForce(const Vector3R& f) {
	force += f;
}