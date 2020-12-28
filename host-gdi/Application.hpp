#pragma once

#include <minwindef.h>

#include <vector>

#include <rasterizer/gamma_bgra_t.hpp>

namespace gdi {

class Application final
{
public:
	Application(HINSTANCE hInstance, int nShowCmd) noexcept;
	Application(const Application&) = delete;
	Application(Application&&) = delete;

	Application& operator=(const Application&) = delete;
	Application& operator=(Application&&) = delete;

	int run() noexcept;
private:
	static Application* s_instance;
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	rasterizer::gamma_bgra_t* draw(unsigned width, unsigned height);

	std::vector<rasterizer::gamma_bgra_t> m_framebuffer;
};

}