#pragma once

#include <array>
#include <variant>
#include <vector>

#include "glm-include.hpp"
#include "Vertex.hpp"

namespace rasterizer {
namespace clipping {

/// <summary>
/// Clips the triangle.
/// Given the distances from each vertex to the plane the function produces zero, one, or two triangles.
/// </summary>
/// <param name="planeDistances">distances from each vertex to the clipping plane</param>
/// <returns>either an empty result or barycentric coordinates (alpha, beta, 1 - alpha - beta) of a new polygon relative to the original triangle</returns>
std::variant<std::monostate, std::array<glm::vec2, 3>, std::array<glm::vec2, 4>> triangleClip(const glm::vec3& planeDistances) noexcept;

template <typename T>
T interpolate(const typename std::array<T, 3>& props, const glm::vec2& barycentric)
{
	return props[0] * barycentric.x + props[1] * barycentric.y + props[2] * (1.0f - barycentric.x - barycentric.y);
}

template <typename TClippingPlanesType>
struct RecursiveClipper
{
	const std::array<Vertex, 3>& vertices;

	const typename TClippingPlanesType::const_iterator curPlane;
	const typename TClippingPlanesType::const_iterator endPlane;

	std::vector<std::array<Vertex, 3>>& output;

	template<typename T>
	void operator ()(const T&);

	//discard triangle
	template<>
	void operator ()(const std::monostate& /*empty*/)
	{

	}

	//write triangle
	template<>
	void operator ()(const std::array<glm::vec2, 3>& triangle)
	{
		const std::array<Vertex, 3> interpolatedTriangle{ interpolate(vertices, triangle[0]), interpolate(vertices, triangle[1]), interpolate(vertices, triangle[2]) };

		if (curPlane == endPlane)
		{
			output.emplace_back(interpolatedTriangle);
			return;
		}

		const auto clippingResult = triangleClip({
			glm::dot(*curPlane, interpolatedTriangle[0].position),
			glm::dot(*curPlane, interpolatedTriangle[1].position),
			glm::dot(*curPlane, interpolatedTriangle[2].position) });

		RecursiveClipper clipper
		{
			interpolatedTriangle,
			curPlane + 1,
			endPlane,
			output
		};
		std::visit(clipper, clippingResult);
	}

	template<>
	void operator ()(const std::array<glm::vec2, 4>& quad)
	{
		this->operator()(std::array<glm::vec2, 3>{quad[0], quad[1], quad[2]});
		this->operator()(std::array<glm::vec2, 3>{quad[0], quad[2], quad[3]});
	}
};


}
}