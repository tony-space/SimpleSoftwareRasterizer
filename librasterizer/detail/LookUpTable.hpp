#pragma once

#include <array>
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <utility>

namespace rasterizer {
namespace detail {

template<typename ValueType>
class LookUpTable
{
public:
    template<typename TFunc>
    explicit LookUpTable(TFunc&& func)
    {
        std::iota(m_table.begin(), m_table.end(), ValueType(0));
        std::for_each(m_table.begin(), m_table.end(), std::forward<TFunc>(func));
    }
    LookUpTable(const LookUpTable&) = delete;
    LookUpTable(LookUpTable&&) = delete;
    LookUpTable& operator= (const LookUpTable&) = delete;
    LookUpTable& operator= (LookUpTable&&) = delete;

    const ValueType& operator[](size_t idx) const noexcept
    {
        return m_table[idx];
    }
private:
    std::array<ValueType, 0x100> m_table;
};

}
}