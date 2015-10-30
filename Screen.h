#pragma once
#include "stdafx.h"
#include "opencv2\opencv.hpp"

using std::vector;
using cv::Mat;
using cv::Vec3b;
typedef unsigned char uchar;

struct Color {
private:
	Real r = 0, g = 0, b = 0; 
public:
	Color() {}
	Color(Real _r, Real _g, Real _b) 
		: r(_r), g(_g), b(_b) {
	}
	Vec3b value() {
		return Vec3b(
			std::min((int)b, 255),
			std::min((int)g, 255),
			std::min((int)r, 255));
	}

	Color scale(Real k) {
		return Color(k*r, k*g, k*b);
	}
	static Color fromHSV(Real, Real, Real);
	static Color gray(Real g) {
		return Color(g, g, g);
	}
	inline Real red() const { return r; }
	inline Real green() const { return g; }
	inline Real blue() const { return b; }
	inline void operator+=(const Color& c) {
		r += c.red();
		g += c.green();
		b += c.blue();
	}
	inline Color filter(Real _r, Real _g, Real _b) const {
		return Color(
			red() * _r,
			green() * _g,
			blue() * _b);
	}
	inline Color gammaCorrection(Real gamma) {
		return Color(
			255. * pow(r / 255., gamma),
			255. * pow(g / 255., gamma),
			255. * pow(b / 255., gamma));
	}
};

struct Point {
	Real x, y;
	Point(Real x_, Real y_) : x(x_), y(y_) {}
};

class Screen {
private:
	Real top, bottom, left, right;
	int width, height;
	// granularity
	Real dx, dy;
	Mat bitmap;

public:
	// status
	Color color = Color();
	bool antialiasing = false;

public:
	Screen(Real t, Real b, Real l, Real r, int w, int h) :
		top(t), bottom(b), left(l), right(r), width(w), height(h) {
		bitmap = Mat(cv::Size(width, height), CV_8UC3);
		for (int w = 0; w < width; w++) {
			for (int h = 0; h < height; h++) {
				bitmap.at<Vec3b>(h, w) = color.value();
			}
		}
		dx = (right - left) / width;
		dy = (top - bottom) / height;
	}
	Screen(int w, int h) : width(w), height(h) {
		bitmap = Mat(cv::Size(width, height), CV_8UC3);
		for (int w = 0; w < width; w++) {
			for (int h = 0; h < height; h++) {
				bitmap.at<Vec3b>(h, w) = color.value();
			}
		}
	}

	int getWidth() const { return width; }
	int getHeight() const { return height; }
	int getX(Real x) const { return lround((x - left) / dx); }
	int getY(Real y) const { return lround((y - bottom) / dy); }
	Real getDX() const { return dx; }
	Real getDY() const { return dy; }

	void drawPixel(int i, int j);
	void drawPoint(Point p);
	void clear(Color);
	void show();

	void drawLine(Point, Point);
	void drawPolygon(const std::vector<Point> &);

private:
	void coreDrawLine();
	void coreDrawPixel(int i, int j);
};


#pragma region constants

namespace Colors {
	const Color black = Color(0, 0, 0);
	const Color red = Color(255, 0, 0);
	const Color white = Color(255, 255, 255);
	const Color green = Color(0, 255, 0);
	const Color blue = Color(0, 0, 255);
}

#pragma endregion