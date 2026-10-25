// SPDX-License-Identifier: GPL-3.0-only
// USA 00D462..00D6D5. Original state flow; independent validation pending.
#pragma once
#include "break_adjacent_counters.hpp"
#include "chain_display.hpp"
#include <cstdint>
#include <initializer_list>
namespace puzzle_fighter {
struct MarkBreaksResult {
    std::uint32_t processed{},stored_count{};
    unsigned propagated_edges{};
    bool found{},chain_called{},count_stored{};
};
template<class Memory,class Effects>
MarkBreaksResult mark_breaks(std::uint32_t player,std::uint32_t globals,Memory&m,Effects&effects) {
    MarkBreaksResult result;
    const auto high=m.read_word(player+0x164u);
    const auto field=(std::uint32_t(high)<<16)|m.read_word(player+0x166u);
    for(unsigned column=0;column<6;++column) {
        auto cell=field+0xc2u+column*2u;
        for(unsigned row=0;row<14;++row,cell-=0x10u) {
            const auto initial=m.read_word(cell);
            if(!initial || (initial&0xfu)==7)continue;
            auto starter=m.read_word(cell);
            if(starter&0x20u) {
                if(!(starter&0x10u))continue;
                (void)break_adjacent_counters(player,cell,static_cast<std::uint16_t>(13u-row),m);
            }else{
                if(starter&0x40u)continue;
                starter&=0xfu;
                if(starter<=7)continue;
            }
            m.write_word(player+0x25au,starter);
            m.write_byte(cell+1u,m.read_byte(cell+1u)&0xdfu);
            m.write_byte(cell+1u,m.read_byte(cell+1u)|0x40u);
            const auto kind=m.read_word(cell)&0xfu;
            if(kind>=1&&kind<=4)
                m.write_byte(player+0x394u,m.read_byte(player+0x394u)|(1u<<(kind-1)));
            ++result.processed;
            const auto colour=starter&7u;
            bool spread=false;
            for(const auto offset:{0xfffffffeu,2u,0x10u,0xfffffff0u}) {
                const auto beside=cell+offset;
                if((m.read_word(beside)&7u)==7)continue;
                const auto low=m.read_byte(beside+1u);
                if((low&0x50u)||(low&7u)!=colour)continue;
                m.write_word(beside,m.read_word(beside)|0x20u);
                m.write_word(cell,m.read_word(cell)|0x20u);
                result.found=true;spread=true;++result.propagated_edges;
                (void)break_adjacent_counters(player,cell,static_cast<std::uint16_t>(13u-row),m);
                (void)break_adjacent_counters(player,beside,static_cast<std::uint16_t>(13u-row),m);
            }
            if(!spread)m.write_byte(cell+1u,m.read_byte(cell+1u)&0x9fu);
        }
    }
    result.stored_count=result.processed;
    if(!result.found){m.write_byte(player+0x31au,0);return result;}
    m.write_byte(player+0x17eu,3);m.write_byte(player+0x31au,1);
    m.write_byte(player+0x174u,1);m.write_byte(player+0x175u,0x28);m.write_byte(player+0x18cu,1);
    if(!m.read_byte(player+0x37cu)) {
        m.write_byte(player+0xd8u,static_cast<std::uint8_t>(m.read_byte(player+0x19cu)+1u));
        result.stored_count=chain_display(player,globals,result.processed,m,effects).count;
        result.chain_called=true;m.write_byte(player+0x37cu,1);
    }
    const auto colour=m.read_word(player+0x25au)&7u;
    if(colour!=m.read_word(player+0x258u)) {
        const auto signed_word=[](std::uint16_t w){return std::uint32_t(w)|(w&0x8000u?0xffff0000u:0u);};
        const auto colour_offset=signed_word(m.read_word(player+0x254u));
        m.write_word(player+0x280u+colour_offset,colour);
        const auto count_offset=signed_word(m.read_word(player+0x254u));
        m.write_word(player+0x2c0u+count_offset,static_cast<std::uint16_t>(result.stored_count));
        m.write_word(player+0x254u,static_cast<std::uint16_t>(m.read_word(player+0x254u)+2u));
        result.count_stored=true;
    }
    return result;
}
} // namespace puzzle_fighter
