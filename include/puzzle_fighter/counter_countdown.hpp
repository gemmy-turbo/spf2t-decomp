// SPDX-License-Identifier: GPL-3.0-only
// USA D8BC..D933 reachable countdown paths; sound is an explicit effect.
// Instruction-derived candidate; validation pending.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct CounterCountdownResult {
    unsigned candidates{},decremented{},expired{};
    bool sound_requested{};
};
// Effects.counter_sound(player) requests original helper 2BAD0. Its bounded
// audio scratch/queue state must be accounted for by the integration adapter.
template<class Memory,class Effects>
CounterCountdownResult counter_countdown(std::uint32_t player,Memory&m,Effects&effects) {
    CounterCountdownResult result;
    const auto high=m.read_word(player+0x164u);
    const auto field=(std::uint32_t(high)<<16)|m.read_word(player+0x166u);
    for(unsigned column=0;column<6;++column) {
        auto cell=field+0xc2u+column*2u;
        for(unsigned row=0;row<14;++row,cell-=0x10u) {
            if((m.read_word(cell)&15u)!=7)continue;
            const auto counter=m.read_byte(cell);
            if(!counter)continue;
            ++result.candidates;
            const auto colour=m.read_byte(cell)&0xf0u;
            auto count=counter&15u;
            if(count){--count;++result.decremented;}
            if(!count){m.write_byte(player+0x246u,0x20);++result.expired;result.sound_requested=true;}
            m.write_byte(cell,static_cast<std::uint8_t>(colour+count));
        }
    }
    if(result.sound_requested)effects.counter_sound(player);
    return result;
}
} // namespace puzzle_fighter
