#pragma once
#include "raster_math.h"
#include <cmath>

Vec3 shade(
	const Vec3& color,
	const Vec3& normal,
	const Vec3& lightDir,
	const Vec3& position,
	const Vec3& eye
);