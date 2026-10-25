// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 player RNG at 0x0122EC; no emulator or host random generator.
#pragma once
#include <cstdint>

namespace puzzle_fighter {
struct PlayerRandomResult { std::uint16_t seed; std::uint8_t ccr; };
constexpr PlayerRandomResult player_random(std::uint16_t stored_seed) {
    const std::uint16_t seed = stored_seed ? stored_seed : 0x01c3;
    const auto triple = static_cast<std::uint16_t>(3u * seed);
    const unsigned low_sum = (seed & 0xffu) + (triple >> 8);
    const auto next = static_cast<std::uint16_t>((triple & 0xff00u) | (low_sum & 0xffu));
    // Final MOVE.W sets NZVC; X survives the preceding byte addition.
    const auto ccr = static_cast<std::uint8_t>((low_sum > 255u ? 0x10u : 0u) |
        (next & 0x8000u ? 0x08u : 0u) | (next == 0 ? 0x04u : 0u));
    return {next, ccr};
}
} // namespace puzzle_fighter
