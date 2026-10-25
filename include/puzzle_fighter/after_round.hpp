// SPDX-License-Identifier: GPL-3.0-only
// USA A996..AAB4, stopping at the match/next-scene boundary.
#pragma once
#include "attack_display.hpp"
namespace puzzle_fighter {
template<class M,class H> bool finish_round_announcements(std::uint32_t g,M&m,H&h){
    if(m.read_byte(g+0x12c))return false;
    h.sound(0x0280ff07,0,0);const auto winner=attack_display_detail::ptr(m.read_word(g+0x13a));
    m.write_word(g+0xc,m.read_word(g+0xc)+2);m.write_word(g+0xe,0);m.write_long(g+0x10,0);m.write_long(g+0x14,0);
    if(m.read_byte(winner+0x9e))m.write_word(g+0x10,6);
    return true;
}
// Returntrue at AAB0's branch boundary. The host still owns standalone rematch
// and scene creation. pressed is the source B9D8 result, not a new timer.
template<class M> bool after_round_phase(std::uint32_t g,M&m,bool pressed,bool allow_transition=true){
    auto advance=[&]{m.write_word(g+0x10,m.read_word(g+0x10)+2);};
    auto expired=[&]{const auto v=std::uint16_t(m.read_word(g+0x12)-1);m.write_word(g+0x12,v);return v&0x8000;};
    switch(m.read_word(g+0x10)){
    case 0:{advance();m.write_word(g+0x12,15);m.write_byte(g+0x10d,0);m.write_byte(g+0x301,0);m.write_byte(g+0x701,0);m.write_byte(g+0x133,0);m.write_byte(g+0x151,1);
        const auto winner=attack_display_detail::ptr(m.read_word(g+0x13a));if(m.read_word(winner+0x340)<60)m.write_byte(winner+0xb3,0xff);initialize_attack_display_pool(g,m);return false;}
    case 2:if(expired()){advance();m.write_word(g+0x12,240);}return false;
    case 4:if(pressed||expired()){advance();m.write_word(g+0x12,15);m.write_byte(g+0x151,2);}return false;
    case 6:
        if(m.read_byte(g+0x151))return false;
        if(!pressed&&!expired())return false;
        if(!allow_transition)return false;
        for(auto off:{0xcu,0x10u,0x14u})m.write_long(g+off,0);
        for(auto off:{0x10eu,0x10bu,0xb8u,0x301u,0x701u,0xb01u,0xc01u,0x133u})m.write_byte(g+off,0);
        m.write_byte(g+0x8f,1);m.write_byte(g+0x12a,1);return true;
    default:throw std::runtime_error("Invalid original after-round step");
    }
}
} // namespace puzzle_fighter
