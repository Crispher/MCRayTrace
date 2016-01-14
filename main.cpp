#include "stdafx.h"
#include "Object.h"
#include "Scene.h"
#include "Screen.h"
#include "IntersectionTester.h"
#include "RayTracer.h"
#include "renderer.h"

int main() {
	Object object;
	int n = object.faces.size();
	object.load("test_data/block.obj");
	object.meshSimplify(10000, false);
	return 0;
	
	
	RenderSetting rs("renderconfig2/render.cfg");
	ImageRenderer ir(&rs);
	ir.renderImage();
	return 0;
}
