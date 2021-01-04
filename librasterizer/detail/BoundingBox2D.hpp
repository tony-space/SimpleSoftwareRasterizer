#pragma once

#include <limits>

#include "glm-include.hpp"

namespace rasterizer {

class BoundingBox2D
{
public:
	BoundingBox2D() = default;
	BoundingBox2D(const BoundingBox2D&) noexcept = default;
	BoundingBox2D(BoundingBox2D&&) noexcept = default;
	~BoundingBox2D() = default;

	BoundingBox2D(std::initializer_list<glm::vec2> points) noexcept;

	glm::vec2 min() const noexcept;
	glm::vec2 max() const noexcept;
	glm::vec2 center() const noexcept;
	glm::vec2 size() const noexcept;
	
	void add(const glm::vec2& point) noexcept;
	//void add(const BoundingBox2D& box) noexcept;

	bool inside(const glm::vec2& point) const noexcept;
	bool overlaps(const BoundingBox2D& other) const noexcept;

private:
	glm::vec2 m_min = glm::vec2(std::numeric_limits<float>::infinity());
	glm::vec2 m_max = glm::vec2(-std::numeric_limits<float>::infinity());
};

inline BoundingBox2D::BoundingBox2D(std::initializer_list<glm::vec2> points) noexcept
{
	for (const auto& p : points)
		add(p);
}

inline glm::vec2 BoundingBox2D::min() const noexcept
{
	return m_min;
}

inline glm::vec2 BoundingBox2D::max() const noexcept
{
	return m_max;
}

inline glm::vec2 BoundingBox2D::center() const noexcept
{
	return (m_max + m_min) * 0.5f;
}

inline glm::vec2 BoundingBox2D::size() const noexcept
{
	return m_max - m_min;
}

inline void BoundingBox2D::add(const glm::vec2& point) noexcept
{
	m_min = glm::min(m_min, point);
	m_max = glm::max(m_max, point);
}

//inline void BoundingBox2D::add(const BoundingBox2D& box) noexcept
//{
//	m_min = glm::min(m_min, box.min());
//	m_max = glm::max(m_max, box.max());
//}

inline bool BoundingBox2D::inside(const glm::vec2& point) const noexcept
{
	auto outside = false;

	outside = outside || point.x > m_max.x;
	outside = outside || point.y > m_max.y;
	outside = outside || point.x < m_min.x;
	outside = outside || point.y < m_min.y;

	return !outside;
}

inline bool BoundingBox2D::overlaps(const BoundingBox2D& other) const noexcept
{
	const auto center1 = center();
	const auto center2 = other.center();

	auto delta = glm::abs(center1 - center2) * 2.0f;
	auto totalSize = size() + other.size();

	return (delta.x < totalSize.x) && (delta.y < totalSize.y);
}

}