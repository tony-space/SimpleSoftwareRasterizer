#include <stdexcept>
#include <string>
#include <vector>
#include <clocale>

#include "glm-include.hpp"

#include <rasterizer/finally.hpp>
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
	static const auto triangleRegex = std::regex("\\s*f ((\\d+)\\/(\\d*)\\/(\\d*)) ((\\d+)\\/(\\d*)\\/(\\d*)) ((\\d+)\\/(\\d*)\\/(\\d*))");
	static const auto triangleRegex2 = std::regex("\\s*f (\\d+) (\\d+) (\\d+)");
	static const auto groupRegex = std::regex("\\s*g.*");

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::u16vec3> triangles;

	const auto oldLocale = std::setlocale(LC_NUMERIC, nullptr);
	std::setlocale(LC_NUMERIC, "C");
	const auto _ = finally([&]
	{
		std::setlocale(LC_NUMERIC, oldLocale);
	});

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
		else if (std::regex_match(row, match, groupRegex))
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
		else if (std::regex_match(row, match, triangleRegex2))
		{
			triangles.emplace_back(glm::u16vec3{
				std::stoi(match[1]) - 1,
				std::stoi(match[2]) - 1,
				std::stoi(match[3]) - 1,
				});
		}
		else
		{
			throw std::runtime_error("unknown token: " + row);
		}
	});

	constexpr auto kFltMax = std::numeric_limits<float>::max();
	const auto box3D = std::transform_reduce(TRY_PARALLELIZE_PAR_UNSEQ positions.cbegin(), positions.cend(), std::make_pair(glm::vec3(kFltMax), glm::vec3(-kFltMax)),
		[](const std::pair<glm::vec3, glm::vec3>& lhs, const std::pair<glm::vec3, glm::vec3>& rhs)
	{
		return std::make_pair(glm::min(lhs.first, rhs.first), glm::max(lhs.second, rhs.second));
	}, [](const glm::vec3& pos) {
		return std::make_pair(pos, pos);
	});
	const auto boxSize = box3D.second - box3D.first;
	const auto boxCenter = boxSize * 0.5f + box3D.first;

	if (texCoords.empty())
	{
		texCoords.resize(positions.size());
		std::transform(TRY_PARALLELIZE_PAR_UNSEQ positions.cbegin(), positions.cend(), texCoords.begin(), [&](const glm::vec3& pos)
		{
			return (pos.xy() - box3D.first.xy()) / boxSize.xy();
		});
	}

	std::transform(TRY_PARALLELIZE_PAR_UNSEQ positions.cbegin(), positions.cend(), positions.begin(), [&](const glm::vec3& pos)
	{
		return pos - boxCenter;
	});

	Mesh result{ unsigned(positions.size()), unsigned(triangles.size()) };

	result.setPositions(std::move(positions));
	result.setTexCoords0(std::move(texCoords));
	result.setTriangles(std::move(triangles));
	if (normals.empty())
	{
		result.computeNormals();
	}
	else
	{
		result.setNormals(std::move(normals));
	}

	return result;
}

}
}