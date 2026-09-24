#pragma once
#include "raster_math.h"
#include <array>
#include <vector>

class Mesh
{
public:
	std::vector<Vec3> positions;
	std::vector<std::array<int, 3>> triangles;
};