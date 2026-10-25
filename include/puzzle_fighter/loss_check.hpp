// SPDX-License-Identifier: GPL-3.0-only
// USA C79E..C7DD. Gameplay state and the exact C948 banner closure.
#pragma once
#include <cstdint>
#include "clear_banner.hpp"
namespace puzzle_fighter {
template<class Memory>
bool loss_check(std::uint32_t player,Memory&m) {
    if(m.read_byte(player+0x176u))return false;
    const auto high=m.read_word(player+0x164u);
    const auto field=(std::uint32_t(high)<<16)|m.read_word(player+0x166u);
    if(!m.read_word(field+8u))return false;
    m.write_byte(player+0x17eu,0x14);
    m.write_byte(player+0x1a0u,0);
    m.write_byte(player+0x354u,0x0e);
    m.write_byte(player+0x328u,0);
    m.write_word(player+0x14u,0);
    m.write_word(player+0x152u,0);
    m.write_word(player+0x154u,0);
    clear_banner(player,m);
    return true;
}
} // namespace puzzle_fighter
