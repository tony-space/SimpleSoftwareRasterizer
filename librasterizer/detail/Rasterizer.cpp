#pragma once

#include <rasterizer/Rasterizer.hpp>

namespace rasterizer {

void Rasterizer::setTexture(Texture texture) noexcept
{
	m_texture = std::move(texture);
}

void Rasterizer::setMesh(Mesh mesh) noexcept
{
	m_mesh = std::move(mesh);
}

void Rasterizer::draw(unsigned width, unsigned height, std::vector<gamma_bgra_t>& out)
{
	setViewport(width, height);
	cleanBuffers();
	updateScene();

	swapBuffers(out);
}

void Rasterizer::setViewport(unsigned width, unsigned height)
{
	m_framebuffer.resize(size_t(width) * size_t(height));
	m_depthbuffer.resize(size_t(width) * size_t(height));
}

void Rasterizer::updateScene()
{
	m_parameters.rotateDeg.x += 0.25f;
	m_parameters.rotateDeg.y += 0.25f;
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