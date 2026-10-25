// SPDX-License-Identifier: GPL-3.0-only
// USA D824..D8BB. Original instruction-derived candidate; validation pending.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct CounterConversionResult {
    bool timer_decremented{};
    unsigned candidates{},unchanged_count_writes{},flashed{},converted{};
};
template<class Memory>
CounterConversionResult counter_conversion(std::uint32_t player,Memory&m) {
    CounterConversionResult result;
    if(m.read_byte(player+0x246u)) {
        m.write_byte(player+0x246u,static_cast<std::uint8_t>(m.read_byte(player+0x246u)-1u));
        result.timer_decremented=true;
    }
    const auto high=m.read_word(player+0x164u);
    const auto field=(std::uint32_t(high)<<16)|m.read_word(player+0x166u);
    for(unsigned column=0;column<6;++column) {
        auto cell=field+0xc2u+column*2u;
        for(unsigned row=0;row<14;++row,cell-=0x10u) {
            const auto kind=m.read_word(cell)&15u;
            if(kind!=6 && kind!=7)continue;
            const auto counter=m.read_byte(cell);
            if(!counter)continue;
            ++result.candidates;
            const auto colour=m.read_byte(cell)&0xf0u;
            const auto count=counter&15u;
            if(count) {
                // ASR.B followed by ASL.B reconstructs the colour byte;
                // ADD.W then MOVE.B preserves this original unchanged write.
                m.write_byte(cell,static_cast<std::uint8_t>(colour+count));
                ++result.unchanged_count_writes;
            }else if(m.read_byte(player+0x246u)) {
                const auto phase=m.read_byte(player+0x246u)&3u;
                const auto value=m.read_word(cell)&0xf0f0u;
                m.write_word(cell,static_cast<std::uint16_t>(value+(phase?7u:6u)));
                ++result.flashed;
            }else{
                const auto value=m.read_byte(cell)&0xf0u;
                // MOVEQ clears D3 before ASR.B: sign-filled low byte is then
                // written as an unsigned 00xx word, not sign-extended to FFxx.
                const auto shifted=(value>>4)|((value&0x80u)?0xf0u:0u);
                m.write_word(cell,static_cast<std::uint16_t>(shifted));
                ++result.converted;
            }
        }
    }
    return result;
}
} // namespace puzzle_fighter
