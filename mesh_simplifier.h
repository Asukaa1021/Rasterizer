#pragma once
#include "raster_math.h"
#include "mesh.h"
#include <array>
#include <vector>
#include <queue>
#include <unordered_map>
#include <cstdint>
#include <cstddef>

bool get_plane(const Vec3& A, const Vec3& B, const Vec3& C, std::array<double, 4>& plane);

struct Quadric
{
	double q[4][4]{};

	void addPlane(const std::array<double, 4>& plane);
	double evaluate(const Vec3& position) const;
	Quadric operator+(const Quadric& b) const;
};

std::vector<Quadric> buildQuadric(const Mesh& mesh);

bool solveOptimcalPosition(
	const Quadric& quadric,
	Vec3& position
);


struct CollapseCandidate
{
	int a;
	int b;
	double cost;
	Vec3 position;
	std::uint64_t versionA;
	std::uint64_t versionB;
};

struct SimplifyState
{
	std::vector<bool> vertexAlive;
	std::vector<bool> triangleAlive;

	std::vector<std::vector<int>> vertexTriangles;

	std::vector<std::uint64_t> vertexVersion;
};

CollapseCandidate makeCandidate(
	const Mesh& mesh,
	const std::vector<Quadric>& quadrics,
	const SimplifyState& state,
	int a,
	int b
);

std::vector<std::array<int, 2>> collectEdge(const Mesh& mesh);

struct CandidateCompare
{
	bool operator()(
		const CollapseCandidate& a, 
		const CollapseCandidate& b
	)const
	{
		return a.cost > b.cost;
	}
};

struct CollapseResult {
	bool success = false;
	int removedTriangleCount = 0;
	std::vector<int> affectedVertices;
};

using candidateQueue = std::priority_queue<
	CollapseCandidate, 
	std::vector<CollapseCandidate>, 
	CandidateCompare
>;

candidateQueue buildCandidateQueue(
	const Mesh& mesh,
	const std::vector<Quadric>& quadrics,
	const SimplifyState& state
);

std::vector<int> collectVertexNeighbors(
	const Mesh& mesh,
	const SimplifyState& state,
	int vertex
);

SimplifyState buildSimplifyState(const Mesh& mesh);

std::vector<int> collectAffectTriangles(
	const SimplifyState& state,
	int a,
	int b
);

bool preservesGeometry(
	const Mesh& mesh,
	const SimplifyState& state,
	const CollapseCandidate& candidate
);

std::vector<int> collectEdgeTriangles(
	const Mesh& mesh,
	const SimplifyState& state,
	int a,
	int b
);

bool isBoundaryVertex(
	const Mesh& mesh,
	const SimplifyState& state,
	int vertex
);

bool hasExpectedCommonNeighbors(
	const Mesh& mesh,
	const SimplifyState& state,
	int a,
	int b
);

bool preservesFaceUniqueness(
	const Mesh& mesh,
	const SimplifyState& state,
	int a,
	int b
);

bool canCollapse(
	const Mesh& mesh,
	const SimplifyState& state,
	const CollapseCandidate& candidtate
);

CollapseResult CollapseEdge(
	Mesh& mesh,
	SimplifyState& state,
	std::vector<Quadric>& quadrics,
	const CollapseCandidate& candidate
);

bool isCandidateCurrent(
	const SimplifyState& state,
	const CollapseCandidate& candidate
);

void updateLocalCandidates(
	const Mesh& mesh,
	const SimplifyState& state,
	const std::vector<Quadric>& quadrics,
	const std::vector<int>& affectedVertices,
	candidateQueue& queue
);

Mesh compactMesh(
	const Mesh& mesh,
	const SimplifyState& state
);

Mesh SimplifyMesh(
	const Mesh& source,
	std::size_t targetTriangleCount
);

