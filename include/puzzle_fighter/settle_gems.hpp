// SPDX-License-Identifier: GPL-3.0-only
// USA 960620: 00CEC4..00D01A plus out-of-line 00D0E8..00D183.
#pragma once
#include <cstdint>
#include <initializer_list>

namespace puzzle_fighter {
struct SettleResult {
    unsigned ordinary_moves{}, wall_events{}, big_moves{}, big_blocked{};
    bool sound_requested{}, restarted{}, diamond_scanned{};
};

// Memory is the game's byte/word storage, not an instruction interpreter.
// Effects supplies landing_sound(player) and find_diamond(player, memory).
// With the normal global layout and aligned in-range sound queue, landing
// sound preserves gameplay state. Its platform/audio implementation is an
// explicit dependency, not a silent emulation fallback. Malformed queue
// aliases are outside the event-only validation contract.
template<class Memory, class Effects>
SettleResult settle_gems(std::uint32_t player, Memory &m, Effects &effects) {
    SettleResult result;
    const auto signed_byte = [](std::uint8_t v) { return v < 128 ? int(v) : int(v) - 256; };
    const auto note_moved = [&]() {
        for (auto offset : {0x174u, 0x171u, 0x177u, 0x178u, 0x18cu})
            m.write_byte(player + offset, 1);
    };
    m.write_byte(player + 0x1e4u, 0);
    (void)m.read_word(player + 0x372u); // Original scratch-register reads.
    (void)m.read_byte(player + 0x32du);
    auto most = m.read_byte(player + 0x3a0u);
    for (auto offset : {0x3a1u, 0x3a2u, 0x3a3u}) {
        if (signed_byte(most) < signed_byte(m.read_byte(player + offset)))
            most = m.read_byte(player + offset);
    }
    if (signed_byte(most) > signed_byte(m.read_byte(player + 0x32fu)))
        m.write_byte(player + 0x32fu, most);
    m.write_byte(player + 0x32du, 0);
    m.write_word(player + 0x3a0u, 0);
    m.write_word(player + 0x3a2u, 0);
    const auto high = m.read_word(player + 0x164u);
    const auto field = (std::uint32_t(high) << 16) | m.read_word(player + 0x166u);
    bool fell = false;
    for (unsigned column = 0; column < 6; ++column) {
        auto cell = field + 0xc2u + 2u * column;
        bool solid_below = true;
        for (unsigned row = 0; row < 14; ++row, cell -= 0x10u) {
            if (m.read_word(cell) == 0) { solid_below = false; continue; }
            if (m.read_word(cell) == 0xffu) {
                ++result.wall_events;
            } else {
                if (solid_below) continue;
                if ((m.read_word(cell) & 0xfu) != 7u && m.read_byte(cell) != 0) {
                    auto gem = m.read_byte(cell + 0x400u);
                    m.write_byte(player + 0x240u, m.read_byte(cell + 0x3feu));
                    bool can_move = gem != m.read_byte(player + 0x240u);
                    std::uint32_t width_minus_one = 0;
                    if (can_move) {
                        auto at = cell;
                        do {
                            at += 2u;
                            ++width_minus_one;
                            m.write_byte(player + 0x240u, m.read_byte(at + 0x400u));
                        } while (gem == m.read_byte(player + 0x240u));
                        --width_minus_one;
                        can_move = width_minus_one != 0;
                    }
                    if (can_move) {
                        // DBRA uses the low word; preserve that original count.
                        const auto width = (width_minus_one & 0xffffu) + 1u;
                        gem = m.read_byte(cell + 0x400u);
                        for (std::uint32_t n = 0; n < width; ++n) {
                            if (m.read_word(cell + 0x10u + 2u*n) != 0) {
                                can_move = false; break;
                            }
                        }
                        if (can_move) {
                            gem = m.read_byte(cell + 0x400u);
                            for (std::uint32_t n = 0; n < width; ++n) {
                                auto at = cell + 2u*n;
                                do {
                                    m.write_word(at + 0x10u, m.read_word(at));
                                    m.write_word(at, 0);
                                    m.write_word(at + 0x410u, m.read_word(at + 0x400u));
                                    m.write_word(at + 0x400u, 0);
                                    at -= 0x10u;
                                    m.write_byte(player + 0x240u, m.read_byte(at + 0x400u));
                                } while (gem == m.read_byte(player + 0x240u));
                            }
                            note_moved(); fell = true; ++result.big_moves;
                        }
                    }
                    if (!can_move) ++result.big_blocked;
                }
                const auto value = m.read_word(cell);
                if (value & 0x80u) { solid_below = true; continue; }
                m.write_word(cell + 0x10u, value);
                m.write_word(cell, 0);
                m.write_word(cell + 0x410u, m.read_word(cell + 0x400u));
                m.write_word(cell + 0x400u, 0);
                ++result.ordinary_moves;
            }
            note_moved();
            m.write_byte(player + 0x371u, 1);
            m.write_byte(player + 0x37cu, 0);
            fell = true;
            break;
        }
    }
    if (!fell) {
        m.write_byte(player + 0x371u, m.read_byte(player + 0x371u) ? 2 : 0);
        m.write_byte(player + 0x17eu, static_cast<std::uint8_t>(m.read_byte(player + 0x17eu) + 1u));
        if (m.read_byte(player + 0x371u) == 2) {
            effects.landing_sound(player); result.sound_requested = true;
            m.write_byte(player + 0x371u, 0);
        }
        if (m.read_byte(player + 0x178u)) {
            m.write_byte(player + 0x178u, 0);
            m.write_byte(player + 0x174u, 0);
            m.write_byte(player + 0x171u, 1);
            m.write_byte(player + 0x17eu, 1);
            m.write_byte(player + 0x275u, 1);
            result.restarted = true;
            return result;
        }
    }
    effects.find_diamond(player, m); result.diamond_scanned = true;
    if (m.read_byte(player + 0x275u)) {
        m.write_byte(player + 0x275u, 0);
        m.write_byte(player + 0x266u, 1);
    }
    return result;
}
} // namespace puzzle_fighter
