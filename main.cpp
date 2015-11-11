#include "stdafx.h"
#include "Object.h"
#include "Scene.h"
#include "Screen.h"
#include "IntersectionTester.h"
#include "RayTracer.h"
#include "renderer.h"

int main() {
	RenderSetting rs("render.cfg");
	ImageRenderer ir(&rs);
	ir.renderImage();
	return 0;
}
