#pragma once
#include <cstdint>

namespace rasterizer {

struct linear_rgba_t;

struct gamma_bgra_t
{
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;

	template<typename T = uint8_t> T red() const noexcept;
	template<typename T = uint8_t> T green() const noexcept;
	template<typename T = uint8_t> T blue() const noexcept;
	template<typename T = uint8_t> T alpha() const noexcept;

	static gamma_bgra_t from(const linear_rgba_t& linear);
};

template<> float gamma_bgra_t::red() const noexcept;
template<> float gamma_bgra_t::green() const noexcept;
template<> float gamma_bgra_t::blue() const noexcept;
template<> float gamma_bgra_t::alpha() const noexcept;

template<> uint8_t gamma_bgra_t::red() const noexcept;
template<> uint8_t gamma_bgra_t::green() const noexcept;
template<> uint8_t gamma_bgra_t::blue() const noexcept;
template<> uint8_t gamma_bgra_t::alpha() const noexcept;

}