// SPDX-License-Identifier: GPL-3.0-only
// USA 11D06..11DB7 (left) and 11DB8..11E69 (right).
#pragma once
#include "make_room_to_turn.hpp"
namespace puzzle_fighter {
struct TurnPairResult { bool attempted{},pushed{},locked{},turned{}; };
// The bounded original 2BA18 closure is a host sound effect. Legacy CPU
// scratch/stack state is outside this gameplay API. Tables are external data.
template<class Memory,class Effects>
TurnPairResult turn_pair(std::uint32_t player,bool right,Memory&m,Effects&effects) {
    TurnPairResult result;
    if(m.read_word(player+0x14u)==2u && (m.read_word(player+0x242u)&0x8000u))return result;
    if(m.read_byte(player+0x316u))return result;
    const auto previous=m.read_word(player+0x142u);
    const auto pressed=static_cast<std::uint16_t>(~previous)&m.read_word(player+0x140u);
    if(!(pressed&(right?0x200u:0x100u)))return result;
    if(m.read_byte(player+0x176u)==0x10u)return result;
    result.attempted=true;effects.turn_sound(player);
    const auto high=m.read_word(player+0x164u);
    PairCellState state;state.a[0]=((std::uint32_t(high)<<16)|m.read_word(player+0x166u))+2u;
    const auto row=m.read_word(player+0x14u);const auto column=m.read_word(player+0x10u);
    const auto cell=pair_cell(state,row,column).a[0];
    m.write_word(player+0x248u,0);m.write_word(player+0x24au,0);m.write_word(player+0x24cu,8);
    const auto initial_cell=m.read_word(cell);
    const auto turn=static_cast<std::uint16_t>((m.read_byte(player+0x170u)+(right?1u:0xffffu))&3u);
    m.write_byte(player+0x1a4u,right?1:0);
    const auto room=make_room_to_turn(player,cell,turn,m);result.pushed=room.pushed;
    if(m.read_byte(player+0x247u)!=2u)m.write_byte(player+0x247u,right?0:1);
    if(m.read_byte(player+0x316u)){result.locked=true;return result;}
    const auto doubled=static_cast<std::uint16_t>(room.turn*2u);
    const auto index=std::uint32_t(doubled&0x8000u?0xffff0000u|doubled:doubled);
    const auto table=right?0x12558u:0x12568u;
    const auto first=m.read_word(table+index);
    const auto delta=static_cast<std::uint16_t>(first+m.read_word(table+index+8u));
    const auto offset=std::uint32_t(delta&0x8000u?0xffff0000u|delta:delta);
    if(static_cast<std::uint16_t>(initial_cell+m.read_word(room.cell+offset)))return result;
    m.write_byte(player+0x170u,static_cast<std::uint8_t>(m.read_byte(player+0x170u)+(right?1u:255u)));
    m.write_byte(player+0x170u,static_cast<std::uint8_t>(m.read_byte(player+0x170u)&3u));
    m.write_byte(player+0x247u,right?0:1);result.turned=true;return result;
}
} // namespace puzzle_fighter
