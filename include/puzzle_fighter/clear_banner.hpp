// SPDX-License-Identifier: GPL-3.0-only
// USA C948..C9BD: exact three-strip tile clearing. Validation via callers.
#pragma once
#include <cstdint>
#include <initializer_list>
namespace puzzle_fighter {
template<class Memory>
void clear_banner(std::uint32_t player,Memory&m) {
    for(const auto offset:{0xfffffffcu,0xfffffff8u,0u}) {
        const auto high=m.read_word(player+0x382u);
        auto tile=((std::uint32_t(high)<<16)|m.read_word(player+0x384u))+offset;
        for(unsigned row=0;row<12;++row,tile+=0x80u) {
            m.write_word(tile,0x20);m.write_word(tile+2u,0);
            m.write_word(tile+4u,0x20);m.write_word(tile+6u,0);
        }
    }
}
} // namespace puzzle_fighter
