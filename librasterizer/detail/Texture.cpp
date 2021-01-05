#include <rasterizer/Texture.hpp>

#include "LookUpTable.hpp"

namespace rasterizer {

static detail::LookUpTable<float> fromGammaTable{ [](float& v)
{
	float value = v / 255.0f;
	v = std::pow(value, 2.2f);
} };

Texture::Texture(unsigned width, unsigned height, const std::vector<gamma_bgra_t>& bitmap) :
	m_width(width),
	m_height(height)
{
	if (bitmap.size() != width * height)
	{
		throw std::invalid_argument("incorrect image size");
	}

	m_texels.resize(bitmap.size());

	std::transform(std::execution::par_unseq, bitmap.cbegin(), bitmap.cend(), m_texels.begin(), [](const gamma_bgra_t& gamma)
	{
		return glm::vec4
		{
			fromGammaTable[gamma.r],
			fromGammaTable[gamma.g],
			fromGammaTable[gamma.b],
			fromGammaTable[gamma.a],
		};
	});
}

glm::vec4 Texture::sample(glm::vec2 textureCoords) const noexcept
{
	textureCoords = glm::clamp(textureCoords, glm::vec2(0.0f), glm::vec2(1.0f));

	const auto texelCoords = glm::uvec2(textureCoords.s * (m_width - 1), textureCoords.t * (m_height - 1));
	const auto texelIdx = texelCoords.y * m_width + texelCoords.x;
	return m_texels[texelIdx];
}

}