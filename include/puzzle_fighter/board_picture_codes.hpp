// SPDX-License-Identifier: GPL-3.0-only
// USA D9AE..DAF9 plus DAFA..DCA9. This changes gameplay cells and metadata.
// Instruction-derived candidate; independent validation pending.
#pragma once
#include <cstdint>
#include <array>
#include "shape_code.hpp"
namespace puzzle_fighter {
struct BoardPictureCodesResult {
    bool gated{};
    unsigned walls{},cleared{},counters{},ungrouped{},shapes{};
};
template<class Memory>
BoardPictureCodesResult board_picture_codes(std::uint32_t player,Memory&m) {
    BoardPictureCodesResult result;
    if(m.read_byte(player+0x31au)){result.gated=true;return result;}
    const auto field_high=m.read_word(player+0x164u);
    const auto field=(std::uint32_t(field_high)<<16)|m.read_word(player+0x166u);
    const auto picture_high=m.read_word(player+0x168u);
    const auto picture=(std::uint32_t(picture_high)<<16)|m.read_word(player+0x16au);
    constexpr std::array<std::uint32_t,4> offsets{0xfffffff0u,2u,0x10u,0xfffffffeu};
    for(unsigned column=0;column<6;++column) {
        auto cell=field+0xc2u+column*2u;
        auto code=picture+0xc2u+column*2u;
        for(unsigned row=0;row<13;++row,cell-=0x10u,code-=0x10u) {
            if(m.read_word(cell)==0xffu){++result.walls;continue;}
            if(!m.read_byte(cell+1u)) {
                m.write_word(cell,0);m.write_word(cell+0x400u,0);++result.cleared;continue;
            }
            m.write_word(code,m.read_word(code)&0xff00u);
            if((m.read_word(cell)&15u)==7u){++result.counters;continue;}
            const auto group=m.read_byte(cell+0x400u);
            if(!group) {
                m.write_byte(cell,0);m.write_byte(cell+0x400u,0);++result.ungrouped;continue;
            }
            shape_code(cell,group,m);++result.shapes;
            const auto low=m.read_word(cell)&0xffu;
            const auto current_group=m.read_byte(cell+0x400u);
            unsigned edges=0;
            for(unsigned direction=0;direction<4;++direction) {
                const auto neighbor=cell+offsets[direction];
                if((m.read_word(neighbor)&15u)==7u)continue;
                if((m.read_word(neighbor)&0xffu)!=low)continue;
                if(m.read_byte(neighbor+0x400u)==current_group)edges|=1u<<direction;
            }
            // LEA-PC followed by register-indirect MOVE.W: ordinary ROM DATA.
            const auto increment=m.read_word(0xffd8u+edges*2u);
            m.write_word(code,static_cast<std::uint16_t>(m.read_word(code)+increment));
        }
    }
    return result;
}
} // namespace puzzle_fighter
