#include <rasterizer/Mesh.hpp>


namespace rasterizer {

static constexpr float kRadius = 1.0f;
static constexpr size_t kStackCount = 24;
static constexpr size_t kSliceCount = 24;
static constexpr float kPI = glm::pi<float>();
static std::once_flag meshCallOnceFlag;

const Mesh& Mesh::sphere()
{
	static Mesh sphere{ 0, 0 };

	std::call_once(meshCallOnceFlag, [](Mesh& sphere)
	{
		//http://www.richardssoftware.net/2013/07/shapes-demo-with-direct3d11-and-slimdx.html
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texCoords;

		positions.emplace_back(0.0f, kRadius, 0.0f);
		normals.emplace_back(0.0f, 1.0f, 0.0f);
		texCoords.emplace_back(0.0f, 1.0f);

		auto phiStep = kPI / kStackCount;
		auto thetaStep = 2.0f * kPI / kSliceCount;

		for (size_t i = 1; i <= kStackCount - 1; i++)
		{
			auto phi = i * phiStep;
			for (size_t j = 0; j <= kSliceCount; j++)
			{
				auto theta = j * thetaStep;
				auto p = glm::vec3(
					(kRadius * std::sin(phi) * std::cos(theta)),
					(kRadius * std::cos(phi)),
					(kRadius * std::sin(phi) * std::sin(theta)));

				auto n = glm::normalize(p);

				auto uv = glm::vec2(theta / (kPI * 2.0f), 1.0f - phi / kPI);
				positions.emplace_back(p);
				normals.emplace_back(n);
				texCoords.emplace_back(uv);
			}
		}
		positions.emplace_back(0.0f, -kRadius, 0.0f);
		normals.emplace_back(0.0f, -1.0f, 0.0f);
		texCoords.emplace_back(0.0f, 0.0f);

		std::vector<glm::u16vec3> triangles;
		for (uint16_t i = 1; i <= kSliceCount; i++)
		{
			triangles.emplace_back(glm::u16vec3{ 0, i, i + 1 });
		}
		auto baseIndex = uint16_t(1);
		auto ringVertexCount = uint16_t(kSliceCount + 1);
		for (uint16_t i = 0; i < kStackCount - 2; i++)
		{
			for (uint16_t j = 0; j < kSliceCount; j++)
			{
				triangles.emplace_back(glm::u16vec3{
					baseIndex + i * ringVertexCount + j,
					baseIndex + (i + uint16_t(1)) * ringVertexCount + j,
					baseIndex + i * ringVertexCount + j + 1 });

				triangles.emplace_back(glm::u16vec3{
					baseIndex + (i + uint16_t(1)) * ringVertexCount + j,
					baseIndex + (i + uint16_t(1)) * ringVertexCount + j + 1,
					baseIndex + i * ringVertexCount + j + 1 });
			}
		}
		auto southPoleIndex = uint16_t(positions.size() - 1);
		baseIndex = southPoleIndex - ringVertexCount;
		for (uint16_t i = 0; i < kSliceCount; i++)
		{
			triangles.emplace_back(glm::u16vec3{
				southPoleIndex,
				baseIndex + i + 1,
				baseIndex + i });
		}

		Mesh newSphere{ unsigned(positions.size()), unsigned(triangles.size()) };
		newSphere.setPositions(std::move(positions));
		newSphere.setNormals(std::move(normals));
		newSphere.setTexCoords0(std::move(texCoords));
		newSphere.setTriangles(std::move(triangles));

		sphere = std::move(newSphere);

	}, std::ref(sphere));

	return sphere;
}

}