#include <rasterizer/finally.hpp>
#include <rasterizer/obj-loader.hpp>

#include "Application.hpp"

namespace gtk
{
	Application::Application(int argc, char **argv) : m_argc(argc), m_argv(argv)
	{
		m_app = gtk_application_new("space.vasin.rasterizer", G_APPLICATION_FLAGS_NONE);
		g_signal_connect(m_app, "activate", G_CALLBACK(activate), this);
	}

	Application::~Application()
	{
		g_object_unref(m_app);
	}

	void Application::activate(GtkApplication *app, gpointer self)
	{
		reinterpret_cast<Application *>(self)->activateImpl(app);
	}
	gboolean Application::onDraw(GtkWidget *widget, cairo_t *cr, gpointer self)
	{
		return reinterpret_cast<Application *>(self)->onDrawImpl(widget, cr);
	}

	int Application::run()
	{
		return g_application_run(G_APPLICATION(m_app), m_argc, m_argv);
	}

	rasterizer::Texture Application::loadTexture(std::filesystem::path path)
	{
		GError *error{nullptr};
		GdkPixbuf *buf = gdk_pixbuf_new_from_file(path.c_str(), &error);
		if (error)
		{
			g_error_free(error);
			throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
		}
		const auto _ = rasterizer::finally([&]
		{
			g_object_unref(buf);
		});

		const auto width = unsigned(gdk_pixbuf_get_width(buf));
		const auto height = unsigned(gdk_pixbuf_get_height(buf));
		const auto rowStride = size_t(gdk_pixbuf_get_rowstride(buf));
		const auto channels = size_t(gdk_pixbuf_get_n_channels(buf));
		const auto hasAlpha = bool(gdk_pixbuf_get_has_alpha(buf));
		const auto data = gdk_pixbuf_read_pixels(buf);

		std::vector<rasterizer::gamma_bgra_t> result(width * height);

		#pragma omp parallel for collapse(2)
		for (size_t row = 0; row != height; ++row)
		{
			for (size_t column = 0; column != width; ++column)
			{
				const auto rowStart = row * rowStride;
				const auto pixelOffset = rowStart + column * channels;

				const auto red = data[pixelOffset + 0];
				const auto green = data[pixelOffset + 1];
				const auto blue = data[pixelOffset + 2];
				const auto alpha = hasAlpha ? data[pixelOffset + 3] : uint8_t(255);
				result[row * width + column] = {blue, green, red, alpha};
			}
		}

		return rasterizer::Texture{width, height, std::move(result)};
	}

	void Application::activateImpl(GtkApplication *app)
	{
		m_window = gtk_application_window_new(app);
		gtk_window_set_title(GTK_WINDOW(m_window), "SimpleSoftwareRasterizer");
		gtk_window_set_default_size(GTK_WINDOW(m_window), 400, 300);
		g_signal_connect(m_window, "draw", G_CALLBACK(onDraw), this);
		m_image = gtk_image_new();
		gtk_container_add(GTK_CONTAINER(m_window), m_image);

		//m_rasterizer.setTexture(loadTexture("../assets/Jupiter.bmp"));
		m_rasterizer.setTexture(loadTexture("../assets/UV_Grid_Sm.bmp"));
		//m_rasterizer.setTexture(loadTexture("../assets/Moon.bmp"));
		//m_rasterizer.setTexture(loadTexture("../assets/white.bmp"));
		//m_rasterizer.setMesh(rasterizer::Mesh::sphere());
		//m_rasterizer.setMesh(rasterizer::Mesh::cube());
		m_rasterizer.setMesh(rasterizer::loaders::loadObj("../assets/bunny.obj"));

		gtk_widget_show_all(m_window);
	}

	gboolean Application::onDrawImpl(GtkWidget *widget, cairo_t *cr)
	{
		gint width, height;
		gtk_window_get_size(GTK_WINDOW(m_window), &width, &height);

		m_rasterizer.draw(width, height, m_framebuffer);

		#pragma omp parallel for collapse(2)
		for (int y = 0; y != height / 2; ++y)
		{
			for (int x = 0; x != width; ++x)
			{
				const auto rowStart1 = y * width;
				const auto rowStart2 = (height - y - 1) * width;
				auto pixel1 = m_framebuffer[rowStart1 + x];
				auto pixel2 = m_framebuffer[rowStart2 + x];
				std::swap(pixel1.r, pixel1.b);
				std::swap(pixel2.r, pixel2.b);
				m_framebuffer[rowStart1 + x] = pixel2;
				m_framebuffer[rowStart2 + x] = pixel1;
			}
		}

		auto pixBuf = gdk_pixbuf_new_from_data(
			reinterpret_cast<const guchar *>(m_framebuffer.data()),
			GdkColorspace::GDK_COLORSPACE_RGB,
			true,
			8,
			width,
			height,
			width * sizeof(rasterizer::gamma_bgra_t),
			nullptr,
			nullptr);
		const auto _ = rasterizer::finally([&]
		{
			g_object_unref(pixBuf);
		});
		gtk_image_set_from_pixbuf(GTK_IMAGE(m_image), pixBuf);

		static auto last = std::chrono::high_resolution_clock::now();
		static unsigned framesCounter = 0;

		auto cur = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(cur - last).count();
		framesCounter++;

		if (duration >= 2000)
		{
			gtk_window_set_title(GTK_WINDOW(m_window), std::to_string(float(framesCounter) / 2.0f).c_str());
			framesCounter = 0;
			last = cur;
		}

		return false;
	}

} // namespace gtk