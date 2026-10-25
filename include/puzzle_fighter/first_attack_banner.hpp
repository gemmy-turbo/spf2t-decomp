// SPDX-License-Identifier: GPL-3.0-only
// USA C8E0..C8F7 and C9BE..CA37; original opponent banner and tile clear.
#pragma once
#include <cstdint>
#include <initializer_list>
namespace puzzle_fighter {
template<class Memory>
void first_attack_banner(std::uint32_t player,Memory&m) {
    const auto high=m.read_word(player+0x160u);
    const auto opponent=(std::uint32_t(high)<<16)|m.read_word(player+0x162u);
    m.write_byte(opponent+0x38du,3);
    m.write_byte(opponent+0x381u,0x3c);
    // C9BE reloads the pointer after the flag writes. Keep that alias behavior.
    const auto current_high=m.read_word(player+0x160u);
    const auto current=(std::uint32_t(current_high)<<16)|m.read_word(player+0x162u);
    for(const auto offset:{0xfffffffcu,0xfffffff8u,0u}) {
        const auto tile_high=m.read_word(current+0x382u);
        auto tile=((std::uint32_t(tile_high)<<16)|m.read_word(current+0x384u))+offset;
        for(unsigned row=0;row<12;++row,tile+=0x80u) {
            m.write_word(tile,0x20);m.write_word(tile+2u,0);
            m.write_word(tile+4u,0x20);m.write_word(tile+6u,0);
        }
    }
}
} // namespace puzzle_fighter
