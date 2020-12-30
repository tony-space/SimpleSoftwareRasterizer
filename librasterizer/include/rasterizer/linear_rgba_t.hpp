#pragma once
#include <cstdint>

namespace rasterizer {

struct gamma_bgra_t;

struct linear_rgba_t
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

	template<typename T = uint8_t> T red() const noexcept;
	template<typename T = uint8_t> T green() const noexcept;
	template<typename T = uint8_t> T blue() const noexcept;
	template<typename T = uint8_t> T alpha() const noexcept;

	static linear_rgba_t from(const gamma_bgra_t& gamma);
};

template<> float linear_rgba_t::red() const noexcept;
template<> float linear_rgba_t::green() const noexcept;
template<> float linear_rgba_t::blue() const noexcept;
template<> float linear_rgba_t::alpha() const noexcept;

template<> uint8_t linear_rgba_t::red() const noexcept;
template<> uint8_t linear_rgba_t::green() const noexcept;
template<> uint8_t linear_rgba_t::blue() const noexcept;
template<> uint8_t linear_rgba_t::alpha() const noexcept;

}