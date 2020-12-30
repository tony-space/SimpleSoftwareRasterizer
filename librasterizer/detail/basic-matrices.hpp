#pragma once

#include "glm-include.hpp"

namespace rasterizer {
namespace matrices {

glm::mat4 viewportTransformMatrix(float width, float height);
glm::mat4 viewMatrix(const glm::vec3& eulerAnglesDeg, const glm::vec3& cameraPos);
glm::mat4 projectionMatrix(float width, float height, float verticalFovDeg, float zNear, float zFar);

}
}