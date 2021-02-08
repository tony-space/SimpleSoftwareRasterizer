#pragma once
#include <utility>

namespace rasterizer {

template <typename TFunc>
[[nodiscard]] auto finally(TFunc&& f)
{
    struct Finalizer
    {
        TFunc f;

        ~Finalizer() noexcept
        {
            f();
        }
    };

    return Finalizer{std::forward<TFunc>(f)};
}

}