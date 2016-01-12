#include "stdafx.h"
#include "Object.h"
#include "Scene.h"
#include "Screen.h"
#include "IntersectionTester.h"
#include "RayTracer.h"
#include "renderer.h"

int main() {/*
	Object object;
	int n = object.faces.size();
	object.load("test_data/arma.obj");
	object.meshSimplify(0.4 * n);
	return 0;
	
	for (Real r = 0.9; r > 0.05; r -= 0.1) {
		object.meshSimplify(r * n);
	}
	return 0;*/
	
	RenderSetting rs("renderconfig2/render.cfg");
	ImageRenderer ir(&rs);
	ir.renderImage();
	return 0;
}
