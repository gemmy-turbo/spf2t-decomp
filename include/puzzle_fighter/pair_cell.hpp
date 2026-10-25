// SPDX-License-Identifier: GPL-3.0-only
// USA 960620, 0x011916: address calculation for the falling pair.
// Recovered from MAME instruction/call-boundary evidence. The European
// cps2recomp PairCellRoutine was a search lead, not the behavioral oracle.
#pragma once
#include <array>
#include <cstdint>

namespace puzzle_fighter {
struct PairCellState {
    std::array<std::uint32_t, 8> d{};
    std::array<std::uint32_t, 7> a{};
    std::uint32_t supervisor_sp{}, user_sp{};
    std::uint16_t sr{};
};

// Produces the state immediately before RTS. Memory is read only through the
// two explicit word arguments; no memory writes or runtime hooks are enabled.
inline PairCellState pair_cell(PairCellState state, std::uint16_t row,
                              std::uint16_t column) {
    const auto decremented_row = static_cast<std::uint16_t>(row - 1u);
    const auto decremented_column = static_cast<std::uint16_t>(column - 1u);
    const std::uint32_t rows = (decremented_row & 0x8000u) ? 0u : decremented_row + 1u;
    const std::uint32_t columns = (decremented_column & 0x8000u) ? 0u : decremented_column + 1u;
    state.a[0] += rows * 16u + columns * 2u - 32u;
    state.a[1] += rows * 4u + columns * 64u - 12u;
    state.d[1] = (state.d[1] & 0xffff0000u) | (rows ? 0xffffu : decremented_row);
    state.d[0] = (state.d[0] & 0xffff0000u) | (columns ? 0xffffu : decremented_column);
    // LEA and DBRA preserve CCR. The final SUBQ.W on column determines all
    // five condition bits, including extend and signed overflow.
    const auto flags = static_cast<std::uint16_t>(
        (column == 0 ? 0x11u : 0u) |
        (decremented_column & 0x8000u ? 0x08u : 0u) |
        (decremented_column == 0 ? 0x04u : 0u) |
        (column == 0x8000u ? 0x02u : 0u));
    state.sr = static_cast<std::uint16_t>((state.sr & 0xffe0u) | flags);
    return state;
}
} // namespace puzzle_fighter
