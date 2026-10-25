// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 11E6A..11F33. Instruction-derived candidate; validation pending.
#pragma once
#include "pair_cell.hpp"
namespace puzzle_fighter {
struct MakeRoomResult {std::uint32_t cell;std::uint16_t turn;bool pushed{};};
template<class Memory>
MakeRoomResult make_room_to_turn(std::uint32_t player,std::uint32_t cell,std::uint16_t turn,Memory&m) {
    MakeRoomResult result{cell,turn,false};
    const auto signed_word=[](std::uint16_t word)->std::uint32_t{return (word&0x8000u)?0xffff0000u|word:word;};
    const auto index=signed_word(static_cast<std::uint16_t>(turn*2u));
    const auto offset=[&](std::uint32_t table) {
        const auto first=m.read_word(table+index);
        return signed_word(static_cast<std::uint16_t>(first+m.read_word(table+index+8u)));
    };
    if(!m.read_word(cell+offset(0x12558u))||m.read_word(cell+offset(0x12538u))) {
        m.write_byte(player+0x1a3u,0);m.write_byte(player+0x247u,2);return result;
    }
    const auto column_delta=m.read_word(0x12548u+index);
    m.write_word(player+0x10u,static_cast<std::uint16_t>(m.read_word(player+0x10u)+column_delta));
    const auto row_delta=m.read_word(0x12550u+index);
    m.write_word(player+0x14u,static_cast<std::uint16_t>(m.read_word(player+0x14u)+row_delta));
    m.write_byte(player+0x397u,1);
    if(row_delta&0x8000u) {
        const auto high=m.read_word(player+0x164u);
        PairCellState state;state.a[0]=((std::uint32_t(high)<<16)|m.read_word(player+0x166u))+2u;
        const auto row=m.read_word(player+0x14u);const auto column=m.read_word(player+0x10u);
        result.cell=pair_cell(state,row,column).a[0];
        if(m.read_word(result.cell+0x20u)) {
            m.write_byte(player+0x1a4u,0);
            m.write_word(player+0x314u,static_cast<std::uint16_t>(m.read_word(player+0x314u)+1u));
            const auto kicks=m.read_word(player+0x314u);
            if(!(kicks&0x8000u)&&kicks>2u) {
                m.write_word(player+0x314u,5);m.write_byte(player+0x316u,1);m.write_byte(player+0x176u,1);
            }
        }else {m.write_word(player+0x314u,0);m.write_byte(player+0x316u,0);}
    }
    m.write_byte(player+0x1a3u,1);
    if(m.read_byte(player+0x1a3u)==1) {
        result.turn=static_cast<std::uint16_t>((m.read_byte(player+0x170u)+1u)&3u);
        const auto direction=m.read_byte(player+0x1a4u);
        m.write_byte(player+0x170u,static_cast<std::uint8_t>(m.read_byte(player+0x170u)+direction));
        m.write_byte(player+0x170u,static_cast<std::uint8_t>(m.read_byte(player+0x170u)&3u));
    }
    result.pushed=true;return result;
}
} // namespace puzzle_fighter
