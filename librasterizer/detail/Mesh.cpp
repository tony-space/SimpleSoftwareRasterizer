#include <stdexcept>

#include <rasterizer/Mesh.hpp>

namespace rasterizer {

Mesh::Mesh(unsigned verticesCount, unsigned trinaglesCount) : m_verticesCount(verticesCount), m_trianglesCount(trinaglesCount)
{
}

void Mesh::setPositions(std::vector<glm::vec3> positions)
{
	if (positions.size() != m_verticesCount)
		throw std::invalid_argument("incorrect size");

	m_positions = std::move(positions);
}

void Mesh::setNormals(std::vector<glm::vec3> normals)
{
	if (normals.size() != m_verticesCount)
		throw std::invalid_argument("incorrect size");

	m_normals = std::move(normals);
}

void Mesh::setTexCoords0(std::vector<glm::vec2> texCoords)
{
	if (texCoords.size() != m_verticesCount)
		throw std::invalid_argument("incorrect size");

	m_texCoords0 = std::move(texCoords);
}

void Mesh::setTriangles(std::vector<glm::u16vec3> triangles)
{
	if (triangles.size() != m_trianglesCount)
		throw std::invalid_argument("incorrect size");

	if (std::any_of(triangles.cbegin(), triangles.cend(), [&](const glm::u16vec3& triangle)
	{
		return triangle.x >= m_verticesCount || triangle.y >= m_verticesCount || triangle.z >= m_verticesCount;
	}))
	{
		throw std::invalid_argument("invalid index detected");
	}

	m_triangles = std::move(triangles);
}

const std::vector<glm::vec3>& Mesh::positions() const noexcept
{
	return m_positions;
}

const std::vector<glm::vec3>& Mesh::normals() const noexcept
{
	return m_normals;
}

const std::vector<glm::vec2>& Mesh::texCoords0() const noexcept
{
	return m_texCoords0;
}

const std::vector<glm::u16vec3>& Mesh::triangles() const noexcept
{
	return m_triangles;
}

}