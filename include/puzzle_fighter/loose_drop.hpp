// SPDX-License-Identifier: GPL-3.0-only
// USA D058..D0E7. Original instruction-derived candidate; validation pending.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct LooseDropResult {
    unsigned moved{};
    bool restarted{};
};
template<class Memory>
LooseDropResult loose_drop(std::uint32_t player,Memory&m) {
    LooseDropResult result;
    m.write_byte(player+0x1e4u,0);
    const auto high=m.read_word(player+0x164u);
    const auto field=(std::uint32_t(high)<<16)|m.read_word(player+0x166u);
    for(unsigned column=0;column<6;++column) {
        auto cell=field+0xc2u+column*2u;
        bool supported=true;
        for(unsigned row=0;row<15;++row,cell-=0x10u) {
            if(!m.read_word(cell)){supported=false;continue;}
            if(supported)continue;
            const auto value=m.read_word(cell);
            if(value&0x80u){supported=true;continue;}
            m.write_word(cell+0x10u,value);m.write_word(cell,0);
            m.write_byte(player+0x174u,1);m.write_byte(player+0x171u,1);
            m.write_byte(player+0x177u,1);m.write_byte(player+0x178u,1);
            m.write_byte(player+0x18cu,1);++result.moved;
            supported=false;
        }
    }
    m.write_byte(player+0x17eu,0);
    if(m.read_byte(player+0x178u)) {
        result.restarted=true;
        m.write_byte(player+0x178u,0);m.write_byte(player+0x174u,0);
        m.write_byte(player+0x171u,1);m.write_byte(player+0x17eu,0x11);
    }
    return result;
}
} // namespace puzzle_fighter
