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

void Mesh::computeNormals()
{
	union float_storage_t
	{
		float flt;
		uint32_t storage;

		float_storage_t() noexcept = default;
		float_storage_t(float val) noexcept : flt(val)
		{
		}

	};

	using atomic_vec3_t = std::array<std::atomic<float_storage_t>, 3>;

	constexpr auto atomicFloatAdd = [](std::atomic<float_storage_t>& flt, const float& val) noexcept
	{
		auto storage = flt.load(std::memory_order_relaxed);

		while (!flt.compare_exchange_weak(storage, float_storage_t(storage.flt + val), std::memory_order_relaxed, std::memory_order_relaxed))
		{
		}
	};

	auto normals = std::vector<atomic_vec3_t>(m_verticesCount);

	std::for_each(TRY_PARALLELIZE_PAR_UNSEQ m_triangles.cbegin(), m_triangles.cend(), [&](const glm::u16vec3& triangle) noexcept
	{
		const auto a = m_positions.at(triangle.x);
		const auto b = m_positions.at(triangle.y);
		const auto c = m_positions.at(triangle.z);
		
		const auto n = glm::cross(c - a, b - a);
		//const auto n = glm::cross(b - a, c - a);

		auto& v1 = normals[triangle.x];
		auto& v2 = normals[triangle.y];
		auto& v3 = normals[triangle.z];

		atomicFloatAdd(v1[0], n.x);
		atomicFloatAdd(v1[1], n.y);
		atomicFloatAdd(v1[2], n.z);

		atomicFloatAdd(v2[0], n.x);
		atomicFloatAdd(v2[1], n.y);
		atomicFloatAdd(v2[2], n.z);

		atomicFloatAdd(v3[0], n.x);
		atomicFloatAdd(v3[1], n.y);
		atomicFloatAdd(v3[2], n.z);
	});

	m_normals.resize(m_verticesCount);

	std::transform(TRY_PARALLELIZE_PAR_UNSEQ normals.cbegin(), normals.cend(), m_normals.begin(), [](const atomic_vec3_t& n)
	{
		return glm::normalize(glm::vec3(n[0].load().flt, n[1].load().flt, n[2].load().flt));
	});
}

}