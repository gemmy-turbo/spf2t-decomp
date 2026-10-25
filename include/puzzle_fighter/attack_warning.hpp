// SPDX-License-Identifier: GPL-3.0-only
// USA C50C..C5E1 with actor 142D2 and palette 14770/14756 closure.
// Instruction-derived candidate; original execution validation pending.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct AttackWarningResult {
    bool latched{},height_checked{},actor_updated{};
    std::uint16_t empty_rows{};
    std::uint8_t initial_pending{};
    unsigned palette_longs{};
};
namespace attack_warning_detail {
constexpr std::int32_t signed_word(std::uint16_t v) {
    return v<0x8000u?std::int32_t(v):std::int32_t(v)-0x10000;
}
}
// Memory includes ordinary DATA ROM and palette RAM. Field scans require an
// original-valid nonzero terminator; this function does not invent a bound.
template<class Memory>
AttackWarningResult attack_warning(std::uint32_t player,std::uint32_t globals,Memory&m) {
    using attack_warning_detail::signed_word;
    AttackWarningResult result;
    if(m.read_byte(player+0x396u))return result;
    m.write_byte(player+0x396u,1);result.latched=true;
    const auto initial_high=m.read_word(player+0x160u);
    auto opponent=(std::uint32_t(initial_high)<<16)|m.read_word(player+0x162u);
    const auto pending=m.read_byte(opponent+0x19eu);result.initial_pending=pending;
    if(!pending)return result;
    m.write_byte(opponent+0x244u,0);m.write_byte(opponent+0xd0u,0);
    if(pending>=12)m.write_byte(opponent+0xd0u,2);
    if(pending>=18)m.write_byte(opponent+0xd0u,4);
    if(pending>=24)m.write_byte(opponent+0xd0u,6);
    if(pending>=30) {
        m.write_byte(opponent+0xd0u,8);result.height_checked=true;
        const auto field_high=m.read_word(player+0x164u);
        auto cell=((std::uint32_t(field_high)<<16)|m.read_word(player+0x166u))+8u;
        std::uint16_t empty=0;
        while(!m.read_word(cell)){empty=static_cast<std::uint16_t>(empty+1u);cell+=0x10u;}
        result.empty_rows=empty;
        const auto height=static_cast<std::uint16_t>(13u-empty+pending/6u);
        if(signed_word(height)>=13){m.write_byte(opponent+0xd0u,10);m.write_byte(player+0x260u,1);}
    }
    const auto current_high=m.read_word(player+0x160u);
    opponent=(std::uint32_t(current_high)<<16)|m.read_word(player+0x162u);
    const auto current_pending=m.read_byte(opponent+0x19eu);
    const auto total=static_cast<std::uint16_t>(current_pending+m.read_word(player+0x374u));
    m.write_byte(player+0x253u,0);
    if(signed_word(total)>=12)m.write_byte(player+0x253u,2);
    if(signed_word(total)>=30)m.write_byte(player+0x253u,4);
    if(m.read_byte(globals+0x107u))return result;
    const auto actor_word=m.read_word(opponent+0x32u);
    const auto actor=actor_word<0x8000u?std::uint32_t(actor_word):0xffff0000u|actor_word;
    if(m.read_byte(actor+0xd9u))return result;
    m.write_byte(actor+0xd0u,m.read_byte(opponent+0xd0u));
    m.write_word(actor+4u,0x020a);m.write_word(actor+6u,0);result.actor_updated=true;
    auto destination=m.read_byte(player+2u)?0x90c200u:0x90c180u;
    const auto character=std::uint16_t(m.read_byte(player+0x82u))*2u;
    const auto colour=m.read_byte(player+0x9fu);
    const auto index=static_cast<std::uint16_t>((character&0xff00u)|((character+colour)&0xffu));
    auto source=0xa0000u+static_cast<std::uint32_t>(signed_word(static_cast<std::uint16_t>(index<<7)));
    for(unsigned i=0;i<32;++i,destination+=4u,source+=4u) {
        const auto old_high=m.read_word(destination),old_low=m.read_word(destination+2u);
        const auto new_high=m.read_word(source),new_low=m.read_word(source+2u);
        m.write_word(destination,static_cast<std::uint16_t>(new_high|(old_high&0xf000u)));
        m.write_word(destination+2u,static_cast<std::uint16_t>(new_low|(old_low&0xf000u)));
        ++result.palette_longs;
    }
    return result;
}
} // namespace puzzle_fighter
