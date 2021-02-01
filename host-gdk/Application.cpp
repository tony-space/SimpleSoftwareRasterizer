#include <rasterizer/obj-loader.hpp>

#include "Application.hpp"

namespace gdk {

Application* Application::s_instance{ nullptr };

Application::Application(int argc, char** argv)
{
	//m_rasterizer.setTexture(loadTexture("../assets/Jupiter.bmp"));
	m_rasterizer.setTexture(loadTexture("../assets/UV_Grid_Sm.bmp"));
	//m_rasterizer.setTexture(loadTexture("../assets/Moon.bmp"));
	//m_rasterizer.setTexture(loadTexture("../assets/white.bmp"));
	//m_rasterizer.setMesh(rasterizer::Mesh::sphere());
	//m_rasterizer.setMesh(rasterizer::Mesh::cube());
	m_rasterizer.setMesh(rasterizer::loaders::loadObj("../assets/bunny.obj"));
}

int Application::run()
{
	return -1;
}

rasterizer::Texture Application::loadTexture(std::filesystem::path path)
{
	return rasterizer::Texture {0, 0, std::vector<rasterizer::gamma_bgra_t>{}};
}

rasterizer::gamma_bgra_t* Application::draw(unsigned width, unsigned height)
{
	m_rasterizer.draw(width, height, m_framebuffer);
	return m_framebuffer.data();
}

}