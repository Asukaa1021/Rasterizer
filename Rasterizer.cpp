#include "Rasterizer.h"
#include "raster_math.h"
#include "vertex.h"
#include <algorithm>

void Rasterizer::drawTriangle(const Triangle& triangle)
{
	Mat4 mvp = projection * view * model;
	Vec3 point3A = triangle[0].get_pos();
	Vec3 point3B = triangle[1].get_pos();
	Vec3 point3C = triangle[2].get_pos();

	Vec4 point4A = mvp * Vec4(point3A.get_x(), point3A.get_y(), point3A.get_z(), 1);
	Vec4 point4B = mvp * Vec4(point3B.get_x(), point3B.get_y(), point3B.get_z(), 1);
	Vec4 point4C = mvp * Vec4(point3C.get_x(), point3C.get_y(), point3C.get_z(), 1);


	Vec2 point2A = Vec2((point4A.get_x() / point4A.get_w() + 1.0f) * width * 0.5f,
						(-(point4A.get_y() / point4A.get_w()) + 1.0f) * height * 0.5f);
	Vec2 point2B = Vec2((point4B.get_x() / point4B.get_w() + 1.0f) * width * 0.5f,
						(-(point4B.get_y() / point4B.get_w()) + 1.0f) * height * 0.5f);
	Vec2 point2C = Vec2((point4C.get_x() / point4C.get_w() + 1.0f) * width * 0.5f,
						(-(point4C.get_y() / point4C.get_w()) + 1.0f) * height * 0.5f);

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
				pixels[idx] = 1;
			}
		}
	}

}
