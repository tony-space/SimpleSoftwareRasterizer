#pragma once

#include <array>
#include <vector>

#include "../../detail/glm-include.hpp"
#include "../../detail/Tile.hpp"
#include "../../detail/Vertex.hpp"

#include "gamma_bgra_t.hpp"
#include "linear_rgba_t.hpp"
#include "Mesh.hpp"
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
	void setMesh(Mesh mesh) noexcept;

	void draw(unsigned width, unsigned height, std::vector<gamma_bgra_t>& out);
private:
	struct Framebuffer
	{
		glm::uvec2 screenSize;
		glm::uvec2 gridDim;
		std::vector<Tile> grid;
	} m_framebuffer;

	struct Parameters
	{
		float verticalFovDeg{ 60.0f };
		float zNear{ 0.5f };
		float zFar{ 5.0f };
		glm::vec3 lightPos{ glm::normalize(glm::vec3{1.0f, 1.0f, -1.0f}) };

		glm::vec3 translate{ 0.0f, -1.0f, 3.0f };
		glm::vec3 rotateDeg{ 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
	} m_parameters;

	struct Pipeline
	{
		struct MatrixState
		{
			glm::mat4 viewport;
			glm::mat4 projection;
			glm::mat4 modelView;
			glm::mat4 normal;
		} matrices;

		struct VertexStageOutput
		{
			std::vector<glm::vec4> positions;
			std::vector<glm::vec3> normals;
		} vertexStageOutput;

		std::vector<std::array<Vertex, 3>> projectedTriangles;
	} m_pipeline;


	Texture m_texture{ 0, 0, {} };
	Mesh m_mesh{ 0, 0 };

	void resetViewport(unsigned width, unsigned height);
	void updateScene();
	void runPipleine();
	void vertexStage();
	void clippingStage();
	void viewportTransformStage();
	void rasterizationStage();
	void swapBuffers(std::vector<gamma_bgra_t>& out);
};

}