#pragma once

#include <filesystem>
#include <vector>

#include <rasterizer/gamma_bgra_t.hpp>
#include <rasterizer/Rasterizer.hpp>
#include <rasterizer/Texture.hpp>


namespace gdk {

class Application final
{
public:
	explicit Application(int argc, char** argv);
	~Application() = default;

	Application(const Application&) = delete;
	Application(Application&&) = delete;

	Application& operator=(const Application&) = delete;
	Application& operator=(Application&&) = delete;

	int run();
private:
	static Application* s_instance;
	static rasterizer::Texture loadTexture(std::filesystem::path path);

	rasterizer::gamma_bgra_t* draw(unsigned width, unsigned height);
	std::vector<rasterizer::gamma_bgra_t> m_framebuffer;
	rasterizer::Rasterizer m_rasterizer;
};

}