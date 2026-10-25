// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 FCC6 entry branches to FD24; ends at FD5E or FD62 RTS.
// Original instruction-derived word arithmetic; validation pending.
#pragma once
#include <cstdint>

namespace puzzle_fighter {
constexpr std::uint16_t cancellation_scale(std::uint16_t input) {
    // CMP.W #12 followed by signed BLE includes every negative input word.
    // Express ASR.W explicitly, without implementation-defined signed shifts.
    if ((input & 0x8000u) || input <= 12u)
        return static_cast<std::uint16_t>((input >> 1) | (input & 0x8000u));

    const unsigned additions = input <= 18u ? 5u : input <= 24u ? 6u : input <= 30u ? 7u : 8u;
    std::uint16_t groups = 0;
    auto remaining = input;
    do {
        ++groups;
        remaining = static_cast<std::uint16_t>(remaining - 8u);
    } while (!(remaining & 0x8000u));
    // FD4A counts before subtraction: exact multiples include one extra group.
    // D3 starts at 4..7; ADD.W then DBRA performs 5..8 word additions.
    std::uint16_t output = 0;
    for (unsigned i = 0; i < additions; ++i)
        output = static_cast<std::uint16_t>(output + groups);
    return output;
}
} // namespace puzzle_fighter
