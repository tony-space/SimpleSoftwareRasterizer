#include <rasterizer/linear_rgba_t.hpp>
#include <rasterizer/gamma_bgra_t.hpp>

#include "LookUpTable.hpp"

namespace rasterizer {

static detail::LookUpTable<float> toFloatTable{ [](float& v)
{
	v = v / 255.0f;
} };

static detail::LookUpTable<uint8_t> fromGammaTable{ [](uint8_t& v)
{
	float value = float(v) / 255.0f;
	value = std::pow(value, 2.2f);
	v = uint8_t(std::round(value * 255.0f));
} };

template<> float linear_rgba_t::red() const noexcept { return toFloatTable[r]; };
template<> float linear_rgba_t::green() const noexcept { return toFloatTable[g]; };
template<> float linear_rgba_t::blue() const noexcept { return toFloatTable[b]; };
template<> float linear_rgba_t::alpha() const noexcept { return toFloatTable[a]; };

template<> uint8_t linear_rgba_t::red() const noexcept { return r; };
template<> uint8_t linear_rgba_t::green() const noexcept { return g; };
template<> uint8_t linear_rgba_t::blue() const noexcept { return b; };
template<> uint8_t linear_rgba_t::alpha() const noexcept { return a; };

linear_rgba_t linear_rgba_t::from(const gamma_bgra_t& gamma)
{
	return
	{
		fromGammaTable[gamma.red()],
		fromGammaTable[gamma.green()],
		fromGammaTable[gamma.blue()],
		fromGammaTable[gamma.alpha()],
	};
}

}