#include <rasterizer/linear_rgba_t.hpp>
#include <rasterizer/gamma_bgra_t.hpp>

#include "LookUpTable.hpp"

namespace rasterizer {

static detail::LookUpTable<float> toFloatTable{ [](float& v)
{
	v = v / 255.0f;
} };

static detail::LookUpTable<uint8_t> toGammaTable{ [](uint8_t& v)
{
	float value = float(v) / 255.0f;
	value = std::pow(value, 1.0f / 2.2f);
	v = uint8_t(std::round(value * 255.0f));
} };

template<> float gamma_bgra_t::red() const noexcept { return toFloatTable[r]; };
template<> float gamma_bgra_t::green() const noexcept { return toFloatTable[g]; };
template<> float gamma_bgra_t::blue() const noexcept { return toFloatTable[b]; };
template<> float gamma_bgra_t::alpha() const noexcept { return toFloatTable[a]; };

template<> uint8_t gamma_bgra_t::red() const noexcept { return r; };
template<> uint8_t gamma_bgra_t::green() const noexcept { return g; };
template<> uint8_t gamma_bgra_t::blue() const noexcept { return b; };
template<> uint8_t gamma_bgra_t::alpha() const noexcept { return a; };

gamma_bgra_t gamma_bgra_t::from(const linear_rgba_t& linear)
{
	return
	{
		toGammaTable[linear.blue()],
		toGammaTable[linear.green()],
		toGammaTable[linear.red()],
		toGammaTable[linear.alpha()],
	};
}

}