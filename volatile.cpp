#include "Screen.h"
#include "Renderer.h"

void ImageRenderer::fromRaw(const char* rawfile) {
	std::ifstream is(rawfile);
	int w, h;
	is >> w >> h;
	Screen screen(w, h);
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			Real r, g, b;
			is >> r >> g >> b;
			Color color(r, g, b);
			color = color.filter(20, 20, 16);
			screen.color = color.gammaCorrection(0.7);
			screen.drawPixel(i, j);
		}
	}
	screen.show();
}
