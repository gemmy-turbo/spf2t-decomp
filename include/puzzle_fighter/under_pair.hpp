// SPDX-License-Identifier: GPL-3.0-only
// USA 960620, 0x011948: sum the two board words below the falling pair.
#pragma once
#include "pair_cell.hpp"

namespace puzzle_fighter {
// Memory supplies read_word/read_byte/write_word with a 24-bit board address
// boundary. State retains full 32-bit address registers. The internal BSR's
// stack writes remain observable; the outer RTS belongs to the caller harness.
template<class Memory>
PairCellState under_pair(PairCellState state, Memory &memory) {
    const auto player = state.a[6];
    const auto base_high = memory.read_word(player + 0x164u);
    const auto base_low = memory.read_word(player + 0x166u);
    state.a[0] = (std::uint32_t(base_high) << 16) | base_low;
    state.a[0] += 2u;
    auto &sp = (state.sr & 0x2000u) ? state.supervisor_sp : state.user_sp;
    sp -= 4u;
    memory.write_word(sp, 0x0001u);
    memory.write_word(sp + 2u, 0x1952u);
    const auto row = memory.read_word(player + 0x14u);
    const auto column = memory.read_word(player + 0x10u);
    state = pair_cell(state, row, column);
    // The original callee returns through the stack it just wrote.
    memory.read_word(sp);
    memory.read_word(sp + 2u);
    sp += 4u;
    const std::uint16_t first = memory.read_word(state.a[0] + 0x10u);
    state.d[3] = first;
    state.d[0] = std::uint32_t(memory.read_byte(player + 0x170u)) * 2u;
    state.a[1] = 0x12bc8u;
    const auto table_first = memory.read_word(state.a[1] + state.d[0]);
    const auto table_second = memory.read_word(state.a[1] + state.d[0] + 8u);
    const auto offset = static_cast<std::uint16_t>(table_first + table_second);
    state.d[1] = (state.d[1] & 0xffff0000u) | offset;
    const std::uint32_t signed_offset = (offset & 0x8000u)
        ? 0xffff0000u | offset : offset;
    state.a[0] += signed_offset;
    const std::uint16_t second = memory.read_word(state.a[0] + 0x10u);
    const std::uint32_t sum = std::uint32_t(first) + second;
    const auto result = static_cast<std::uint16_t>(sum);
    state.d[3] = result;
    const auto flags = static_cast<std::uint16_t>(
        (sum > 0xffffu ? 0x11u : 0u) |
        (result & 0x8000u ? 0x08u : 0u) |
        (result == 0 ? 0x04u : 0u) |
        ((~(first ^ second) & (first ^ result) & 0x8000u) ? 0x02u : 0u));
    state.sr = static_cast<std::uint16_t>((state.sr & 0xffe0u) | flags);
    return state;
}
} // namespace puzzle_fighter
