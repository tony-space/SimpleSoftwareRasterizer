#include "clipping.hpp"

namespace rasterizer {
namespace clipping {

//A one-dimensional alternative of an isoline/isosurface.
static float isovalue(float first, float second) noexcept
{
	return first / (first - second);
}

std::variant<std::monostate, std::array<glm::vec2, 3>, std::array<glm::vec2, 4>> triangleClip(const glm::vec3& planeDistances) noexcept
{
	constexpr std::array<glm::vec2, 3> kTriangle{ glm::vec2{ 1.0f, 0.0f }, glm::vec2{ 0.0f, 1.0f }, glm::vec2{ 0.0f, 0.0f } };

	int nonNegativeValues = 0;
	int lastNegativeIdx = -1;
	int lastNonNegativeIdx = -1;
	for (int i = 0; i < 3; ++i)
	{
		if (planeDistances[i] >= 0.0f)
		{
			lastNonNegativeIdx = i;
			nonNegativeValues++;
		}
		else
		{
			lastNegativeIdx = i;
		}
	}

	switch (nonNegativeValues)
	{
	case 3:
	{
		//pass the whole triangle as is
		return kTriangle;
	}
	case 0:
	{
		//discard the whole triangle
		return std::monostate{};
	}
	case 1:
	case 2:
	{
		const auto curVertex = nonNegativeValues == 1 ? lastNonNegativeIdx : lastNegativeIdx;
		const auto prevVertex = (curVertex - 1 + 3) % 3;
		const auto nextVertex = (curVertex + 1) % 3;
		
		const auto prevIntersection = isovalue(planeDistances[curVertex], planeDistances[prevVertex]);
		const auto nextIntersection = isovalue(planeDistances[curVertex], planeDistances[nextVertex]);

		const auto& prevBarycentric = kTriangle[prevVertex];
		const auto& curBarycentric = kTriangle[curVertex];
		const auto& nextBarycentric = kTriangle[nextVertex];

		const auto prevIntBarycentric = glm::lerp(curBarycentric, prevBarycentric, prevIntersection);
		const auto nextIntBarycentric = glm::lerp(curBarycentric, nextBarycentric, nextIntersection);

		if (nonNegativeValues == 1)
		{
			return std::array<glm::vec2, 3> {prevIntBarycentric, curBarycentric, nextIntBarycentric};
		}
		else
		{
			return std::array<glm::vec2, 4> {prevBarycentric, prevIntBarycentric, nextIntBarycentric, nextBarycentric};
		}
	}
	}

	std::terminate();
}

}
}