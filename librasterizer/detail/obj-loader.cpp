#include <stdexcept>
#include <string>
#include <vector>

#include "glm-include.hpp"
#include "BoundingBox2D.hpp"

#include <rasterizer/obj-loader.hpp>

namespace rasterizer {
namespace loaders {

template <typename TFunc>
static void readRows(std::ifstream& stream, TFunc&& func)
{
	auto line = std::string();
	for (size_t row = 0;; row++)
	{
		std::getline(stream, line);
		if (stream.eof() && stream.fail())
			break;
		if (stream.fail())
			throw std::invalid_argument("unexpected failure while reading row #" + std::to_string(row));

		func(line);
	}
}

Mesh loadObj(const std::filesystem::path& path)
{
	std::ifstream file(path);

	if (file.fail())
	{
		throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
	}
	static const auto commentRegex = std::regex("\\s*#.*");
	static const auto emptyString = std::regex("\\s*");
	static const auto vertexRegex = std::regex("\\s*v (-?[\\d.]+(?:e-?\\d+)?) (-?[\\d.]+(?:e-?\\d+)?) (-?[\\d.]+(?:e-?\\d+)?)");
	static const auto normalRegex = std::regex("\\s*vn (-?[\\d.]+(?:e-?\\d+)?) (-?[\\d.]+(?:e-?\\d+)?) (-?[\\d.]+(?:e-?\\d+)?)");
	static const auto triangleRegex = std::regex("\\s*f ((\\d*)\\/(\\d*)\\/(\\d*)) ((\\d*)\\/(\\d*)\\/(\\d*)) ((\\d*)\\/(\\d*)\\/(\\d*))");

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::u16vec3> triangles;

	readRows(file, [&](const std::string& row)
	{
		auto match = std::smatch();

		if (std::regex_match(row, match, commentRegex))
		{
			return;
		}
		else if (std::regex_match(row, match, emptyString))
		{
			return;
		}
		else if (std::regex_match(row, match, vertexRegex))
		{
			positions.emplace_back(glm::vec3{
				std::stof(match[1]),
				std::stof(match[2]),
				-std::stof(match[3]) });
		}
		else if (std::regex_match(row, match, normalRegex))
		{
			normals.emplace_back(glm::vec3{
				std::stof(match[1]),
				std::stof(match[2]),
				-std::stof(match[3]) });
		}
		else if (std::regex_match(row, match, triangleRegex))
		{
			triangles.emplace_back(glm::u16vec3{
				std::stoi(match[2]) - 1,
				std::stoi(match[6]) - 1,
				std::stoi(match[10]) - 1,
				});
		}
		else
		{
			throw std::runtime_error("unknown token: " + row);
		}
	});

	if (texCoords.empty())
	{
		BoundingBox2D box;
		for (const auto& pos : positions)
			box.add(pos.xy());

		texCoords.resize(positions.size());

		std::transform(positions.cbegin(), positions.cend(), texCoords.begin(), [&](const glm::vec3& pos)
		{
			return (pos.xy() - box.min()) / box.size();
		});
	}

	Mesh result{ unsigned(positions.size()), unsigned(triangles.size()) };

	result.setPositions(std::move(positions));
	result.setNormals(std::move(normals));
	result.setTexCoords0(std::move(texCoords));
	result.setTriangles(std::move(triangles));

	return result;
}

}
}