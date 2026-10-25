// SPDX-License-Identifier: GPL-3.0-only
// USA13D0..149E input/scroll-latch tail of IRQ13A2; main2CE9E..2CEE2.
#pragma once
#include <array>
#include <cstdint>
namespace puzzle_fighter {
struct InputPorts {
    // Active-low CPS2 input words; byte ordering follows the original bus.
    std::uint16_t players=0xffff,extra=0xffff,system=0xffff,status=0xffff;
};
struct InputIoWrite {std::uint32_t address;std::uint16_t value;};
struct InputPollResult {std::array<InputIoWrite,8> writes{};unsigned count=0;};
// Host device adapter: original logical direction/button bits and bit15 Start.
// No SOCD cleaning, debounce or repeat timing is invented here.
inline InputPorts input_ports_from_words(std::array<std::uint16_t,2> words){
    InputPorts p;const auto low=[](unsigned w){return (w&15u)|((w>>4)&0xf0u);};
    p.players=std::uint16_t(~(low(words[0])|(low(words[1])<<8)));
    const auto extras=((words[0]>>12)&7u)|((words[1]>>8)&0x30u);
    p.extra=std::uint16_t(~extras);
    const auto system=(words[0]&0x8000?1u:0u)|(words[1]&0x8000?2u:0u)|((words[1]>>8)&0x40u);
    p.system=std::uint16_t(~(system<<8));return p;
}
template<class M> InputPollResult sample_input_ports(std::uint32_t g,M&m,InputPorts p){
    InputPollResult out;auto write=[&](std::uint32_t at,std::uint16_t value){out.writes[out.count++]={at,value};};
    if(!m.read_byte(g+0x87))write(0x804040,std::uint16_t((m.read_word(g+0x6a)&0x7f33)|m.read_byte(g+0x88)|m.read_byte(g+0xd1)));
    m.write_word(g+0x68,p.status);
    m.write_long(g+0x60,m.read_long(g+0x60)>>8);m.write_long(g+0x64,m.read_long(g+0x64)>>8);
    const auto starts=std::uint8_t(~(p.system>>8));m.write_byte(g+0x60,starts);m.write_byte(g+0x64,std::uint8_t(~p.system));
    m.write_word(g+0x5a,m.read_word(g+0x58));m.write_word(g+0x5e,m.read_word(g+0x5c));
    const auto extra=std::uint8_t(~p.extra),p1=std::uint8_t(~p.players),p2=std::uint8_t(~(p.players>>8));
    m.write_byte(g+0x59,p1&15);m.write_byte(g+0x58,(p1>>4)|((extra<<4)&0x70));
    m.write_byte(g+0x5d,p2&15);m.write_byte(g+0x5c,(p2>>4)|(extra&0x30)|(starts&0x40));
    m.write_byte(g+0x57a9,m.read_byte(g+0x57a8));m.write_byte(g+0x57a8,p.extra>>8);
    //14A0's scroll shadow/output writes are part of the same source tail.
    for(const auto pair:{std::array<unsigned,3>{0x4a,0x22,0x80010c},{0x4e,0x26,0x800110},{0x52,0x2a,0x800114}}){write(pair[2],m.read_word(g+pair[0]));m.write_word(g+pair[0],std::uint16_t(m.read_word(g+pair[1])-0x40));}
    for(const auto pair:{std::array<unsigned,4>{0x4c,0x24,0x80010e,0x100},{0x50,0x28,0x800112,0x300},{0x54,0x2c,0x800116,0x700},{0x56,0x2e,0x800120,0x300}}){write(pair[2],m.read_word(g+pair[0]));m.write_word(g+pair[0],std::uint16_t(pair[3]-m.read_word(g+pair[1])));}
    m.write_long(g+0x57a4,0);return out;
}
template<class M> void copy_player_inputs(std::uint32_t g,M&m){
    m.write_word(g+0x39c,m.read_word(g+0x39a));m.write_word(g+0x79c,m.read_word(g+0x79a));
    const auto starts=m.read_byte(g+0x60);
    m.write_word(g+0x39a,std::uint16_t(m.read_word(g+0x58)|((starts&1)?0x8000:0)));
    m.write_word(g+0x79a,std::uint16_t(m.read_word(g+0x5c)|((starts&2)?0x8000:0)));
    m.write_word(g+0xb9a,m.read_word(g+0x39a));m.write_word(g+0xc9a,m.read_word(g+0x79a));
    m.write_word(g+0xb9c,m.read_word(g+0x39c));m.write_word(g+0xc9c,m.read_word(g+0x79c));
}
} // namespace puzzle_fighter
