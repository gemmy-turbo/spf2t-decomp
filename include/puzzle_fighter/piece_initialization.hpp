// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 shared piece-stream initialization prefix at 0x005AD2.
#pragma once
#include "global_rng.hpp"
#include <array>
#include <cstdint>

namespace puzzle_fighter {
struct PieceStreamInitialization {
    std::uint16_t global_seed;
    std::uint16_t player_seed; // Identical initial seed copied to BOTH players.
    std::array<std::uint16_t, 2> next;
    std::uint8_t order;
    std::array<std::uint8_t, 4> colours;
};

// Stops at the instruction boundary 0x005B22, after helper 0x005B30 returns.
// The next pair, order and four raw colour-counter bytes are shared by both
// players. Later player setup, diamond scheduling and round setup are excluded.
constexpr PieceStreamInitialization initialize_piece_streams(std::uint16_t global_seed) {
    PieceStreamInitialization result{global_seed, global_seed, {}, 0, {}};
    const auto draw = [&result]() {
        const auto random = global_random(result.global_seed);
        result.global_seed = random.seed;
        return random.value;
    };
    result.next[0] = static_cast<std::uint16_t>((draw() & 3u) + 1u);
    result.next[1] = static_cast<std::uint16_t>((draw() & 3u) + 1u);
    result.order = static_cast<std::uint8_t>(draw() & 7u);
    for (auto &colour : result.colours)
        colour = static_cast<std::uint8_t>(draw() & 3u);
    return result;
}
} // namespace puzzle_fighter
