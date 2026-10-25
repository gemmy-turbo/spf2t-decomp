// SPDX-License-Identifier: GPL-3.0-only
// USA 00D26A..00D355 with D23E/D252 helpers and C8B8/C948 bonus writes.
// Original instruction-derived candidate; independent validation pending.
#pragma once
#include <cstdint>
#include <initializer_list>
namespace puzzle_fighter {
struct DiamondBreaksResult {
    unsigned visited{},converted_counters{},marked_cells{};
    bool scanned{},found{},floor_bonus{};
};
template<class Memory>
DiamondBreaksResult diamond_breaks(std::uint32_t player,Memory&m) {
    DiamondBreaksResult result;
    m.write_byte(player+0x390u,0);
    const auto under=m.read_word(player+0x1f2u);
    if(under!=0xfeu && (under&7u)) {
        result.scanned=true;
        auto colour=static_cast<std::uint16_t>(under&7u);
        if(colour==7)colour=(m.read_byte(player+0x1f2u)>>4)&15u;
        const auto high=m.read_word(player+0x164u);
        const auto field=(std::uint32_t(high)<<16)|m.read_word(player+0x166u);
        for(unsigned column=0;column<6;++column) {
            auto cell=field+0xc2u+column*2u;
            for(unsigned row=0;row<14;++row,cell-=0x10u) {
                ++result.visited;
                auto kind=static_cast<std::uint16_t>(m.read_word(cell)&7u);
                if(!kind)continue;
                if(kind==7) {
                    kind=(m.read_byte(cell)>>4)&15u;
                    if(kind==colour){m.write_word(cell,kind);++result.converted_counters;}
                }
                if(kind==7)continue;
                if(kind==5) {
                    result.found=true;++result.marked_cells;
                    m.write_word(cell,m.read_word(cell)|0x20u);
                    const auto below=m.read_word(cell+0x10u);
                    if(below==0xffu)m.write_byte(player+0x390u,1);
                    else if(below==7)m.write_word(cell+0x10u,m.read_word(cell+0x10u)|0x20u);
                    else if(!(below&0x80u))m.write_word(cell+0x10u,0x26u);
                    m.write_word(player+0x1f2u,0xfeu);
                }else if(kind==colour) {
                    result.found=true;++result.marked_cells;
                    m.write_word(cell,m.read_word(cell)|0x20u);
                }
            }
        }
        if(result.found) {
            m.write_byte(player+0x17eu,3);m.write_byte(player+0x174u,1);
            m.write_byte(player+0x175u,0x28);m.write_byte(player+0x18cu,1);
        }
    }
    if(m.read_byte(player+0x390u)) {
        result.floor_bonus=true;
        const auto high=m.read_word(player+0xaau);
        const auto score=((std::uint32_t(high)<<16)|m.read_word(player+0xacu))+10000u;
        // Original ADDI.L memory writeback is low word, then high word.
        m.write_word(player+0xacu,static_cast<std::uint16_t>(score));
        m.write_word(player+0xaau,static_cast<std::uint16_t>(score>>16));
        m.write_byte(player+0x38du,1);m.write_byte(player+0x381u,0x3c);
        // Original bonus display clears three strips in the tilemap. Keep the
        // repeated pointer reads and overlapping writes, including alias effects.
        for(const auto offset:{0xfffffffcu,0xfffffff8u,0u}) {
            const auto p_high=m.read_word(player+0x382u);
            auto tile=((std::uint32_t(p_high)<<16)|m.read_word(player+0x384u))+offset;
            for(unsigned row=0;row<12;++row,tile+=0x80u) {
                m.write_word(tile,0x20);m.write_word(tile+2u,0);
                m.write_word(tile+4u,0x20);m.write_word(tile+6u,0);
            }
        }
    }
    return result;
}
} // namespace puzzle_fighter
