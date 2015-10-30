#include "stdafx.h"
#include "Object.h"
#include "Scene.h"
#include "Screen.h"
#include "IntersectionTester.h"
#include "RayTracer.h"
#include "renderer.h"

void demo_1();

int main() {
	
	Scene scene;
	scene.loadObject("model.obj");
	scene.constructScene();
	PixelRenderer pr;
	RenderSetting rs("render.cfg");
	ImageRenderer ir(&rs);
	ir.renderImage();
	return 0;
}
