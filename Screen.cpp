#include "Screen.h"
#include <iostream>

// this function guarantees consistent behavior in both positive and negative case, moreover, allows a tolerance at each boundary.
using std::lround;

void Screen::drawPixel(int i, int j) {
	coreDrawPixel(i, j);
}

void Screen::drawPoint(Point p) {
	int i = lround((p.x - left) / dx), j = lround((p.y - bottom) / dy);
	bitmap.at<Vec3b>(i, j) = color.value();
}


void Screen::clear(Color color) {
	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			bitmap.at<Vec3b>(h, w) = color.value();
		}
	}
}

void Screen::show() {
	cv::imshow("Image", bitmap);
	cv::waitKey();
}

void Screen::coreDrawPixel(int i, int j) {
	if (i < 0 || j < 0 || i >= width || j >= height) {
		return;
	}
	bitmap.at<Vec3b>(height - 1 - j, i) = color.value();
}

void Screen::drawLine(Point p0, Point p1) {
	if (p0.x > p1.x) {
		drawLine(p1, p0);
		return;
	}
	else {
		if (abs(p1.y - p0.y) < abs(p1.x - p0.x)) {
			Real k = (p1.y - p0.y) / (p1.x - p0.x);
			int i0 = lround((p0.x - left) / dx), i1 = lround((p1.x - left) / dx);
			Real j = (p0.y - bottom);
			for (i0; i0 < i1; i0++) {
				coreDrawPixel(i0, lround(j / dy));
				j += k * dx;
			}
		}
		else if (p1.y > p0.y) {
			Real k = (p1.x - p0.x) / (p1.y - p0.y);
			int i0 = lround((p0.y - bottom) / dy), i1 = lround((p1.y - bottom) / dy);
			Real j = (p0.x - left);
			for (i0; i0 < i1; i0++) {
				coreDrawPixel(lround(j / dx), i0);
				j += k * dy;
			}
		}
		else {
			Real k = (p0.x - p1.x) / (p0.y - p1.y);
			int i0 = lround((p1.y - bottom) / dy), i1 = lround((p0.y - bottom) / dy);
			Real j = (p1.x - left);
			for (i0; i0 < i1; i0++) {
				coreDrawPixel(lround(j / dx), i0);
				j += k * dy;
			}
		}
	}
}

void Screen::drawPolygon(const vector<Point>& p) {
	int l = p.size();
	for (int i = 0; i < l - 1; i++) {
		drawLine(p[i], p[i + 1]);
	}
	drawLine(p[l - 1], p[0]);
}