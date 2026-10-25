// SPDX-License-Identifier: GPL-3.0-only
// USA 960620, 00D3D6..00D461. Gameplay memory only; audio is an explicit effect.
#pragma once
#include <cstdint>

namespace puzzle_fighter {
enum class BreakSoundKind { normal, power };
struct StartBreaksResult {
    std::uint32_t visited{}, counter_transitions{}, normal_requests{}, power_requests{};
};

// Memory supplies ordered big-endian byte/word operations and 24-bit bus mapping.
// Effects supplies break_sound(player, kind); this requests the original helper,
// without claiming that every request emits audio (the helpers have global gates).
template<class Memory, class Effects>
StartBreaksResult start_breaks(std::uint32_t player, Memory& m, Effects& effects) {
    StartBreaksResult result;
    const auto high = m.read_word(player + 0x164u);
    auto column = (std::uint32_t(high) << 16) | m.read_word(player + 0x166u);
    column += 0xc2u;
    m.write_word(player + 0x276u, 5);
    std::uint16_t remaining_columns;
    do {
        auto cell = column;
        m.write_word(player + 0x278u, 13);
        std::uint16_t remaining_rows;
        do {
            ++result.visited;
            const auto value = m.read_word(cell);
            if (value != 0) {
                if ((value & 15u) == 7u) {
                    if (m.read_word(cell) & 0x20u) {
                        m.write_byte(cell + 1u, m.read_byte(cell + 1u) & 0xdfu);
                        m.write_byte(cell + 1u, m.read_byte(cell + 1u) | 0x40u);
                        ++result.counter_transitions;
                    }
                } else if (m.read_word(cell) & 0x20u) {
                    // D426 reaches D42A whether bit 4 was already set or clear.
                    m.write_word(cell, m.read_word(cell) | 0x10u);
                    const auto kind = (m.read_byte(cell) & 0xf0u) == 0x30u
                        ? BreakSoundKind::power : BreakSoundKind::normal;
                    effects.break_sound(player, kind);
                    if (kind == BreakSoundKind::power) ++result.power_requests;
                    else ++result.normal_requests;
                }
            }
            cell -= 0x10u;
            remaining_rows = static_cast<std::uint16_t>(m.read_word(player + 0x278u) - 1u);
            m.write_word(player + 0x278u, remaining_rows);
        } while ((remaining_rows & 0x8000u) == 0);
        column += 2u;
        remaining_columns = static_cast<std::uint16_t>(m.read_word(player + 0x276u) - 1u);
        m.write_word(player + 0x276u, remaining_columns);
    } while ((remaining_columns & 0x8000u) == 0);
    return result;
}
} // namespace puzzle_fighter
