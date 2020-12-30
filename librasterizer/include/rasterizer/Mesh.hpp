#pragma once

#include <vector>

#include "../../detail/glm-include.hpp"

namespace rasterizer {

class Mesh final
{
public:
	Mesh(unsigned verticesCount, unsigned trinaglesCount);
	~Mesh() = default;
	Mesh(const Mesh&) = default;
	Mesh(Mesh&&) noexcept = default;

	Mesh& operator=(const Mesh&) = default;
	Mesh& operator=(Mesh&&) noexcept = default;

	void setPositions(std::vector<glm::vec3> positions);
	void setNormals(std::vector<glm::vec3> normals);
	void setTexCoords0(std::vector<glm::vec2> texCoords);
	void setTriangles(std::vector<glm::u16vec3> triangles);

	const std::vector<glm::vec3>& positions() const noexcept;
	const std::vector<glm::vec3>& normals() const noexcept;
	const std::vector<glm::vec2>& texCoords0() const noexcept;
	const std::vector<glm::u16vec3>& triangles() const noexcept;

	static const Mesh& cube();

private:
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec2> m_texCoords0;
	std::vector<glm::u16vec3> m_triangles;

	unsigned m_verticesCount;
	unsigned m_trianglesCount;
};

}