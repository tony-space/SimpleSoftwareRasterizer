#pragma once

#include <filesystem>
#include <rasterizer/Mesh.hpp>

namespace rasterizer {
namespace loaders {

Mesh loadObj(const std::filesystem::path& path);

}
}