#include <rasterizer/Texture.hpp>

#include "BoundingBox2D.hpp"
#include "Tile.hpp"

namespace rasterizer {

template<typename T>
static T interpolate(const glm::vec3& barycentricPerZ, float interpolatedOriginalZ, const T& f1, const T& f2, const T& f3)
{
	return interpolatedOriginalZ * (barycentricPerZ.x * f1 + barycentricPerZ.y * f2 + barycentricPerZ.z * f3);
}

void Tile::scheduleTriangle(const std::array<Vertex, 3>& triangle) noexcept
{
	while (m_lock->exchange(true, std::memory_order::memory_order_acquire)) {};
	m_triangles.emplace_back(&triangle);
	m_lock->store(false, std::memory_order::memory_order_release);
}

void Tile::rasterize(const BoundingBox2D& tileBox, const UniformData& uniforms) noexcept
{
	std::fill(m_color.begin(), m_color.end(), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	std::fill(m_normal.begin(), m_normal.end(), glm::vec3(0.0f));
	std::fill(m_depth.begin(), m_depth.end(), 1.0f);

	for (const auto& trianglePtr : m_triangles)
	{
		const auto& triangle = *trianglePtr;

		for (size_t y = 0; y != kSize; ++y)
		{
			const auto stride = y * kSize;
			for (size_t x = 0; x != kSize; ++x)
			{
				const auto point = tileBox.min() + glm::vec2(x, y);
				const auto areas = barycentric(triangle[0].position, triangle[1].position, triangle[2].position, point);
				//render only the front side
				if (!(areas.x >= 0.0f && areas.y >= 0.0f && areas.z >= 0.0f))
					continue;
				const auto barycentricPos = glm::vec3(areas) / areas.w;

				/*if (barycentricPos.x > 0.01f && barycentricPos.y > 0.01f && barycentricPos.z > 0.01f)
					continue;*/

				//render both sides of the triangle
				/*if (!(barycentricPos.x >= 0.0f && barycentricPos.y >= 0.0f && barycentricPos.z >= 0.0f))
					continue;*/

				const auto idx = stride + x;
				drawImpl(uniforms, triangle, barycentricPos, m_color[idx], m_normal[idx], m_depth[idx]);
			}
		}
	}

	m_triangles.clear();
}

glm::vec4 Tile::colorAt(size_t x, size_t y) const noexcept
{
	return m_color[y * kSize + x];
}

glm::vec3 Tile::normalAt(size_t x, size_t y) const noexcept
{
	return m_normal[y * kSize + x];
}

float Tile::depthAt(size_t x, size_t y) const noexcept
{
	return m_depth[y * kSize + x];
}

glm::uvec2 Tile::computeGridDim(glm::uvec2 screenSize) noexcept
{
	return (screenSize - glm::uvec2(1)) / glm::uvec2(kSize) + glm::uvec2(1);
}

void Tile::drawImpl(const UniformData& uniforms, const std::array<Vertex, 3>& triangle, glm::vec3 barycentricPos, glm::vec4& color, glm::vec3& normal, float& depth) noexcept
{
	const auto barycentricPerZ = barycentricPos / glm::vec3(triangle[0].position.w, triangle[1].position.w, triangle[2].position.w);					// (alpha/Za, beta/Zb, gamma/Zc)
	const auto interpolatedOriginalZ = 1.0f / (barycentricPerZ.x + barycentricPerZ.y + barycentricPerZ.z);												// 1 / (alpha/Za + beta/Zb + gamma/Zc)
	const auto interpolatedNormalizedZ = glm::dot(barycentricPos, glm::vec3(triangle[0].position.z, triangle[1].position.z, triangle[2].position.z));	// alpha * Zna + beta * Znb + gamma * Znb

	// depth test
	if (interpolatedNormalizedZ > depth)
		return;
	//depth write
	depth = interpolatedNormalizedZ;

	//perspective correct interpolations
	const auto interpolatedNormal = interpolate(barycentricPerZ, interpolatedOriginalZ, triangle[0].normal, triangle[1].normal, triangle[2].normal);
	const auto interpolatedTc = interpolate(barycentricPerZ, interpolatedOriginalZ, triangle[0].texCoord0, triangle[1].texCoord0, triangle[2].texCoord0);

	color = uniforms.texture.sample(interpolatedTc);
	normal = glm::normalize(interpolatedNormal.xyz());
}

glm::vec4 Tile::barycentric(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& point) noexcept
{
	//the same as `glm::cross(x,y).z`;
	constexpr auto edgeFunc = [](const glm::vec3& x, const glm::vec3& y)
	{
		return x.x * y.y - y.x * x.y;
	};

	const auto Sa = edgeFunc(glm::vec3(c - b, 0.0f), glm::vec3(point - b, 0.0f));
	const auto Sb = edgeFunc(glm::vec3(a - c, 0.0f), glm::vec3(point - c, 0.0f));
	const auto Sc = edgeFunc(glm::vec3(b - a, 0.0f), glm::vec3(point - a, 0.0f));

	const auto S = Sa + Sb + Sc;
	return
	{
		Sa,
		Sb,
		Sc,
		S
	};
}

}