#pragma once
#include "stdafx.h"
#include "Screen.h"
#include "Scene.h"
#include "RayTracer.h"
#include "Sampler.h"
#include "IntersectionTester.h"

class PixelRenderer {
public:
	Camera *cameraPtr;
	RayTracer *rayTracerPtr;
    Sampler2D *samplerPtr;
	int sampleSize;

	PixelRenderer() {}
	Color renderPixel(int i, int j);
private:
	Color reconstruct(const std::vector<Sample2D>&);
};

class RenderSetting {
public:
	Scene *scenePtr;
	Camera *cameraPtr;

	std::string intersectiontester;
	std::string rayTracerSampler;
	std::string pixelSampler;

	int rayTraceDepth;
	int BRDF_sampleSize;
	int pixelSampleSize;
	int threading = 1;

	int imageWidth, imageHeight;
	Real Er = 1, Eg = 1, Eb = 1;
	bool writeRaw = false;
	bool fromRaw = false;
	std::string rawFile;
	RenderSetting(const char*);
};

struct ThreadingTask {
	std::vector<std::vector<Color>> bitmap;
	int start, end, h; // [ , )
	ThreadingTask() {}
	ThreadingTask(int, int, int);
};

class ImageRenderer
{
	RenderSetting *renderSetting;
	
public:
	ImageRenderer(RenderSetting *_r) : renderSetting(_r) {}
	~ImageRenderer(){};
	void renderImageThreading(ThreadingTask&);
	void renderImage();
	void fromRaw(const char*);
};
