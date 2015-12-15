#include "Renderer.h"
#include "stdafx.h"
#include "Kdtree.h"
#define SHOW_PROGRESS

#pragma region PIXEL_RENDERER
Color PixelRenderer::renderPixel(int x, int y) {
	Vector3R pixel;
	std::vector<Sample2R> samples2R = samplerPtr->sample_UnitSquare_Uniform(sampleSize);
	int n = samples2R.size();
	if (!cameraPtr->DOF) {
		for (int i = 0; i < n; i++) {
			Real offsetI = (samples2R[i].u + x) * cameraPtr->step - cameraPtr->width / 2;
			Real offsetJ = (samples2R[i].v + y) * cameraPtr->step - cameraPtr->height / 2;
			pixel = cameraPtr->position + cameraPtr->direction * cameraPtr->focolength + cameraPtr->right * offsetI + cameraPtr->up * offsetJ;
			Ray ray = Ray::fromPoints(cameraPtr->position, pixel);
			samples2R[i].value = rayTracerPtr->rayTrace_Single(ray, rayTracerPtr->depth);
		}
	}
	else {
		for (int i = 0; i < n; i++) {
			Real offsetI = (samples2R[i].u + x) * cameraPtr->step - cameraPtr->width / 2;
			Real offsetJ = (samples2R[i].v + y) * cameraPtr->step - cameraPtr->height / 2;
			pixel = cameraPtr->position + cameraPtr->direction * cameraPtr->focolength + cameraPtr->right * offsetI + cameraPtr->up * offsetJ;
			Vector3R focoPoint = (pixel - cameraPtr->position) * (cameraPtr->focalPlane / cameraPtr->focolength) + cameraPtr->position;
			Vector3R samplePixel = pixel + (samples2R[i].u - 0.5) * cameraPtr->aperture * cameraPtr->right + (samples2R[i].v - 0.5) * cameraPtr->up * cameraPtr->aperture;
			Ray ray = Ray::fromPoints(samplePixel, focoPoint);
			samples2R[i].value = rayTracerPtr->rayTrace_Single(ray, rayTracerPtr->depth);
		}
	}
	return reconstruct(samples2R);
}

// Gaussian filter
Color PixelRenderer::reconstruct(const std::vector<Sample2R>& samples2R) {
	Real weightSum = 0;
	Real r = 0, g = 0, b = 0;
	int n = samples2R.size();
	for (int i = 0; i < n; i++) {
		Real weight = exp(-((samples2R[i].u - 0.5) * (samples2R[i].u - 0.5) + (samples2R[i].v - 0.5) * (samples2R[i].v - 0.5)));
		weightSum += weight;
		r += std::max((Real)0, samples2R[i].value.red() * weight);
		g += std::max((Real)0, samples2R[i].value.green() * weight);
		b += std::max((Real)0, samples2R[i].value.blue() * weight);
	}
	return Color(r / weightSum, g / weightSum, b / weightSum);
}

# pragma endregion

# pragma region IMAGE_RENDERER

void ImageRenderer::renderImage() {
	if (renderSetting->fromRaw) {
		fromRaw(renderSetting->rawFile.c_str());
		return;
	}

	int w = renderSetting->imageWidth;
	int h = renderSetting->imageHeight;
	Screen screen(w, h);

	// unified procedure. creating tasks for threads
	std::vector<ThreadingTask> tasks(renderSetting->threading);
	int dw = w / renderSetting->threading;
	int i = 0;
	for (; i < renderSetting->threading - 1; i++) {
		tasks[i] = ThreadingTask(i * dw, (i + 1) * dw, h);
	}
	tasks[i] = ThreadingTask(i * dw, w, h);

	// create and run threads
	std::vector<std::thread> threads(renderSetting->threading);
	for (i = 0; i < renderSetting->threading; i++) {
		threads[i] = std::thread(&ImageRenderer::renderImageThreading, this, std::ref(tasks[i]));
	}

	// join
	for (i = 0; i < renderSetting->threading; i++) {
		threads[i].join();
	}

	// merge result
	for (i = 0; i < renderSetting->threading; i++) {
		for (int a = tasks[i].start; a < tasks[i].end; a++) {
			for (int j = 0; j < h; j++) {
				screen.color = tasks[i].bitmap[a - tasks[i].start][j].filter(renderSetting->Er, renderSetting->Eg, renderSetting->Eb);
				screen.drawPixel(a, j);
			}
		}
	}
	if (renderSetting->writeRaw) {
		std::ofstream os(renderSetting->rawFile);
		os << renderSetting->imageWidth << ' ' << renderSetting->imageHeight << std::endl;
		for (i = 0; i < renderSetting->threading; i++) {
			for (int a = tasks[i].start; a < tasks[i].end; a++) {
				for (int j = 0; j < h; j++) {
					Color color = tasks[i].bitmap[a - tasks[i].start][j];
					os << color.red() << ' ' << color.green() << ' ' << color.blue() << std::endl;
				}
			}
		}
		os.close();
	}
	screen.show();
	printf("Adjust your camera:\n");
	std::string s; double d;
	std::cin >> s >> d;
	if (s == "zoomIn") {
		renderSetting->cameraPtr->zoomIn(d);
	}
	else if (s == "tiltUp") {
		renderSetting->cameraPtr->tiltUp(d);
	}
	else if (s == "tiltRight") {
		renderSetting->cameraPtr->tiltRight(d);
	} 
	else if (s == "rotate") {
		renderSetting->cameraPtr->rotate(d);
	}
	else if (s == "move") {
		double r, u;
		std::cin >> r >> u;
		renderSetting->cameraPtr->move(d, r, u);
	}
	else {
		renderSetting->cameraPtr->printInfo();
		return;
	}
	renderImage();
}


void ImageRenderer::renderImageThreading(ThreadingTask &task) {
	// SimpleIntersectionTester sit;
	//Kdtree sit(renderSetting->scenePtr);
	IntersectionTester *it;
	MonteCarloRayTracer mcrt;
	if (renderSetting->intersectiontester == "SimpleIntersectionTester") {
		it = new SimpleIntersectionTester();
		it->scenePtr = renderSetting->scenePtr;
	}
	else if (renderSetting->intersectiontester == "KdtreeIntersectionTester") {
		it = new Kdtree(renderSetting->scenePtr);
	}
	PixelRenderer pr;

	if (renderSetting->rayTracerSampler == "Stratified") {
		mcrt.samplerPtr = new StratifiedSampler();
	}
	else if (renderSetting->rayTracerSampler == "LatinCube") {
		mcrt.samplerPtr = new LatinCubeSampler();
	}
	else {
		printf("No proper sampler for ray tracer\n");
		exit(-1);
	}

	if (renderSetting->pixelSampler == "Stratified") {
		pr.samplerPtr = new StratifiedSampler();
	}
	else if (renderSetting->pixelSampler == "LatinCube") {
		pr.samplerPtr = new StratifiedSampler();
	}
	else {
		printf("No proper sampler for pixel renderer\n");
		exit(-1);
	}

	mcrt.samplerPtr->generatePreSample_Specular(renderSetting->BRDF_sampleSize, 7793, 1000);
	mcrt.samplerPtr->generatePreSample_Diffuse(renderSetting->BRDF_sampleSize, 7379);
	mcrt.samplerPtr->generatePreSample_Diffuse_Single(737797);
	mcrt.samplerPtr->generatePreSample_Specular_Single(717977, 1000);
	mcrt.samplerPtr->generatePreSample_Specular_Single(717997, 500);

	mcrt.depth = renderSetting->rayTraceDepth;
	mcrt.intersectionTesterPtr = it;
	mcrt.sampleSize = renderSetting->BRDF_sampleSize;

	pr.cameraPtr = renderSetting->cameraPtr;
	pr.rayTracerPtr = &mcrt;
	pr.sampleSize = renderSetting->pixelSampleSize;

	it->sampler = mcrt.samplerPtr;
/*
	Color c = pr.renderPixel(30, 390);
	exit(0);*/

	for (int i = task.start; i < task.end; i++) {
		printf("line %d being rendered\n", i);
		for (int j = 0; j < task.h; j++) {
			Color out = pr.renderPixel(i, j);
			task.bitmap[i - task.start][j] = out;
		}
	}
}

#pragma endregion

ThreadingTask::ThreadingTask(int _start, int _end, int _h) : start(_start), end(_end), h(_h) {
	bitmap = std::vector<std::vector<Color>>();
	for (int i = start; i < end; i++) {
		std::vector<Color> col(h);
		bitmap.push_back(col);
	}
}

RenderSetting::RenderSetting(const char *filename) {
	std::ifstream in = std::ifstream(filename);
	std::string line;
	scenePtr = new Scene;
	while (std::getline(in, line)) {
		std::vector<std::string> argv;
		boost::split(argv, line, boost::is_any_of("\t "));
		if (argv[0] == "ambient") {
			Real
				x = boost::lexical_cast<Real>(argv[1]),
				y = boost::lexical_cast<Real>(argv[2]),
				z = boost::lexical_cast<Real>(argv[3]);
			scenePtr->ambientLight = Colors::white.filter(x, y, z);
		}
		if (argv[0] == "object") {
			scenePtr->loadObject(argv[1].c_str());
			
		}
		if (argv[0] == "relocate") {
			Real
				x = boost::lexical_cast<Real>(argv[1]),
				y = boost::lexical_cast<Real>(argv[2]),
				z = boost::lexical_cast<Real>(argv[3]);
			(scenePtr->objects.end() - 1)->relocate(x, y, z);
		}
		if (argv[0] == "scale") {
			Real ratio = boost::lexical_cast<Real>(argv[1]);
			(scenePtr->objects.end() - 1)->scale(ratio);
		}
		if (argv[0] == "rotate") {
			int axis = boost::lexical_cast<int>(argv[1]);
			Real angle = boost::lexical_cast<Real>(argv[2]);
			(scenePtr->objects.end() - 1)->rotate(axis, Constants::Pi * angle);
		}
		if (argv[0] == "camera") {
			Real
				x = boost::lexical_cast<Real>(argv[1]),
				y = boost::lexical_cast<Real>(argv[2]),
				z = boost::lexical_cast<Real>(argv[3]),
				dx = boost::lexical_cast<Real>(argv[4]),
				dy = boost::lexical_cast<Real>(argv[5]),
				dz = boost::lexical_cast<Real>(argv[6]),
				ux = boost::lexical_cast<Real>(argv[7]),
				uy = boost::lexical_cast<Real>(argv[8]),
				uz = boost::lexical_cast<Real>(argv[9]),
				foco = boost::lexical_cast<Real>(argv[10]),
				w = boost::lexical_cast<Real>(argv[11]),
				h = boost::lexical_cast<Real>(argv[12]),
				step = boost::lexical_cast<Real>(argv[13]);
			cameraPtr = new Camera(x, y, z, dx, dy, dz, ux, uy, uz, foco, w, h, step);
			if (argv.size() > 14) {
				Real
					focoplane = boost::lexical_cast<Real>(argv[14]),
					aperture = boost::lexical_cast<Real>(argv[15]);
				cameraPtr->setDOF(aperture, focoplane);
				cameraPtr->DOF = true;
			}
			continue;
		}
		if (argv[0] == "camera::move") {
			if (argv.size() > 3) {
				Real
					x = boost::lexical_cast<Real>(argv[1]),
					y = boost::lexical_cast<Real>(argv[2]),
					z = boost::lexical_cast<Real>(argv[3]);
				cameraPtr->move(x, y, z);
			}
			double d = boost::lexical_cast<Real>(argv[2]);
			if (argv[1] == "zoomIn") {
				cameraPtr->zoomIn(d);
			}
			else if (argv[1] == "tiltUp") {
				cameraPtr->tiltUp(d);
			}
			else if (argv[1] == "tiltRight") {
				cameraPtr->tiltRight(d);
			}
			else if (argv[1] == "rotate") {
				cameraPtr->rotate(d);
			}
		}
		if (argv[0] == "pixelSampler") {
			pixelSampler = argv[1];
			pixelSampleSize = boost::lexical_cast<int>(argv[2]);
			continue;
		}
		if (argv[0] == "intersectionTester") {
			intersectiontester = argv[1];
			continue;
		}
		if (argv[0] == "rayTraceSampler") {
			rayTracerSampler = argv[1];
			rayTraceDepth = boost::lexical_cast<int>(argv[2]);
			BRDF_sampleSize = boost::lexical_cast<int>(argv[3]);
			continue;
		}
		if (argv[0] == "threading") {
			threading = boost::lexical_cast<int>(argv[1]);
			continue;
		}
		if (argv[0] == "imageSize") {
			imageWidth = boost::lexical_cast<int>(argv[1]);
			imageHeight = boost::lexical_cast<int>(argv[2]);
			continue;
		}
		if (argv[0] == "filter") {
			Er = boost::lexical_cast<Real>(argv[1]);
			Eg = boost::lexical_cast<Real>(argv[2]);
			Eb = boost::lexical_cast<Real>(argv[3]);
			continue;
		}
		if (argv[0] == "writeRaw") {
			writeRaw = true;
			rawFile = argv[1];
			continue;
		}
		if (argv[0] == "fromRaw") {
			fromRaw = true;
			rawFile = argv[1];
			continue;
		}
		if (argv[0] == "#") {
			continue;
		}
	}
	scenePtr->constructScene();
}