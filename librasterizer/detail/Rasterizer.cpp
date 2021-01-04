#pragma once

#include "basic-matrices.hpp"
#include "clipping.hpp"

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
	m_framebuffer.width = width;
	m_framebuffer.height = height;
	m_framebuffer.color.resize(size_t(width) * size_t(height));
	m_framebuffer.depth.resize(size_t(width) * size_t(height));
	std::fill(std::execution::par_unseq, m_framebuffer.color.begin(), m_framebuffer.color.end(), linear_rgba_t{ 0, 0, 0, 0 });
	std::fill(std::execution::par_unseq, m_framebuffer.depth.begin(), m_framebuffer.depth.end(), 1.0f);

	m_pipeline.matrices.viewport = matrices::viewportTransformMatrix(float(width), float(height));
	m_pipeline.matrices.projection = matrices::projectionMatrix(float(width), float(height), m_parameters.verticalFovDeg, m_parameters.zNear, m_parameters.zFar);
}

void Rasterizer::updateScene()
{
	m_parameters.rotateDeg.x += 0.25f;
	m_parameters.rotateDeg.y += 0.25f;

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
	for (const auto& triangle : m_pipeline.projectedTriangles)
	{
		std::for_each(std::execution::par_unseq, m_framebuffer.color.begin(), m_framebuffer.color.end(), [&](linear_rgba_t& out)
		{
			const auto idx = std::distance(m_framebuffer.color.data(), &out);
			auto& depthValue = m_framebuffer.depth[idx];
			const auto [yPixel, xPixel] = std::div(idx, m_framebuffer.width);
			const auto point = glm::vec2(float(xPixel), float(yPixel));

			const auto areas = barycentric(triangle[0].position, triangle[1].position, triangle[2].position, point);
			const auto barycentricPos = glm::vec3(areas) / areas.w;

			//render only the front side
			if (!(areas.x >= 0.0f && areas.y >= 0.0f && areas.z >= 0.0f))
				return;
			//render both sides of the triangle
			//if (!(barycentricPos.x >= 0.0f && barycentricPos.y >= 0.0f && barycentricPos.z >= 0.0f))
			//	return;

			const auto barycentricPerZ = barycentricPos / glm::vec3(triangle[0].position.w, triangle[1].position.w, triangle[2].position.w);					// (alpha/Za, beta/Zb, gamma/Zc)
			const auto interpolatedOriginalZ = 1.0f / (barycentricPerZ.x + barycentricPerZ.y + barycentricPerZ.z);												// 1 / (alpha/Za + beta/Zb + gamma/Zc)
			const auto interpolatedNormalizedZ = glm::dot(barycentricPos, glm::vec3(triangle[0].position.z, triangle[1].position.z, triangle[2].position.z));	// alpha * Zna + beta * Znb + gamma * Znb

			// depth test
			if (interpolatedNormalizedZ > depthValue)
				return;
			//depth write
			depthValue = interpolatedNormalizedZ;

			//perspective correct interpolations
			const auto interpolatedNormal = interpolate(barycentricPerZ, interpolatedOriginalZ, triangle[0].normal, triangle[1].normal, triangle[2].normal);
			const auto interpolatedTc = interpolate(barycentricPerZ, interpolatedOriginalZ, triangle[0].texCoord0, triangle[1].texCoord0, triangle[2].texCoord0);
			/*const auto interpolatedNormal = triangle[0].normal * barycentricPos.x + triangle[1].normal * barycentricPos.y + triangle[2].normal * barycentricPos.z;
			const auto interpolatedTc = triangle[0].texCoord0 * barycentricPos.x + triangle[1].texCoord0 * barycentricPos.y + triangle[2].texCoord0 * barycentricPos.z;*/

			//Lambertian BRDF
			const auto diffuse = glm::clamp(glm::dot(glm::normalize(interpolatedNormal.xyz()), m_parameters.lightPos), 0.1f, 1.0f);
			//const auto color = glm::u8vec3(diffuse * sampleTexture(texture, interpolatedTc));
			//out = linear_bgra_t{ color.b, color.g, color.r, 255 };
			const auto result = glm::u8vec3(diffuse * m_texture.sample(interpolatedTc) * 255.0f);
			out = {
				result.x,
				result.y,
				result.z
			};
		});
	}
}

void Rasterizer::swapBuffers(std::vector<gamma_bgra_t>& out)
{
	out.resize(m_framebuffer.color.size());
	std::transform(std::execution::par_unseq, m_framebuffer.color.cbegin(), m_framebuffer.color.cend(), out.begin(), &gamma_bgra_t::from);
}

glm::vec4 Rasterizer::barycentric(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& point) noexcept
{
	const auto Sa = glm::cross(glm::vec3(c - b, 0.0f), glm::vec3(point - b, 0.0f)).z;
	const auto Sb = glm::cross(glm::vec3(a - c, 0.0f), glm::vec3(point - c, 0.0f)).z;
	const auto Sc = glm::cross(glm::vec3(b - a, 0.0f), glm::vec3(point - a, 0.0f)).z;

	const auto S = Sa + Sb + Sc;
	//const auto S = glm::cross(glm::vec3(b - a, 0.0f), glm::vec3(c - a, 0.0f)).z;

	return
	{
		Sa,
		Sb,
		Sc,
		S
	};
}

}