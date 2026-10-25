// SPDX-License-Identifier: GPL-3.0-only
// USA ScoreBreak prefix FAB0 through arrival at FB90; validation pending.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct RemoveAndTallyResult {
    std::uint32_t field{};
    std::uint16_t removed{};
    unsigned big_cells{},normal_cells{},counter_cells{},crash_cells{};
};
// Only the original removal/tally prefix. Later score/attack formulas are
// separate dependencies; no replacement formula is supplied here.
template<class Memory>
RemoveAndTallyResult remove_and_tally(std::uint32_t player,Memory&m) {
    RemoveAndTallyResult result;
    const auto high=m.read_word(player+0x164u);
    result.field=(std::uint32_t(high)<<16)|m.read_word(player+0x166u);
    m.write_word(player+0x35cu,0);
    for(unsigned column=0;column<6;++column) {
        auto cell=result.field+0xc2u+column*2u;
        for(unsigned row=0;row<14;++row,cell-=0x10u) {
            auto value=m.read_word(cell);
            if(!value || !(value&0x10u))continue;
            m.write_word(cell,0);
            if(value&0x80u) {
                ++result.big_cells;
                m.write_byte(player+0x32du,static_cast<std::uint8_t>(m.read_byte(player+0x32du)+1u));
                const auto colour=value&7u;
                const auto index=colour?colour-1u:0u;
                m.write_byte(player+0x3a0u+index,static_cast<std::uint8_t>(m.read_byte(player+0x3a0u+index)+1u));
                // Original reread occurs after the clear and both byte adds.
                // Aliases can change this value; do not substitute literal zero.
                value=m.read_word(cell);
            }
            m.write_word(player+0x35au,static_cast<std::uint16_t>(5u-column));
            const auto limit=m.read_word(player+0x35cu);
            const int signed_limit=limit<0x8000u?int(limit):int(limit)-65536;
            if(int(13u-row)>signed_limit)m.write_word(player+0x35cu,static_cast<std::uint16_t>(13u-row));
            const auto kind=value&15u;
            if(kind==7) {
                ++result.counter_cells;
                m.write_word(player+0x208u,static_cast<std::uint16_t>(m.read_word(player+0x208u)+10u));
            }else if(kind<7) {
                ++result.normal_cells;
                m.write_word(player+0x204u,static_cast<std::uint16_t>(m.read_word(player+0x204u)+100u));
            }else{
                ++result.crash_cells;
                m.write_word(player+0x206u,static_cast<std::uint16_t>(m.read_word(player+0x206u)+100u));
                if(kind>=8 && kind<=13)m.write_byte(player+0x1e8u+(kind-8u),1);
            }
            m.write_word(player+0x200u,static_cast<std::uint16_t>(m.read_word(player+0x200u)+1u));
            ++result.removed;
        }
    }
    return result;
}
} // namespace puzzle_fighter
