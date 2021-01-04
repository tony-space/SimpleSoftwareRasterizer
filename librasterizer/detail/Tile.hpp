#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <vector>

#include <rasterizer/linear_rgba_t.hpp>

#include "glm-include.hpp"
#include "Vertex.hpp"

namespace rasterizer {

class Texture;
class BoundingBox2D;

class Tile final
{
public:
	struct UniformData
	{
		glm::vec3 lightPos;
		Texture& texture;
	};

	static constexpr size_t kSize = 64;

	Tile() = default;
	Tile(const Tile&) = delete;
	Tile(Tile&&) noexcept = default;
	~Tile() = default;

	Tile& operator= (const Tile&) = delete;
	Tile& operator=(Tile&&) noexcept = default;

	void scheduleTriangle(const std::array<Vertex, 3>& triangle);
	void rasterize(const BoundingBox2D& tileBox, const UniformData& uniforms) noexcept;
	linear_rgba_t at(size_t x, size_t y) const noexcept;

	static glm::uvec2 computeGridDim(glm::uvec2 screenSize) noexcept;
private:
	std::vector<const std::array<Vertex, 3>*> m_triangles;
	std::array<glm::vec4, kSize * kSize> m_color{};
	std::array<float, kSize* kSize> m_depth{};
	std::unique_ptr<std::atomic_bool> m_lock{std::make_unique<std::atomic_bool>(false)};

	void drawImpl(const UniformData& uniforms, const std::array<Vertex, 3>& triangle, glm::vec3 barycentricPos, glm::vec4& color, float& depth) noexcept;

	static glm::vec4 barycentric(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& point) noexcept;
};

}