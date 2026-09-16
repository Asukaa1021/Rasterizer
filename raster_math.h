#pragma once
#include <cmath>

class Vec2
{
private:
	float x;
	float y;

public:
	Vec2(float a = 0, float b = 0)
	{
		x = a, y = b;
	}
	float get_x() const
	{
		return x;
	}
	float get_y() const
	{
		return y;
	}
	Vec2 operator+ (const Vec2& b) const
	{
		return Vec2(x + b.get_x(), y + b.get_y());
	}

	Vec2 operator- (const Vec2& b) const
	{
		return Vec2(x - b.get_x(), y - b.get_y());
	}

	Vec2 operator* (float s) const
	{
		return Vec2(x * s, y * s);
	}

	static float dot(const Vec2& a, const Vec2& b)
	{
		return a.get_x() * b.get_x() + a.get_y() * b.get_y();
	}

	static float cross(const Vec2& a, const Vec2& b)
	{
		return a.get_x() * b.get_y() - a.get_y() * b.get_x();
	}
};

class Vec3
{
private:
	float x;
	float y;
	float z;

public:
	Vec3(float a = 0, float b = 0, float c = 0)
	{
		x = a, y = b, z = c;
	}
	float get_x() const
	{
		return x;
	}
	float get_y() const
	{
		return y;
	}
	float get_z() const
	{
		return z;
	}
	Vec3 normalized() const
	{
		float len = std::sqrt(x * x + y * y + z * z);
		if (len == 0) return Vec3(0, 0, 0);
		return Vec3(x / len, y / len, z / len);
	}
	Vec3 operator+ (const Vec3& b) const
	{
		return Vec3(x + b.get_x(), y + b.get_y(),z + b.get_z());
	}

	Vec3 operator- (const Vec3& b) const
	{
		return Vec3(x - b.get_x(), y - b.get_y(), z - b.get_z());
	}

	Vec3 operator* (float s) const
	{
		return Vec3(x * s, y * s, z * s);
	}

	static float dot(const Vec3& a, const Vec3& b)
	{
		return a.get_x() * b.get_x() + a.get_y() * b.get_y() + a.get_z() * b.get_z();
	}

	static Vec3 cross(const Vec3& a, const Vec3& b)
	{
		return Vec3(a.get_y() * b.get_z() - a.get_z() * b.get_y(),
					a.get_z() * b.get_x() - a.get_x() * b.get_z(),
					a.get_x() * b.get_y() - a.get_y() * b.get_x());
	}

};

class Vec4
{
private:
	float x;
	float y;
	float z;
	float w;

public:
	Vec4(float a = 0, float b = 0, float c = 0,float d = 0)
	{
		x = a, y = b, z = c, w = d;
	}
	float get_x() const
	{
		return x;
	}
	float get_y() const
	{
		return y;
	}
	float get_z() const
	{
		return z;
	}
	float get_w() const
	{
		return w;
	}
	Vec4 operator+ (const Vec4& b) const
	{
		return Vec4(x + b.get_x(), y + b.get_y(), z + b.get_z(), w + b.get_w());
	}

	Vec4 operator- (const Vec4& b) const
	{
		return Vec4(x - b.get_x(), y - b.get_y(), z - b.get_z(), w - b.get_w());
	}

	Vec4 operator* (float s) const
	{
		return Vec4(x * s, y * s, z * s, w * s);
	}

	static float dot(const Vec4& a, const Vec4& b)
	{
		return a.get_x() * b.get_x() + a.get_y() * b.get_y() + a.get_z() * b.get_z() + a.get_w() * b.get_w();
	}

};

class Mat4
{
private:
	float mat[4][4]{ };

public:
	Mat4()
	{
		mat[0][0] = mat[1][1] = mat[2][2] = mat[3][3] = 1.0;
	}

	Vec4 operator* (const Vec4& s) const
	{
		float x = s.get_x();
		float y = s.get_y();
		float z = s.get_z();
		float w = s.get_w();

		return Vec4(mat[0][0] * x + mat[0][1] * y + mat[0][2] * z + mat[0][3] * w,
					mat[1][0] * x + mat[1][1] * y + mat[1][2] * z + mat[1][3] * w,
					mat[2][0] * x + mat[2][1] * y + mat[2][2] * z + mat[2][3] * w,
					mat[3][0] * x + mat[3][1] * y + mat[3][2] * z + mat[3][3] * w);
	}

	Mat4 operator* (const Mat4& b) const
	{
		Mat4 res;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				res.mat[i][j] = 0;
				for (int k = 0; k < 4; k++)
				{
					res.mat[i][j] += mat[i][k] * b.mat[k][j];
				}
			}
		}
		return res;
	}

	static Mat4 translation(float dx, float dy, float dz)
	{
		Mat4 res;
		res.mat[0][3] = dx;
		res.mat[1][3] = dy;
		res.mat[2][3] = dz;
		return res;
	}
	static Mat4 scaling(float sx, float sy,float sz)
	{
		Mat4 res;
		res.mat[0][0] = sx;
		res.mat[1][1] = sy;
		res.mat[2][2] = sz;
		return res;
	}
	static Mat4 rotationX(float angle)
	{
		float s = std::sin(angle);
		float c = std::cos(angle);

		Mat4 res;
		res.mat[1][1] = c;
		res.mat[1][2] = -s;
		res.mat[2][1] = s;
		res.mat[2][2] = c;
		
		return res;
	}
	static Mat4 rotationY(float angle)
	{
		float s = std::sin(angle);
		float c = std::cos(angle);

		Mat4 res;
		res.mat[0][0] = c;
		res.mat[0][2] = s;
		res.mat[2][0] = -s;
		res.mat[2][2] = c;

		return res;
	}
	static Mat4 rotationZ(float angle)
	{
		float s = std::sin(angle);
		float c = std::cos(angle);

		Mat4 res;
		res.mat[0][0] = c;
		res.mat[0][1] = -s;
		res.mat[1][0] = s;
		res.mat[1][1] = c;

		return res;
	}

	static Mat4 lookAt(const Vec3& eye,const Vec3& target,const Vec3& up)
	{
		Vec3 p = (target - eye).normalized();
		Vec3 cameraX = Vec3::cross(p, up).normalized();
		Vec3 cameraZ = p * -1.0f;
		Vec3 cameraY = Vec3::cross(cameraZ,cameraX).normalized();

		Mat4 rotation;
		rotation.mat[0][0] = cameraX.get_x();
		rotation.mat[0][1] = cameraX.get_y();
		rotation.mat[0][2] = cameraX.get_z();

		rotation.mat[1][0] = cameraY.get_x();
		rotation.mat[1][1] = cameraY.get_y();
		rotation.mat[1][2] = cameraY.get_z();

		rotation.mat[2][0] = cameraZ.get_x();
		rotation.mat[2][1] = cameraZ.get_y();
		rotation.mat[2][2] = cameraZ.get_z();

		Mat4 trans = Mat4::translation(
			-eye.get_x(),
			-eye.get_y(),
			-eye.get_z()
		);

		return rotation * trans;
	}

	static Mat4 perspective(float fovY,float aspect,float nearPlane,float farPlane)
	{
		float t = std::tan(fovY / 2.0f);

		Mat4 res;
		res.mat[0][0] = (1.0f / (aspect * t));
		res.mat[1][1] = (1.0f / t);
		res.mat[2][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
		res.mat[2][3] = -farPlane * nearPlane * 2 / (farPlane - nearPlane);
		res.mat[3][2] = -1.0f;
		res.mat[3][3] = 0.0f;

		return res;
	}
};

