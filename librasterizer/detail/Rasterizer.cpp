#pragma once

#include "basic-matrices.hpp"
#include "clipping.hpp"
#include "BoundingBox2D.hpp"

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
	resetViewport(width, height);
	updateScene();
	runPipleine();
	swapBuffers(out);
}

void Rasterizer::resetViewport(unsigned width, unsigned height)
{
	m_framebuffer.screenSize = { width, height };
	m_framebuffer.gridDim = Tile::computeGridDim(m_framebuffer.screenSize);
	m_framebuffer.grid.resize(size_t(m_framebuffer.gridDim.x) * size_t(m_framebuffer.gridDim.y));
	
	m_pipeline.matrices.viewport = matrices::viewportTransformMatrix(float(width), float(height));
	m_pipeline.matrices.projection = matrices::projectionMatrix(float(width), float(height), m_parameters.verticalFovDeg, m_parameters.zNear, m_parameters.zFar);
}

void Rasterizer::updateScene()
{
	static uint64_t frameIdx = 0;

	m_parameters.rotateDeg.x += 0.25f;
	m_parameters.rotateDeg.y += 0.25f;
	m_parameters.translate.z = glm::cos(frameIdx * 0.01f) + 1.0f;
	frameIdx++;

	m_pipeline.matrices.modelView = matrices::viewMatrix(m_parameters.rotateDeg, m_parameters.translate) * glm::scale(glm::identity<glm::mat4>(), m_parameters.scale);
	m_pipeline.matrices.normal = glm::transpose(glm::inverse(m_pipeline.matrices.modelView));
}

void Rasterizer::runPipleine()
{
	vertexStage();
	clippingStage();
	viewportTransformStage();
	rasterizationStage();
}

void Rasterizer::vertexStage()
{
	const auto& positions = m_mesh.positions();
	const auto& normals = m_mesh.normals();

	auto& positionsOut = m_pipeline.vertexStageOutput.positions;
	auto& normalsOut = m_pipeline.vertexStageOutput.normals;

	positionsOut.resize(positions.size());
	normalsOut.resize(normals.size());

	const auto modelViewProjectionMat = m_pipeline.matrices.projection * m_pipeline.matrices.modelView;

	std::transform(std::execution::par_unseq, positions.cbegin(), positions.cend(), positionsOut.begin(), [&](const glm::vec3& in)
	{
		return modelViewProjectionMat * glm::vec4(in, 1.0f);
	});

	std::transform(std::execution::par_unseq, normals.cbegin(), normals.cend(), normalsOut.begin(), [&](const glm::vec3& in) -> glm::vec3
	{
		return m_pipeline.matrices.normal * glm::vec4(in, 0.0f);
	});
}

void Rasterizer::clippingStage()
{
	typedef std::array<glm::vec4, 6> clipping_planes_storage_t;
	//homogeneous planes in the clipping space
	constexpr clipping_planes_storage_t clippingPlanes
	{
		glm::vec4{0.0f, 0.0f, 1.0f, 0.0f},	// near
		glm::vec4{0.0f, 0.0f, -1.0f, 1.0f},	// far
		glm::vec4{-1.0f, 0.0f, 0.0f, 1.0f},	// right
		glm::vec4{1.0f, 0.0f, 0.0f, 1.0f},	// left
		glm::vec4{0.0f, -1.0f, 0.0f, 1.0f},	// top
		glm::vec4{0.0f, 1.0f, 0.0f, 1.0f}	// bottom
	};


	m_pipeline.projectedTriangles.clear();

	for (const auto& trIn : m_mesh.triangles())
	{
		constexpr std::array<glm::vec2, 3> rootTriangle{ glm::vec2{1.0f, 0.0f}, glm::vec2{0.0f, 1.0f}, glm::vec2{0.0f, 0.0f} };

		clipping::RecursiveClipper<clipping_planes_storage_t> clipper
		{
			{
				Vertex{m_pipeline.vertexStageOutput.positions[trIn.x], m_pipeline.vertexStageOutput.normals[trIn.x], m_mesh.texCoords0()[trIn.x]},
				Vertex{m_pipeline.vertexStageOutput.positions[trIn.y], m_pipeline.vertexStageOutput.normals[trIn.y], m_mesh.texCoords0()[trIn.y]},
				Vertex{m_pipeline.vertexStageOutput.positions[trIn.z], m_pipeline.vertexStageOutput.normals[trIn.z], m_mesh.texCoords0()[trIn.z]}
			},
			clippingPlanes.begin(),
			clippingPlanes.end(),

			m_pipeline.projectedTriangles
		};

		clipper(rootTriangle);
	}
}

void Rasterizer::viewportTransformStage()
{
	std::for_each(std::execution::par_unseq, m_pipeline.projectedTriangles.begin(), m_pipeline.projectedTriangles.end(), [&](std::array<Vertex, 3>& triangle)
	{
		for (Vertex& v : triangle)
		{
			v.position = m_pipeline.matrices.viewport * v.position;
			//IMPORTANT: We must save the original Z value for further perspective-correct interpolation
			//Instead of getting (x,y,z,1) we store (x,y,z,originalZ)
			v.position = { v.position.xyz() / v.position.w, v.position.w };
		}
	});
}

void Rasterizer::rasterizationStage()
{
	std::for_each(std::execution::par_unseq, m_pipeline.projectedTriangles.cbegin(), m_pipeline.projectedTriangles.cend(), [&](const std::array<Vertex, 3>& triangle)
	{
		const auto triangleBox = BoundingBox2D{ triangle[0].position.xy(), triangle[1].position.xy(), triangle[2].position.xy() };
		const auto minPixel = glm::uvec2(glm::ceil(triangleBox.min()));
		const auto maxPixel = glm::min(glm::uvec2(glm::ceil(triangleBox.max())), m_framebuffer.screenSize - glm::uvec2(1));

		const auto minTile = minPixel / glm::uvec2(Tile::kSize);
		const auto maxTile = maxPixel / glm::uvec2(Tile::kSize);

		for (unsigned yTile = minTile.y; yTile <= maxTile.y; ++yTile)
		{
			const auto stride = m_framebuffer.gridDim.x * yTile;
			for (unsigned xTile = minTile.x; xTile <= maxTile.x; ++xTile)
			{
				const auto idx = stride + xTile;
				m_framebuffer.grid[idx].scheduleTriangle(triangle);
			}
		}
	});

	std::for_each(std::execution::par_unseq, m_framebuffer.grid.begin(), m_framebuffer.grid.end(), [&](Tile& tile)
	{
		const auto idx = std::distance(m_framebuffer.grid.data(), &tile);
		const auto [yTile, xTile] = std::div(idx, m_framebuffer.gridDim.x);

		const auto tileMin = glm::vec2(xTile, yTile) * glm::vec2(Tile::kSize);
		const auto tileMax = tileMin + glm::vec2(Tile::kSize);

		const auto tileBox = BoundingBox2D{ tileMin, tileMax };

		tile.rasterize(tileBox, { m_parameters.lightPos, m_texture});
	});
}

void Rasterizer::swapBuffers(std::vector<gamma_bgra_t>& out)
{
	out.resize(size_t(m_framebuffer.screenSize.x) * size_t(m_framebuffer.screenSize.y));

	std::for_each(std::execution::par_unseq, out.begin(), out.end(), [&](gamma_bgra_t& result)
	{
		const auto idx = std::distance(out.data(), &result);
		const auto [yPixel, xPixel] = std::div(idx, m_framebuffer.screenSize.x);
		const auto xTile = xPixel / Tile::kSize;
		const auto yTile = yPixel / Tile::kSize;
		const auto tileIdx = m_framebuffer.gridDim.x * yTile + xTile;
		const auto& tile = m_framebuffer.grid[tileIdx];

		result = gamma_bgra_t::from(tile.at(xPixel - xTile * Tile::kSize, yPixel - yTile * Tile::kSize));
	});
}

}