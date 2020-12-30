#pragma once

#include <vector>

#include "../../detail/glm-include.hpp"

#include "gamma_bgra_t.hpp"
#include "linear_rgba_t.hpp"
#include "Texture.hpp"

namespace rasterizer {

class Rasterizer final
{
public:
	Rasterizer() = default;
	~Rasterizer() = default;
	Rasterizer(const Rasterizer&) = delete;
	Rasterizer(Rasterizer&) = delete;

	Rasterizer& operator=(const Rasterizer&) = delete;
	Rasterizer& operator=(Rasterizer&&) = delete;

	void setTexture(Texture texture) noexcept;

	void draw(unsigned width, unsigned height, std::vector<gamma_bgra_t>& out);
private:
	std::vector<linear_rgba_t> m_framebuffer;
	std::vector<float> m_depthbuffer;

	struct Parameters
	{
		float m_verticalFov{ 60.0f };
		float m_zNear{ 0.5f };
		float m_zFar{ 5.0f };
		glm::vec3 lightPos{ glm::normalize(glm::vec3{0.0f, 0.0f, -1.0f}) };

		glm::vec3 translate;
		glm::vec3 rotate;
		glm::vec3 scale;
	} m_parameters;

	struct MatrixState
	{
		glm::mat4 viewport;
		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 model;
		glm::mat4 normal;
	} m_matrices;

	Texture m_texture{ 0, 0, {} };

	void setViewport(unsigned width, unsigned height);
	void cleanBuffers();
	void swapBuffers(std::vector<gamma_bgra_t>& out);
};

}