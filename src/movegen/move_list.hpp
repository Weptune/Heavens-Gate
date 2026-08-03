#pragma once

#include "../core/types.hpp"
#include <array>
#include <cstddef>
#include <cassert>

namespace heavensgate {

class MoveList {
private:
    std::array<Move, 256> moves_{};
    size_t size_{0};

public:
    constexpr MoveList() noexcept = default;

    constexpr void push_back(Move m) noexcept {
        assert(size_ < 256);
        moves_[size_++] = m;
    }

    constexpr size_t size() const noexcept { return size_; }
    constexpr bool empty() const noexcept { return size_ == 0; }
    constexpr void clear() noexcept { size_ = 0; }

    constexpr Move operator[](size_t index) const noexcept {
        assert(index < size_);
        return moves_[index];
    }
    constexpr Move& operator[](size_t index) noexcept {
        assert(index < size_);
        return moves_[index];
    }

    constexpr const Move* begin() const noexcept { return moves_.data(); }
    constexpr const Move* end() const noexcept { return moves_.data() + size_; }
    constexpr Move* begin() noexcept { return moves_.data(); }
    constexpr Move* end() noexcept { return moves_.data() + size_; }
};

} // namespace heavensgate
