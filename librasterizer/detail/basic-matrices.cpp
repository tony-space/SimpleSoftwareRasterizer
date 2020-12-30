#include "basic-matrices.hpp"

namespace rasterizer {
namespace matrices {

glm::mat4 viewportTransformMatrix(float width, float height)
{
	const auto halfWidth = width * 0.5f;
	const auto halfHeight = height * 0.5f;
	return
	{
		halfWidth, 0.0f, 0.0f, 0.0f,
		0.0f, halfHeight, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		halfWidth, halfHeight, 0.0f, 1.0f
	};
}

glm::mat4 viewMatrix(const glm::vec3& eulerAnglesDeg, const glm::vec3& cameraPos)
{
	auto result = glm::identity<glm::mat4>();
	result = glm::translate(result, cameraPos);
	result = glm::rotate(result, glm::radians(eulerAnglesDeg.x), glm::vec3(1.0f, 0.0f, 0.0f));
	result = glm::rotate(result, glm::radians(eulerAnglesDeg.y), glm::vec3(0.0f, 1.0f, 0.0f));
	result = glm::rotate(result, glm::radians(eulerAnglesDeg.z), glm::vec3(0.0f, 0.0f, 1.0f));

	return result;
}

glm::mat4 projectionMatrix(float width, float height, float verticalFovDeg, float zNear, float zFar)
{
	const auto yScale = glm::cot(glm::radians(verticalFovDeg * 0.5f));
	const auto xScale = yScale / (width / height);
	const auto A = zFar / (zFar - zNear);
	const auto B = -zNear * A;

	return
	{
		xScale, 0.0f, 0.0f, 0.0f,
		0.0f, yScale, 0.0f, 0.0f,
		0.0f, 0.0f, A, 1.0f,
		0.0f, 0.0f, B, 0.0f
	};
}

}
}
