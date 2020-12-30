#include <rasterizer/Texture.hpp>

namespace rasterizer {

Texture::Texture(unsigned width, unsigned height, const std::vector<gamma_bgra_t>& bitmap) :
	m_width(width),
	m_height(height)
{
	if (bitmap.size() != width * height)
	{
		throw std::invalid_argument("incorrect image size");
	}

	m_texels.resize(bitmap.size());

	std::transform(std::execution::par_unseq, bitmap.cbegin(), bitmap.cend(), m_texels.begin(), &linear_rgba_t::from);
}

glm::vec3 Texture::sample(glm::vec2 textureCoords) const noexcept
{
	textureCoords = glm::clamp(textureCoords, glm::vec2(0.0f), glm::vec2(1.0f));

	const auto texelCoords = glm::uvec2(textureCoords.s * (m_width - 1), textureCoords.t * (m_height - 1));
	const auto texelIdx = texelCoords.y * m_width + texelCoords.x;
	//const auto& texelColor = m_texels.at(texelIdx);
	const auto& texelColor = m_texels[texelIdx];

	return
	{
		texelColor.red<float>(),
		texelColor.green<float>(),
		texelColor.blue<float>(),
	};
}

}