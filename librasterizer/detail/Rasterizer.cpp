#pragma once

#include <rasterizer/Rasterizer.hpp>

namespace rasterizer {

void Rasterizer::setTexture(Texture texture) noexcept
{
	m_texture = std::move(texture);
}

void Rasterizer::draw(unsigned width, unsigned height, std::vector<gamma_bgra_t>& out)
{
	setViewport(width, height);
	cleanBuffers();

	std::for_each(m_framebuffer.begin(), m_framebuffer.end(), [&](linear_rgba_t& out)
	{
		const auto idx = std::distance(m_framebuffer.data(), &out);
		const auto [yPixel, xPixel] = std::div(idx, width);
		const auto point = glm::vec2(float(xPixel), float(yPixel));

		const auto texel = m_texture.sample(point / glm::vec2(width, height)) * 255.0f;
		out.r = uint8_t(texel.r);
		out.g = uint8_t(texel.g);
		out.b = uint8_t(texel.b);
	});

	swapBuffers(out);
}

void Rasterizer::setViewport(unsigned width, unsigned height)
{
	m_framebuffer.resize(size_t(width) * size_t(height));
	m_depthbuffer.resize(size_t(width) * size_t(height));
}

void Rasterizer::cleanBuffers()
{
	std::fill(std::execution::par_unseq, m_framebuffer.begin(), m_framebuffer.end(), linear_rgba_t{ 0, 0, 0, 0 });
	std::fill(std::execution::par_unseq, m_depthbuffer.begin(), m_depthbuffer.end(), 0.0f);
}

void Rasterizer::swapBuffers(std::vector<gamma_bgra_t>& out)
{
	out.resize(m_framebuffer.size());
	std::transform(std::execution::par_unseq, m_framebuffer.cbegin(), m_framebuffer.cend(), out.begin(), &gamma_bgra_t::from);
}

}