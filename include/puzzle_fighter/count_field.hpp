// SPDX-License-Identifier: GPL-3.0-only
// USA D76E..D823 with original C8CC/C948 all-clear banner writes.
// Instruction-derived candidate; independent validation pending.
#pragma once
#include <cstdint>
#include <initializer_list>
namespace puzzle_fighter {
struct CountFieldResult {
    std::uint16_t gems{};
    unsigned removed{},metadata_cleared{};
    std::uint8_t danger{};
    bool any{},all_clear{};
};
template<class Memory>
CountFieldResult count_field(std::uint32_t player,std::uint32_t globals,Memory&m) {
    CountFieldResult result;
    const auto high=m.read_word(player+0x164u);
    const auto field=(std::uint32_t(high)<<16)|m.read_word(player+0x166u);
    for(unsigned column=0;column<6;++column) {
        auto cell=field+0xc2u+column*2u;
        for(unsigned row=0;row<13;++row,cell-=0x10u) {
            if(!m.read_word(cell))continue;
            if(m.read_byte(cell+1u)==0x57u) {
                m.write_word(cell,0);++result.removed;continue;
            }
            const auto value=m.read_word(cell);result.any=true;
            if(value==0xffu)continue;
            ++result.gems;
            if(!(value&0x80u)){m.write_word(cell+0x400u,0);++result.metadata_cleared;}
        }
    }
    m.write_word(player+0x374u,result.gems);
    result.danger=result.gems<36?0:result.gems<54?1:2;
    m.write_byte(player+0xd1u,result.danger);
    if(result.any || m.read_byte(globals+0xd8u) || m.read_byte(player+0x3a6u))return result;
    result.all_clear=true;m.write_byte(player+0x3a6u,1);
    const auto bonus_high=m.read_word(player+0x25cu);
    auto bonus=((std::uint32_t(bonus_high)<<16)|m.read_word(player+0x25eu))+600u;
    // Original memory ADDI.L and ADD.L write the low word first.
    m.write_word(player+0x25eu,static_cast<std::uint16_t>(bonus));
    m.write_word(player+0x25cu,static_cast<std::uint16_t>(bonus>>16));
    m.write_byte(player+0x392u,static_cast<std::uint8_t>(m.read_byte(player+0x392u)+6u));
    const auto saved_high=m.read_word(player+0x25cu);
    bonus=(std::uint32_t(saved_high)<<16)|m.read_word(player+0x25eu);
    const auto score_high=m.read_word(player+0xaau);
    const auto score=((std::uint32_t(score_high)<<16)|m.read_word(player+0xacu))+bonus;
    m.write_word(player+0xacu,static_cast<std::uint16_t>(score));
    m.write_word(player+0xaau,static_cast<std::uint16_t>(score>>16));
    const auto attack=m.read_byte(player+0x392u);
    m.write_byte(player+0x19eu,static_cast<std::uint8_t>(m.read_byte(player+0x19eu)+attack));
    m.write_byte(player+0x38du,2);m.write_byte(player+0x381u,0x3c);
    for(const auto offset:{0xfffffffcu,0xfffffff8u,0u}) {
        const auto tile_high=m.read_word(player+0x382u);
        auto tile=((std::uint32_t(tile_high)<<16)|m.read_word(player+0x384u))+offset;
        for(unsigned row=0;row<12;++row,tile+=0x80u) {
            m.write_word(tile,0x20);m.write_word(tile+2u,0);
            m.write_word(tile+4u,0x20);m.write_word(tile+6u,0);
        }
    }
    return result;
}
} // namespace puzzle_fighter
