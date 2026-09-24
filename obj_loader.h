#pragma once
#include "mesh.h"
#include <string>
#include <sstream>
#include <fstream>
#include <limits>

inline int get_idx(std::string s)
{
	int num = 0;
	for (char c : s)
	{
		if (c == '/') break;

		if (c >= '0' && c <= '9')
		{
			if (num > (std::numeric_limits<int> ::max() - (c - '0')) / 10) return 0;
			num = num * 10 + (c - '0');
		}
		else return 0;
	}
	return num;
}



inline bool loadOBJ(const std::string& path, Mesh& mesh)
{
	std::ifstream file(path);

	if (!file.is_open())
		return false;

	std::string line;

	while (std::getline(file, line))
	{
		std::istringstream stream(line);

		std::string type;
		
		stream >> type;
		
		if (type == "v")
		{
			float x, y, z;
			if (!(stream >> x >> y >> z)) return false;

			mesh.positions.push_back(Vec3(x, y, z));
		}
			
		else if (type == "f")
		{
			std::string a, b, c;
			std::string extra;
			if (!(stream >> a >> b >> c)) return false;
			if (stream >> extra)
			{
				if (extra[0] != '#')
					return false;
			}
			int num_a = get_idx(a), num_b = get_idx(b), num_c = get_idx(c);
			
			if (num_a <= 0 || num_b <= 0 || num_c <= 0) return false;

			if (static_cast<std::size_t> (num_a) > mesh.positions.size() ||
				static_cast<std::size_t> (num_b) > mesh.positions.size() ||
				static_cast<std::size_t> (num_c) > mesh.positions.size()) return false;

			mesh.triangles.push_back({num_a - 1,num_b - 1,num_c - 1});
		}
	}

	if (file.bad())
		return false;

	return true;
}
