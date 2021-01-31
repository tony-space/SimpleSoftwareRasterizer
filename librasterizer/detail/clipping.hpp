#pragma once

#include <array>
#include <atomic>
#include <variant>
#include <vector>

#include "glm-include.hpp"
#include "Vertex.hpp"

namespace rasterizer {
namespace clipping {

struct discard
{
};
struct leaveAsIs
{
};

typedef std::variant<discard, leaveAsIs, std::array<glm::vec2, 3>, std::array<glm::vec2, 4>> triangle_clip_t;

triangle_clip_t triangleClip(const glm::vec3& planeDistances) noexcept;

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

	std::atomic_bool& outputLock;
	std::vector<std::array<Vertex, 3>>& output;

	//discard triangle
	void operator ()(const discard&)
	{

	}
	
	void operator ()(const leaveAsIs&)
	{
		std::terminate();
	}

	void operator ()(const std::array<glm::vec2, 3>& triangle)
	{
		const std::array<Vertex, 3> interpolatedTriangle{ interpolate(vertices, triangle[0]), interpolate(vertices, triangle[1]), interpolate(vertices, triangle[2]) };

		triangle_clip_t clippingResult{ leaveAsIs{} };
		
		auto plane = curPlane;
		
		while (true)
		{
			if (plane == endPlane)
			{
				emitTriangle(interpolatedTriangle);
				return;
			}

			clippingResult = triangleClip({
				glm::dot(*plane, interpolatedTriangle[0].position),
				glm::dot(*plane, interpolatedTriangle[1].position),
				glm::dot(*plane, interpolatedTriangle[2].position) });

			if (clippingResult.index() != 1)
				break;
			plane++;
		}

		RecursiveClipper clipper
		{
			interpolatedTriangle,
			plane + 1,
			endPlane,
			outputLock,
			output
		};
		std::visit(clipper, clippingResult);
	}

	void emitTriangle(const std::array<rasterizer::Vertex, 3>& interpolatedTriangle)
	{
		try
		{
			bool expected = false;
			bool desired = true;
			while (!outputLock.compare_exchange_weak(expected, desired, std::memory_order::memory_order_acquire, std::memory_order::memory_order_relaxed))
			{
				expected = false;
			}

			output.emplace_back(interpolatedTriangle);
		}
		catch (...)
		{
			outputLock.store(false, std::memory_order::memory_order_release);
			throw;
		}
		outputLock.store(false, std::memory_order::memory_order_release);
	}

	void operator ()(const std::array<glm::vec2, 4>& quad)
	{
		this->operator()(std::array<glm::vec2, 3>{quad[0], quad[1], quad[2]});
		this->operator()(std::array<glm::vec2, 3>{quad[0], quad[2], quad[3]});
	}
};


}
}