#include "raster_math.h"
#include <algorithm>
#include <cmath>

Vec3 shade(
	const Vec3& color,
	const Vec3& normal,
	const Vec3& lightDir,
	const Vec3& position,
	const Vec3& eye
)
{
	const Vec3& N = normal.normalized();
	const Vec3& L = lightDir.normalized();
	const Vec3& V = Vec3(eye - position).normalized();
	const Vec3& H = Vec3(L + V).normalized();

	float p = 32.0f;
	float ks = 0.5f;

	float specular = 0.0f;
	if(Vec3::dot(L, N) > 0.0f) specular = ks * std::pow(std::max(0.0f, Vec3::dot(H, N)), p);

	float diffuse = std::max(Vec3::dot(N, L),0.0f);

	float brightness = 0.1f + 0.9f * diffuse;

	return color * brightness + Vec3(1,1,1) * specular;
}