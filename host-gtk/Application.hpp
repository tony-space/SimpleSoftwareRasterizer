#pragma once

#include <gtk/gtk.h>

#include <filesystem>
#include <vector>

#include <rasterizer/gamma_bgra_t.hpp>
#include <rasterizer/Rasterizer.hpp>
#include <rasterizer/Texture.hpp>

namespace gtk
{

	class Application final
	{
	public:
		explicit Application(int argc, char **argv);
		~Application();

		Application(const Application &) = delete;
		Application(Application &&) = delete;

		Application &operator=(const Application &) = delete;
		Application &operator=(Application &&) = delete;

		int run();

	private:
		rasterizer::Texture loadTexture(std::filesystem::path path);
		static void activate(GtkApplication *app, gpointer self);
		static gboolean onDraw(GtkWidget *widget, cairo_t *cr, gpointer self);

		int m_argc;
		char **m_argv;
		GtkApplication *m_app;
		GtkWidget *m_window;
		GtkWidget *m_image;

		void activateImpl(GtkApplication *app);
		gboolean onDrawImpl(GtkWidget *widget, cairo_t *cr);
		std::vector<rasterizer::gamma_bgra_t> m_framebuffer;
		rasterizer::Rasterizer m_rasterizer;
	};

} // namespace gtk