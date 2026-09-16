#pragma once
#include "raster_math.h"
#include "vertex.h"
#include <vector>
class Rasterizer
{
private:
	int width;
	int height;

	Mat4 model;
	Mat4 view;
	Mat4 projection;

	std::vector<unsigned char> pixels;

public:
	Rasterizer(int w, int h) : width(w), height(h),pixels(w * h,0){}
	void drawTriangle(const Triangle& triangle);
	int get_width()
	{
		return width;
	}
	int get_height()
	{
		return height;
	}
	const std::vector<unsigned char>& get_pixels() const
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