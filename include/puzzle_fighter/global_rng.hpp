// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 global RNG at 0x02DB6A; native arithmetic, no CPU emulation.
#pragma once
#include <cstdint>

namespace puzzle_fighter {
struct GlobalRandomResult {
    std::uint16_t seed;
    std::uint8_t value; // D0 low word becomes this zero-extended byte.
    std::uint8_t ccr;
};
constexpr GlobalRandomResult global_random(std::uint16_t stored_seed) {
    // Unlike the player RNG, zero is not replaced with a nonzero seed.
    const auto triple = static_cast<std::uint16_t>(3u * stored_seed);
    const auto high = static_cast<std::uint8_t>(triple >> 8);
    const unsigned sum = (stored_seed & 0xffu) + high;
    const auto value = static_cast<std::uint8_t>(sum);
    const auto seed = static_cast<std::uint16_t>((unsigned(high) << 8) | value);
    // The last MOVE.B sets N/Z from the returned byte and clears V/C;
    // X survives from the preceding ADD.B into the seed's low byte.
    const auto ccr = static_cast<std::uint8_t>((sum > 255u ? 0x10u : 0u) |
        (value & 0x80u ? 0x08u : 0u) | (value == 0 ? 0x04u : 0u));
    return {seed, value, ccr};
}
} // namespace puzzle_fighter
