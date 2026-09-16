#pragma once
#include "raster_math.h"
#include<array>

class Vertex
{
private:
	Vec3 position;
	Vec3 color;
	
public:
	Vertex(const Vec3& pos,const Vec3& col)
	{
		position = pos;
		color = col;
	}

	Vec3 get_pos() const
	{
		return position;
	}
	Vec3 get_col() const
	{
		return color;
	}
};

using Triangle = std::array<Vertex, 3>;
