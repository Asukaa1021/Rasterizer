// Rasterizer interface: viewport size, transformation matrices, and pixel coverage buffer.
#pragma once
#include "raster_math.h"
#include "vertex.h"
#include <vector>
#include <limits>

class Rasterizer
{
private:
	int width;
	int height;

	Mat4 model;
	Mat4 view;
	Mat4 projection;

	std::vector<Vec3> pixels;
	std::vector<float> depthBuffer;
	void drawLine(const Vec2& start, const Vec2& end);

public:
	Rasterizer(int w, int h) : width(w), height(h),pixels(w * h,Vec3(0,0,0)),depthBuffer(width* height, std::numeric_limits<float>::infinity()) {}
	void drawTriangle(const Triangle& triangle,const Vec3& lightDir,const Vec3& eye);
	void clear();
	int get_width()
	{
		return width;
	}
	int get_height()
	{
		return height;
	}
	const std::vector<Vec3>& get_pixels() const
	{
		return pixels;
	}
	void setView(const Mat4& v)
	{
		view = v;
	}
	void setModel(const Mat4& m)
	{
		model = m;
	}
	void setProjection(const Mat4& p)
	{
		projection = p;
	}
};