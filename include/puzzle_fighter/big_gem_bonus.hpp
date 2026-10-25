// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 F8E0..F9E3: +202 bonus producer and exact counter-marking rays.
// Original data tables remain external; no CPU, flags, ABI or timing model.
#pragma once
#include <cstdint>

namespace puzzle_fighter {
struct BigGemBonusResult {
    std::uint16_t selected_cells=0,coefficient=0,bonus=0;
};
namespace big_gem_bonus_detail {
constexpr std::int32_t signed_word(std::uint16_t value) {
    return value<0x8000u?value:std::int32_t(value)-0x10000;
}
constexpr std::uint32_t indexed(std::uint32_t base,std::uint16_t offset) {
    return base+static_cast<std::uint32_t>(signed_word(offset));
}
} // namespace big_gem_bonus_detail

// These are rays, not a flood fill. Ordinary cells do not stop them. The up
// ray has a different exact sentinel from the other three. The adapter/test
// domain must supply terminating memory; the original has no distance bound.
template<class Memory>
void mark_big_gem_bonus_rays(std::uint32_t cell,Memory &memory) {
    constexpr std::int32_t steps[]={2,-2,16,-16};
    for(const auto step:steps) {
        auto address=cell;
        const std::uint16_t sentinel=step==-16?0:0xff;
        for(;;) {
            address+=static_cast<std::uint32_t>(step);
            const auto word=memory.read_word(address);
            if(word==sentinel)break;
            if(word==7) {
                // Original ORI.W is a separate read/modify/write access.
                const auto previous=memory.read_word(address);
                memory.write_word(address,static_cast<std::uint16_t>(previous|0x10u));
            }
        }
    }
}

template<class Memory>
BigGemBonusResult big_gem_bonus(std::uint32_t player,Memory &memory) {
    using namespace big_gem_bonus_detail;
    memory.write_word(player+0x202u,0);
    const auto high=memory.read_word(player+0x164u);
    const auto low=memory.read_word(player+0x166u);
    const auto field=(std::uint32_t(high)<<16)|low;
    auto column=field+0xc2u;
    for(unsigned x=0;x<6;++x,column+=2u) {
        auto cell=column;
        for(unsigned y=0;y<13;++y,cell-=16u) {
            if(!memory.read_byte(cell))continue; // High byte of the cell word.
            const auto word=memory.read_word(cell);
            if(!(word&0x10u))continue;
            const auto kind=word&15u;
            if(kind==15 || kind==7)continue;
            mark_big_gem_bonus_rays(cell,memory);
            const auto count=memory.read_word(player+0x202u);
            memory.write_word(player+0x202u,static_cast<std::uint16_t>(count+1u));
        }
    }
    BigGemBonusResult result;
    result.selected_cells=memory.read_word(player+0x202u);
    if(result.selected_cells) {
        auto index=static_cast<std::uint16_t>(result.selected_cells-1u);
        if(signed_word(index)>=50)index=50;
        const auto offset=static_cast<std::uint16_t>(index*2u);
        result.coefficient=memory.read_word(indexed(0xfff8u,offset));
        if(signed_word(result.coefficient)>150 && memory.read_byte(player+0x3a4u))result.coefficient=150;
        auto remaining=static_cast<std::uint16_t>(result.selected_cells-1u);
        do {
            result.bonus=static_cast<std::uint16_t>(result.bonus+result.coefficient);
            remaining=static_cast<std::uint16_t>(remaining-1u);
        } while(remaining!=0xffffu);
    }
    memory.write_word(player+0x202u,result.bonus);
    return result;
}
} // namespace puzzle_fighter
