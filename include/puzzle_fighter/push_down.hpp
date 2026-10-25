// SPDX-License-Identifier: GPL-3.0-only
// USA 11BBE..11C5D. Gameplay soft-drop gates, counters and forcing state.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct PushDownResult {
    bool active=false,sound_requested=false,force_fall=false;
    std::uint8_t threshold=0;
};
// Effects.soft_drop_sound(player) represents original 2BA74. The original
// bounded queue closure does not mutate these gameplay fields for normal A5
// and valid aligned queue indices 0..0xff0; hosts supply their sound adapter.
template<class Memory,class Effects>
PushDownResult push_down(std::uint32_t player,Memory&m,Effects&effects) {
    PushDownResult result;
    const auto signed_word=[](std::uint16_t v){return v<0x8000u?int(v):int(v)-0x10000;};
    const auto signed_byte=[](std::uint8_t v){return v<0x80u?int(v):int(v)-0x100;};
    if(m.read_byte(player+0x19au))return result;
    if((m.read_word(player+0x140u)&0xfu)!=4u)return result;
    if(signed_word(m.read_word(player+0x1a6u))<=0x10)return result;
    result.active=true;
    bool sound=false;
    if(!m.read_byte(player+0x329u))sound=m.read_word(player+0x14u)==3u;
    if(!sound) {
        const auto previous=m.read_word(player+0x142u);
        const auto pressed=static_cast<std::uint16_t>(~previous)&m.read_word(player+0x140u);
        if(pressed&4u)sound=!m.read_byte(player+0x9eu);
    }
    if(sound) {
        effects.soft_drop_sound(player);
        m.write_byte(player+0x329u,1);
        result.sound_requested=true;
    }
    m.write_byte(player+0x1a9u,static_cast<std::uint8_t>(m.read_byte(player+0x1a9u)+1u));
    m.write_word(player+0x320u,static_cast<std::uint16_t>(m.read_word(player+0x320u)+1u));
    if(signed_word(m.read_word(player+0x320u))<6)result.threshold=4;
    else if(signed_word(m.read_word(player+0x320u))<12)result.threshold=2;
    else result.threshold=1;
    if(int(result.threshold)>signed_byte(m.read_byte(player+0x1a9u)))return result;
    m.write_byte(player+0x15eu,0);
    m.write_byte(player+0x1a9u,0);
    m.write_word(player+0x242u,0);
    result.force_fall=true;
    return result;
}
} // namespace puzzle_fighter
