#include "raster_math.h"
#include "mesh.h"
#include "mesh_simplifier.h"

#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>
#include <limits>
#include <set>



bool get_plane(const Vec3& A, const Vec3& B, const Vec3& C, std::array<double, 4>& plane)
{
	const Vec3 a2b = Vec3(B - A);
	const Vec3 b2c = Vec3(C - B);
	Vec3 n = Vec3::cross(a2b, b2c);
	if (Vec3::dot(n, n) == 0.0f) return false;

	n = n.normalized();

	double d = -Vec3::dot(n, A);
	plane = { n.get_x(),n.get_y(),n.get_z(),d };
	return true;
}


void Quadric::addPlane(const std::array<double, 4>& plane)
{
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			q[x][y] += plane[x] * plane[y];
		}
	}
}

double Quadric::evaluate(const Vec3& position) const
{
	double v[4] =
	{
		position.get_x(),
		position.get_y(),
		position.get_z(),
		1.0
	};

	double error = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			error += v[i] * q[i][j] * v[j];
		}
	}
	return error;
}

Quadric Quadric::operator+(const Quadric& b) const
{
	Quadric res;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			res.q[i][j] = q[i][j] + b.q[i][j];
		}
	}
	return res;
}

std::vector<Quadric> buildQuadric(const Mesh& mesh)
{
	std::vector<Quadric> quadrics(mesh.positions.size());

	for (const auto& triangle : mesh.triangles)
	{
		int a = triangle[0];
		int b = triangle[1];
		int c = triangle[2];
		const Vec3 A = mesh.positions[a];
		const Vec3 B = mesh.positions[b];
		const Vec3 C = mesh.positions[c];
		std::array<double, 4> plane;
		if (get_plane(A, B, C, plane))
		{
			quadrics[a].addPlane(plane);
			quadrics[b].addPlane(plane);
			quadrics[c].addPlane(plane);
		}
	}
	return quadrics;
}

bool solveOptimcalPosition(
	const Quadric& quadric,
	Vec3& position
)
{
	double m[3][4]{};
	double scale = 0.0f;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m[i][j] = quadric.q[i][j];
			scale = std::max(scale, std::abs(m[i][j]));
		}
		m[i][3] = -quadric.q[i][3];
	}

	if (scale == 0.0f) return false;

	const double eps = scale * 1e-12;

	for (int col = 0; col < 3; col++)
	{
		int pivotRow = col;
		for (int row = col + 1; row < 3; row++)
		{
			if (std::abs(m[pivotRow][col]) < std::abs(m[row][col]))
			{
				pivotRow = row;
			}
		}

		if (std::abs(m[pivotRow][col]) <= eps) return false;

		for (int j = 0; j < 4; j++)
		{
			std::swap(m[pivotRow][j], m[col][j]);
		}

		double pivot = m[col][col];

		for (int j = col; j < 4; j++)
		{
			m[col][j] /= pivot;
		}	

		for (int row = 0; row < 3; row++)
		{
			if (row == col) continue;

			double factor = m[row][col];

			for (int j = col; j < 4; j++)
			{
				m[row][j] -= factor * m[col][j];
			}
		}
	}

	for (int i = 0; i < 3; i++)
	{
		if (!std::isfinite(m[i][3]) ||
			std::numeric_limits<float>::max() < std::abs(m[i][3]))
			return false;
	}


	position = Vec3(
		static_cast<float> (m[0][3]),
		static_cast<float> (m[1][3]),
		static_cast<float> (m[2][3])
	);

	return true;
}

CollapseCandidate makeCandidate(
	const Mesh& mesh,
	const std::vector<Quadric>& quadrics,
	const SimplifyState& state,
	int a,
	int b
)
{
	Quadric quadric = quadrics[a] + quadrics[b];
	Vec3 bestPosition = mesh.positions[a];
	
	double bestCost = quadric.evaluate(bestPosition);

	double costB = quadric.evaluate(mesh.positions[b]);
	if (costB < bestCost)
	{
		bestCost = costB;
		bestPosition = mesh.positions[b];
	}

	Vec3 mid = (mesh.positions[a] + mesh.positions[b]) * 0.5f;
	double costMid = quadric.evaluate(mid);
	if (costMid < bestCost)
	{
		bestCost = costMid;
		bestPosition = mid;
	}

	Vec3 optimal;
	if (solveOptimcalPosition(quadric, optimal))
	{
		double costOptimal = quadric.evaluate(optimal);
		if (costOptimal < bestCost)
		{
			bestCost = costOptimal;
			bestPosition = optimal;
		}
	}

	return {
		a,
		b,
		bestCost,
		bestPosition,
		state.vertexVersion[a],
		state.vertexVersion[b]
	};
}

std::vector<std::array<int, 2>> collectEdge(const Mesh& mesh)
{
	std::vector<std::array<int, 2>> edges;
	std::set<std::array<int, 2>> edgeSet;
	for (auto& triangle : mesh.triangles)
	{
		edgeSet.insert({ std::min(triangle[0],triangle[1]),std::max(triangle[0],triangle[1])});
		edgeSet.insert({ std::min(triangle[1],triangle[2]),std::max(triangle[1],triangle[2]) });
		edgeSet.insert({ std::min(triangle[0],triangle[2]),std::max(triangle[0],triangle[2]) });
	}
	for (auto& e : edgeSet)
	{
		edges.push_back(e);
	}
	return edges;
}

candidateQueue buildCandidateQueue(
	const Mesh& mesh,
	const std::vector<Quadric>& quadrics,
	const SimplifyState& state
)
{
	candidateQueue cQ;
	std::vector<std::array<int, 2>> edges = collectEdge(mesh);

	for (auto& e : edges)
	{
		CollapseCandidate candidate = makeCandidate(mesh, quadrics, state, e[0], e[1]);
		if (std::isfinite(candidate.cost))
		{
			cQ.push(candidate);
		}
	}

	return cQ;
}

SimplifyState buildSimplifyState(const Mesh& mesh)
{
	SimplifyState state;
	state.triangleAlive.resize(mesh.triangles.size(),true);
	state.vertexAlive.resize(mesh.positions.size(),true);

	state.vertexTriangles.resize(mesh.positions.size());

	state.vertexVersion.resize(mesh.positions.size(), 0);

	for (int i = 0;i < mesh.triangles.size();i++)
	{
		state.vertexTriangles[mesh.triangles[i][0]].push_back(i);
		state.vertexTriangles[mesh.triangles[i][1]].push_back(i);
		state.vertexTriangles[mesh.triangles[i][2]].push_back(i);
	}

	return state;
}

std::vector<int> collectAffectTriangles(
	const SimplifyState& state,
	int a,
	int b
)
{
	std::set<int> affectTriangles;
	for (auto& triangle : state.vertexTriangles[a])
	{
		if(state.triangleAlive[triangle] && state.vertexAlive[a])
		affectTriangles.insert(triangle);
	}

	for (auto& triangle : state.vertexTriangles[b])
	{
		if (state.triangleAlive[triangle] && state.vertexAlive[b])
		affectTriangles.insert(triangle);
	}
	std::vector<int> Triangles;
	for (auto& triangle : affectTriangles)
	{
		Triangles.push_back(triangle);
	}

	return Triangles;
}

bool preservesGeometry(
	const Mesh& mesh,
	const SimplifyState& state,
	const CollapseCandidate& candidate
)
{
	int a = candidate.a;
	int b = candidate.b;
	
	if (a == b) return false;

	if (!state.vertexAlive[a] || !state.vertexAlive[b]) return false;

	auto affected = collectAffectTriangles(state, a, b);


	for (auto& triangleId : affected)
	{
		const auto& triangle = mesh.triangles[triangleId];

		bool hasA = false;
		bool hasB = false;

		for (auto& point : triangle)
		{
			if (point == a) hasA = true;
			if (point == b) hasB = true;
		}

		if (hasA && hasB) continue;

		std::array<Vec3, 3> oldPosition;
		std::array<Vec3, 3> newPosition;
		for (int j = 0; j < 3; j++)
		{
			int index = triangle[j];

			oldPosition[j] = mesh.positions[index];

			if (index == a || index == b)
			{
				newPosition[j] = candidate.position;
			}
			else
			{
				newPosition[j] = oldPosition[j];
			}

		}

		Vec3 oldNormal = Vec3::cross(
			oldPosition[2] - oldPosition[1],
			oldPosition[1] - oldPosition[0]
		);

		Vec3 newNormal = Vec3::cross(
			newPosition[2] - newPosition[1],
			newPosition[1] - newPosition[0]
		);

		float oldLengthSquare = Vec3::dot(oldNormal, oldNormal);
		float newLengthSquare = Vec3::dot(newNormal, newNormal);

		if (!std::isfinite(oldLengthSquare) || !std::isfinite(newLengthSquare))
			return false;

		if (oldLengthSquare == 0 ||
			newLengthSquare <= oldLengthSquare * 1e-12f)
			return false;

		float direction = Vec3::dot(oldNormal, newNormal);

		if (direction <= 0.0f || !std::isfinite(direction))
			return false;


		
	}
	return true;
}

std::vector<int> collectEdgeTriangles(
	const Mesh& mesh,
	const SimplifyState& state,
	int a,
	int b
)
{
	std::vector<int> triangles;
	std::unordered_map<int, int> num;
	if(state.vertexAlive[a])
	for (auto& triangle : state.vertexTriangles[a])
	{
		num[triangle]++;
	}
	if (state.vertexAlive[b])
	for (auto& triangle : state.vertexTriangles[b])
	{
		num[triangle]++;
	}
	for (auto& [triangle, cnt] : num)
	{
		if (cnt == 2 && state.triangleAlive[triangle]) 
			triangles.push_back(triangle);
	}
	return triangles;
}

std::vector<int> collectVertexNeighbors(
	const Mesh& mesh,
	const SimplifyState& state,
	int vertex
)
{
	std::vector<int> VertexNeighbors;
	if (!state.vertexAlive[vertex]) return VertexNeighbors;

	std::set<int> point;
	for (auto& triangle : state.vertexTriangles[vertex])
	{
		if (state.triangleAlive[triangle])
		{
			if (mesh.triangles[triangle][0] != vertex &&
				state.vertexAlive[mesh.triangles[triangle][0]])
				point.insert(mesh.triangles[triangle][0]);

			if (mesh.triangles[triangle][1] != vertex &&
				state.vertexAlive[mesh.triangles[triangle][1]])
				point.insert(mesh.triangles[triangle][1]);

			if (mesh.triangles[triangle][2] != vertex &&
				state.vertexAlive[mesh.triangles[triangle][2]])
				point.insert(mesh.triangles[triangle][2]);
		}
	}
	for (auto& p : point)
	{
		VertexNeighbors.push_back(p);
	}
	return VertexNeighbors;
}

bool isBoundaryVertex(
	const Mesh& mesh,
	const SimplifyState& state,
	int vertex
)
{
	std::vector<int> VertexNeighbors = collectVertexNeighbors(mesh, state, vertex);
	for (auto& b : VertexNeighbors)
	{
		std::vector<int> triangles = collectEdgeTriangles(mesh, state, vertex, b);
		if (triangles.size() == 1) return true;
	}
	return false;
}

bool hasExpectedCommonNeighbors(
	const Mesh& mesh,
	const SimplifyState& state,
	int a,
	int b
)
{
	if (!state.vertexAlive[a] || !state.vertexAlive[b])
		return false;
	if (a == b)
		return false;

	auto face = collectEdgeTriangles(mesh, state, a, b);
	if (face.size() != 2)
		return false;

	std::set<int> opposite;
	for (auto& triangle : face)
	{
		for (auto& point : mesh.triangles[triangle])
		{
			if (point != a && point != b)
				opposite.insert(point);
		}
	}
	if (opposite.size() != 2)
		return false;

	auto NeighborsA = collectVertexNeighbors(mesh, state, a);
	auto NeighborsB = collectVertexNeighbors(mesh, state, b);

	std::set<int> points;
	std::set<int> common;
	for (auto& neighbor : NeighborsA)
	{
		points.insert(neighbor);
	}
	for (auto& neighbor : NeighborsB)
	{
		if (points.count(neighbor))
			common.insert(neighbor);
	}
	return common == opposite;
}

bool preservesFaceUniqueness(
	const Mesh& mesh,
	const SimplifyState& state,
	int a,
	int b
)
{
	std::set<std::array<int, 3>> seen;

	auto affected = collectAffectTriangles(state, a, b);

	for (auto& triangleId : affected)
	{
		auto triangle = mesh.triangles[triangleId];
		bool hasA = false;
		bool hasB = false;

		for (auto& point : triangle)
		{
			if (point == a) hasA = true;
			if (point == b) hasB = true;
		}
		if (hasA && hasB) continue;

		for (auto& point : triangle)
		{
			if (point == b)
				point = a;
		}

		std::sort(triangle.begin(), triangle.end());

		if (!seen.insert(triangle).second)
		{
			return false;
		}
		
	}
	return true;
}

bool canCollapse(
	const Mesh& mesh,
	const SimplifyState& state,
	const CollapseCandidate& candidate
)
{
	int a = candidate.a;
	int b = candidate.b;
	if (a == b) return false;
	if (a < 0 || a >= mesh.positions.size() || b < 0 || b >= mesh.positions.size())
		return false;
	if (!state.vertexAlive[a] || !state.vertexAlive[b])
		return false;

	const Vec3& position = candidate.position;
	if (!std::isfinite(position.get_x()) ||
		!std::isfinite(position.get_y()) ||
		!std::isfinite(position.get_z()))
		return false;

	if (isBoundaryVertex(mesh, state, a) || isBoundaryVertex(mesh, state, b))
		return false;

	if (!hasExpectedCommonNeighbors(mesh, state, a, b))
		return false;

	if (!preservesFaceUniqueness(mesh, state, a, b))
		return false;

	if (!preservesGeometry(mesh, state, candidate))
		return false;

	return true;
}


bool isCandidateCurrent(
	const SimplifyState& state,
	const CollapseCandidate& candidate
)
{
	int a = candidate.a;
	int b = candidate.b;

	if (a >= state.vertexAlive.size() || b >= state.vertexAlive.size())
		return false;

	if (!state.vertexAlive[a] || !state.vertexAlive[b])
		return false;

	if (a < 0 || b < 0 || a == b)
		return false;

	return (candidate.versionA == state.vertexVersion[a]) &&
		(candidate.versionB == state.vertexVersion[b]);
}

CollapseResult CollapseEdge(
	Mesh& mesh,
	SimplifyState& state,
	std::vector<Quadric>& quadrics,
	const CollapseCandidate& candidate
)
{
	if (!isCandidateCurrent(state, candidate))
		return {};

	if (!canCollapse(mesh, state, candidate))
		return {};

	int a = candidate.a;
	int b = candidate.b;

	auto affected = collectAffectTriangles(state, a, b);

	std::set<int> affectedVertices;
	affectedVertices.insert(a);
	affectedVertices.insert(b);

	for (auto& index : affected)
	{
		for (auto& point : mesh.triangles[index])
		{
			affectedVertices.insert(point);
		}
	}
	int removedTriangleCount = 0;
	for (auto& index : affected)
	{
		bool hasA = false;
		bool hasB = false;
		auto& triangle = mesh.triangles[index];
		for (auto& point : triangle)
		{
			std::erase(state.vertexTriangles[point], index);
			if (point == a) hasA = true;
			if (point == b) hasB = true;
		}
		
		if (hasA && hasB)
		{
			state.triangleAlive[index] = false;
			removedTriangleCount++;
			continue;
		}
		
		for (auto& point : triangle)
		{
			if (point == b)
				point = a;
		}

		for (auto& point : triangle)
		{
			state.vertexTriangles[point].push_back(index);
		}
	}

	mesh.positions[a] = candidate.position;
	quadrics[a] = quadrics[a] + quadrics[b];
	std::vector<int> affectedPoints;
	for (auto& index : affectedVertices)
	{
		state.vertexVersion[index]++;
		affectedPoints.push_back(index);
	}

	state.vertexAlive[b] = false;
	state.vertexTriangles[b].clear();

	return {
		true,
		removedTriangleCount,
		affectedPoints
	};

}

void updateLocalCandidates(
	const Mesh& mesh,
	const SimplifyState& state,
	const std::vector<Quadric>& quadrics,
	const std::vector<int>& affectedVertices,
	candidateQueue& queue
)
{
	std::set<std::array<int, 2>> edges;
	for (auto& point : affectedVertices)
	{
		if (!state.vertexAlive[point])
			continue;

		std::vector<int> neighbors = collectVertexNeighbors(mesh, state, point);

		for (auto& neighbor : neighbors)
		{
			edges.insert({ 
				std::min(point,neighbor),
				std::max(point,neighbor) 
			});
		}
	}
	for (auto& edge : edges)
	{
		auto candidate = makeCandidate(mesh, quadrics, state, edge[0], edge[1]);
		if (!std::isfinite(candidate.cost))
			continue;

		queue.push(candidate);
	}
	
}

Mesh compactMesh(
	const Mesh& mesh,
	const SimplifyState& state
)
{
	std::vector<bool> used(mesh.positions.size(), false);
	Mesh result;
	std::vector<int> oldToNew(mesh.positions.size(), -1);

	for (int i = 0;i < mesh.triangles.size();i++)
	{
		if (!state.triangleAlive[i])
			continue;

		for (auto& point : mesh.triangles[i])
		{
			used[point] = true;
		}
	}

	for (int i = 0;i < mesh.positions.size();i++)
	{
		if (!used[i] || !state.vertexAlive[i]) continue;

		oldToNew[i] = result.positions.size();
		result.positions.push_back(mesh.positions[i]);
	}

	for (int i = 0; i < mesh.triangles.size(); i++)
	{
		if (!state.triangleAlive[i]) continue;

		result.triangles.push_back({
			oldToNew[mesh.triangles[i][0]],
			oldToNew[mesh.triangles[i][1]],
			oldToNew[mesh.triangles[i][2]]
		});
	}
	return result;
}

Mesh SimplifyMesh(
	const Mesh& source,
	std::size_t targetTriangleCount
)
{
	Mesh workMesh = source;

	auto state = buildSimplifyState(workMesh);
	auto quadrics = buildQuadric(workMesh);
	auto queue = buildCandidateQueue(workMesh, quadrics, state);

	std::size_t remainingTriangle = workMesh.triangles.size();

	while (remainingTriangle > targetTriangleCount && queue.size())
	{
		CollapseCandidate candidate = queue.top();
		queue.pop();

		auto result = CollapseEdge(workMesh, state, quadrics, candidate);

		if (!result.success)
			continue;
		else
		{
			remainingTriangle -= result.removedTriangleCount;

			updateLocalCandidates(workMesh, state, quadrics, result.affectedVertices, queue);
		}
	}

	return compactMesh(workMesh, state);
}
