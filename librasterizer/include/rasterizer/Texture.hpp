#pragma once

#include <vector>

#include "gamma_bgra_t.hpp"
#include "linear_rgba_t.hpp"

namespace rasterizer {

class Texture final
{
public:
	explicit Texture(unsigned width, unsigned height, const std::vector<gamma_bgra_t>& bitmap);
	~Texture() = default;
	Texture(const Texture&) = default;
	Texture(Texture&&) noexcept = default;

	Texture& operator=(const Texture&) = default;
	Texture& operator=(Texture&&) noexcept = default;

	glm::vec3 sample(glm::vec2 textureCoords) const noexcept;

private:
	std::vector<linear_rgba_t> m_texels;
	unsigned m_width;
	unsigned m_height;
};

}