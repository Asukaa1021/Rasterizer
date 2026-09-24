#include "Rasterizer.h"
#include "raster_math.h"
#include "vertex.h"
#include <algorithm>
#include <limits>
#include <vector>
#include "shade.h"

void Rasterizer::drawTriangle(const Triangle& triangle,const Vec3& lightDir,const Vec3& eye)
{
	Mat4 mvp = projection * view * model;
	Vec3 point3A = triangle[0].get_pos();
	Vec3 point3B = triangle[1].get_pos();
	Vec3 point3C = triangle[2].get_pos();

	Vec3 colA = triangle[0].get_col();
	Vec3 colB = triangle[1].get_col();
	Vec3 colC = triangle[2].get_col();

	Vec4 point4A = mvp * Vec4(point3A.get_x(), point3A.get_y(), point3A.get_z(), 1);
	Vec4 point4B = mvp * Vec4(point3B.get_x(), point3B.get_y(), point3B.get_z(), 1);
	Vec4 point4C = mvp * Vec4(point3C.get_x(), point3C.get_y(), point3C.get_z(), 1);

	Vec4 world4A = model * Vec4(point3A.get_x(), point3A.get_y(), point3A.get_z(), 1);
	Vec4 world4B = model * Vec4(point3B.get_x(), point3B.get_y(), point3B.get_z(), 1);
	Vec4 world4C = model * Vec4(point3C.get_x(), point3C.get_y(), point3C.get_z(), 1);

	Vec3 world3A = Vec3(world4A.get_x(), world4A.get_y(), world4A.get_z());
	Vec3 world3B = Vec3(world4B.get_x(), world4B.get_y(), world4B.get_z());
	Vec3 world3C = Vec3(world4C.get_x(), world4C.get_y(), world4C.get_z());

	

	Vec3 normal = Vec3::cross(world3B - world3A, world3C - world3A).normalized();

	Vec3 toEye = eye - world3A;
	if (Vec3::dot(toEye, normal) < 0.0f)
	{
		return;
	}

	Vec2 point2A = Vec2((point4A.get_x() / point4A.get_w() + 1.0f) * width * 0.5f,
						(-(point4A.get_y() / point4A.get_w()) + 1.0f) * height * 0.5f);
	Vec2 point2B = Vec2((point4B.get_x() / point4B.get_w() + 1.0f) * width * 0.5f,
						(-(point4B.get_y() / point4B.get_w()) + 1.0f) * height * 0.5f);
	Vec2 point2C = Vec2((point4C.get_x() / point4C.get_w() + 1.0f) * width * 0.5f,
		(-(point4C.get_y() / point4C.get_w()) + 1.0f) * height * 0.5f);

	drawLine(point2A, point2B);
	drawLine(point2B, point2C);
	drawLine(point2C, point2A);
	
	/*
	
	if (Vec2::cross(point2A - point2B, point2A - point2C) == 0.0f) return;

	float minX = std::min({ point2A.get_x(), point2B.get_x(), point2C.get_x() });
	int stX = std::max(static_cast<int>(std::floor(minX)),0);
	float maxX = std::max({ point2A.get_x(), point2B.get_x(), point2C.get_x() });
	int edX = std::min(static_cast<int>(std::ceil(maxX)),width - 1);

	float minY = std::min({ point2A.get_y(), point2B.get_y(), point2C.get_y() });
	int stY = std::max(static_cast<int>(std::floor(minY)),0);
	float maxY = std::max({ point2A.get_y(), point2B.get_y(), point2C.get_y() });
	int edY = std::min(static_cast<int>(std::ceil(maxY)),height - 1);

	Vec2 b2a = point2A - point2B;
	Vec2 c2b = point2B - point2C;
	Vec2 a2c = point2C - point2A;
	
	for (int x = stX; x <= edX; x++)
	{
		for (int y = stY; y <= edY; y++)
		{
			int idx = x + y * width;
			Vec2 temp = Vec2(x + 0.5f, y + 0.5f);

			Vec2 a2t = temp - point2A;
			Vec2 b2t = temp - point2B;
			Vec2 c2t = temp - point2C;

			if ((Vec2::cross(a2t, a2c) >= 0
				&& Vec2::cross(b2t, b2a) >= 0
				&& Vec2::cross(c2t, c2b) >= 0) ||
				(Vec2::cross(a2t, a2c) <= 0
				&& Vec2::cross(b2t, b2a) <= 0
				&& Vec2::cross(c2t, c2b) <= 0)
				)
			{
				float alpha = Vec2::cross(c2b, c2t) / point4A.get_w();
				float beta = Vec2::cross(a2c, a2t) / point4B.get_w();
				float gamma = Vec2::cross(b2a, b2t) / point4C.get_w();

				float sum = alpha + beta + gamma;

				alpha /= sum;
				beta /= sum;
				gamma /= sum;

				float depA = point4A.get_z();
				float depB = point4B.get_z();
				float depC = point4C.get_z();

				float depth = depA * alpha + depB * beta + depC * gamma;

				if (depth < depthBuffer[idx])
				{
					Vec3 position = world3A * alpha + world3B * beta + world3C * gamma;

					pixels[idx] = shade(Vec3(colA.get_x() * alpha + colB.get_x() * beta + colC.get_x() * gamma,
						colA.get_y() * alpha + colB.get_y() * beta + colC.get_y() * gamma,
						colA.get_z() * alpha + colB.get_z() * beta + colC.get_z() * gamma), normal, lightDir,position,eye);

					depthBuffer[idx] = depth;
				}
				
			}
		}
	}
	*/

}

void Rasterizer::clear()
{
	for (auto& pixel : pixels)
	{
		pixel = Vec3(0, 0, 0);
	}
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			depthBuffer[x + y * width] = std::numeric_limits<float>::infinity();
		}
	}
}

void Rasterizer::drawLine(const Vec2& start, const Vec2& end)
{
	double dx = end.get_x() - start.get_x();
	double dy = end.get_y() - start.get_y();
	int steps = ceil(std::max(std::abs(dx),std::abs(dy)));
	double x = start.get_x();
	double y = start.get_y();
	if (steps == 0)
	{
		int pixelX = static_cast<int> (std::round(x));
		int pixelY = static_cast<int> (std::round(y));

		if (pixelX >= 0 && pixelX < width && pixelY >= 0 && pixelY < height)
		pixels[pixelX + pixelY * width] = Vec3(1.0f, 1.0f, 1.0f);
		return;
	}
	else
	{
		double deltaX = dx / steps;
		double deltaY = dy / steps;
		for (int i = 0; i <= steps; i++)
		{
			int pixelX = static_cast<int> (std::round(x));
			int pixelY = static_cast<int> (std::round(y));

			if(pixelX >= 0 && pixelX < width && pixelY >= 0 && pixelY < height)
				pixels[pixelX + pixelY * width] = Vec3(1.0f, 1.0f, 1.0f);

			x += deltaX;
			y += deltaY;
		}
	}
}
