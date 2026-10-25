// SPDX-License-Identifier: GPL-3.0-only
// USA F72A..F805. Original break-timer/scoring transition; validation pending.
#pragma once
#include "score_break.hpp"
#include "chain_banner.hpp"
namespace puzzle_fighter {
struct ShowBreakResult {
    bool waiting{},scored{},advanced{},pressure_added{},pressure_folded{},pressure_scaled{};
    ScoreBreakResult score{};
};
template<class Memory,class Tables>
ShowBreakResult show_break(std::uint32_t player,std::uint32_t globals,Memory&m,Tables&tables) {
    ShowBreakResult result;
    if(m.read_byte(player+0x175u)) {
        m.write_byte(player+0x177u,1);
        const auto timer=static_cast<std::uint8_t>(m.read_byte(player+0x175u)-1u);
        m.write_byte(player+0x175u,timer);
        if(timer){result.waiting=true;return result;}
        m.write_byte(player+0x17eu,3);
        for(const auto offset:{0x200u,0x204u,0x206u,0x208u})m.write_word(player+offset,0);
        for(unsigned offset=0x1e8u;offset<=0x1edu;++offset)m.write_byte(player+offset,0);
        m.write_byte(player+0x174u,0);m.write_byte(player+0x171u,1);
        m.write_byte(player+0x19cu,static_cast<std::uint8_t>(m.read_byte(player+0x19cu)+1u));
        m.write_byte(player+0x38cu,0);
        chain_banner(player,m);
        result.score=score_break(player,globals,m,tables);result.scored=true;
        m.write_byte(player+0x1fau,0);
        return result;
    }
    const auto high=m.read_word(player+0x160u);
    const auto opponent=(std::uint32_t(high)<<16)|m.read_word(player+0x162u);
    const auto chain=m.read_byte(player+0x19cu);
    const auto increment=static_cast<std::uint16_t>(std::uint32_t(chain)*(chain?chain-1u:0u));
    if(!(increment&0x8000u)) {
        m.write_word(opponent+0x1b0u,static_cast<std::uint16_t>(m.read_word(opponent+0x1b0u)+increment));result.pressure_added=true;
    }
    m.write_byte(player+0x17eu,static_cast<std::uint8_t>(m.read_byte(player+0x17eu)+1u));result.advanced=true;
    m.write_byte(player+0x19fu,0);
    if(m.read_byte(player+0x18du)) {
        const auto later=m.read_word(opponent+0x1b2u);
        m.write_word(opponent+0x1b0u,static_cast<std::uint16_t>(m.read_word(opponent+0x1b0u)+later));
        m.write_word(opponent+0x1b2u,0);result.pressure_folded=true;
    }
    const auto pressure=m.read_word(opponent+0x1b0u);
    if(pressure) {
        const auto scaled=(pressure&0x8000u)?1u:(pressure>>2)>0?pressure>>2:1u;
        m.write_word(opponent+0x1b0u,static_cast<std::uint16_t>(scaled));result.pressure_scaled=true;
    }
    return result;
}
} // namespace puzzle_fighter
