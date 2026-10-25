// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 11C80..11D05. Instruction-derived candidate; validation pending.
#pragma once
#include "pair_cell.hpp"
namespace puzzle_fighter {
// A newly pressed turn button swaps a pair entering from above, or trapped
// between two occupied cells. Original repeated input reads remain observable.
template<class Memory>
bool swap_pair(std::uint32_t player,Memory&m) {
    const auto previous=m.read_word(player+0x142u);
    const auto current=m.read_word(player+0x140u);
    if(!((~previous&current)&0x100u)) {
        const auto previous_again=m.read_word(player+0x142u);
        const auto current_again=m.read_word(player+0x140u);
        if(!((~previous_again&current_again)&0x200u))return false;
    }
    if(m.read_byte(player+0x176u)==0x10u)return false;
    bool entering=false;
    if(m.read_word(player+0x14u)==2u)entering=(m.read_word(player+0x242u)&0x8000u)!=0;
    if(!entering) {
        const auto high=m.read_word(player+0x164u);
        PairCellState state;
        state.a[0]=((std::uint32_t(high)<<16)|m.read_word(player+0x166u))+2u;
        const auto row=m.read_word(player+0x14u);
        const auto column=m.read_word(player+0x10u);
        const auto cell=pair_cell(state,row,column).a[0];
        if(!m.read_word(cell+2u))return false;
        if(!m.read_word(cell-2u))return false;
    }
    const auto first=m.read_word(player+0x152u);
    m.write_word(player+0x152u,m.read_word(player+0x154u));
    m.write_word(player+0x154u,first);
    m.write_word(player+0x248u,0);m.write_word(player+0x24au,0);
    m.write_word(player+0x24cu,8);m.write_byte(player+0x247u,2);
    return true;
}
} // namespace puzzle_fighter
