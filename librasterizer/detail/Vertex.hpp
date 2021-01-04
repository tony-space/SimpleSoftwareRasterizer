#pragma once

#include "glm-include.hpp"

namespace rasterizer {

struct Vertex
{
	glm::vec4 position;
	glm::vec3 normal;
	glm::vec2 texCoord0;

	friend Vertex operator* (const Vertex& lhs, float t) noexcept;
	friend Vertex operator+ (const Vertex& lhs, const Vertex& rhs) noexcept;
};

inline Vertex operator* (const Vertex& lhs, float t) noexcept
{
	return
	{
		lhs.position * t,
		lhs.normal * t,
		lhs.texCoord0 * t
	};
}

inline Vertex operator+ (const Vertex& lhs, const Vertex& rhs) noexcept
{
	return
	{
		lhs.position + rhs.position,
		lhs.normal + rhs.normal,
		lhs.texCoord0 + rhs.texCoord0
	};
}

}