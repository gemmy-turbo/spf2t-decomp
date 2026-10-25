// SPDX-License-Identifier: GPL-3.0-only
// USA 120F8..122EB. Instruction-derived candidate; original validation pending.
#pragma once
#include "pair_cell.hpp"
namespace puzzle_fighter {
struct MovePairResult {unsigned right{},left{},sounds{};};
// Effects.move_sound(player) is the original 2BA46 bounded sound request.
// Movement tests the wrapped sum of the two neighboring words, not two
// independent occupancy booleans. Right runs before left against live state.
template<class Memory,class Effects>
MovePairResult move_pair(std::uint32_t p,Memory&m,Effects&effects) {
    MovePairResult result;
    if(m.read_byte(p+0x397u))return result;
    const auto signed_word=[](std::uint16_t v){return v<0x8000u?int(v):int(v)-0x10000;};
    const auto field=[&](){const auto hi=m.read_word(p+0x164u);return (std::uint32_t(hi)<<16)|m.read_word(p+0x166u);};
    const auto move=[&](bool right) {
        const auto button=right?1u:2u;
        const auto timer=p+(right?0x188u:0x34eu);
        const auto old=m.read_word(p+0x142u);
        const auto pressed=static_cast<std::uint16_t>(~old)&m.read_word(p+0x140u);
        if(pressed&button)m.write_word(timer,0);
        else {
            if(!(m.read_word(p+0x140u)&button))return;
            m.write_byte(p+0x19au,1);
            m.write_word(timer,static_cast<std::uint16_t>(m.read_word(timer)+1u));
            if(signed_word(m.read_word(timer))<12)return;
        }
        bool raised=false;
        if(m.read_word(p+0x10u)==3&&m.read_word(p+0x14u)==3&&signed_word(m.read_word(p+0x242u))>-12)
            raised=m.read_word(field()+(right?0x1au:0x16u))!=0;
        PairCellState state;state.a[0]=field()+2u;
        const auto row=m.read_word(p+0x14u),col=m.read_word(p+0x10u);
        auto cell=pair_cell(state,row,col).a[0];
        if(raised)cell-=0x10u;
        const std::uint32_t delta=right?2u:0xfffffffeu;
        const auto first=m.read_word(cell+delta);
        const auto index=std::uint16_t(m.read_byte(p+0x170u)*2u);
        const auto offset_first=m.read_word(0x12bb8u+index);
        const auto offset=static_cast<std::uint16_t>(offset_first+m.read_word(0x12bc0u+index));
        cell+=(offset&0x8000u)?(0xffff0000u|offset):offset;
        if(static_cast<std::uint16_t>(first+m.read_word(cell+delta)))return;
        if(raised) {
            m.write_word(p+0x14u,2);m.write_word(p+0x242u,0xffffu);
        } else if(m.read_word(p+0x14u)==2)m.write_word(p+0x242u,0xffffu);
        m.write_word(p+0x10u,static_cast<std::uint16_t>(m.read_word(p+0x10u)+(right?1u:0xffffu)));
        effects.move_sound(p);++result.sounds;
        if(right)++result.right;else ++result.left;
    };
    move(true);move(false);
    return result;
}
} // namespace puzzle_fighter
