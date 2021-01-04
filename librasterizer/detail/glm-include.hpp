#pragma once

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4201) //glm\ext\../detail/type_vec3.hpp(67,1): warning C4201: nonstandard extension used: nameless struct/union
#endif // _MSC_VER

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/reciprocal.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/compatibility.hpp>

#ifdef _MSC_VER
#pragma warning (pop)
#endif // _MSC_VER