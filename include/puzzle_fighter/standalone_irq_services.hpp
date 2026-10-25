// SPDX-License-Identifier: GPL-3.0-only
// Ordinary inactive cabinet service domain of1550/1614/1646/16EE.
// Credit acceptance/storage and service overlays are explicit outside paths.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
template<class M> bool standalone_coin_poll(std::uint32_t g,M&m){
 if(m.read_byte(g+0x82))return true;
 auto history=[&](unsigned base,unsigned bit){unsigned v=0;for(unsigned n=0;n<4;++n)v=(v<<1)|((m.read_byte(g+base+3-n)>>bit)&1);return v;};
 m.write_byte(g+0x75,history(0x60,4));m.write_byte(g+0x7d,history(0x60,5));m.write_byte(g+0x6c,history(0x64,2));
 const auto timer=m.read_byte(g+0x6f);if(timer){m.write_byte(g+0x6f,timer-1);if(std::uint8_t(timer-1)==0x20)m.write_byte(g+0x6b,m.read_byte(g+0x6b)&0xfe);}
 else if(m.read_byte(g+0x6e)){m.write_byte(g+0x6e,m.read_byte(g+0x6e)-1);m.write_byte(g+0x6b,m.read_byte(g+0x6b)|1);m.write_byte(g+0x6f,0x40);}
 for(unsigned side=0;side<(m.read_byte(g+0x9f)?2u:1u);++side){const auto a=g+0x70+side*8;const auto active=m.read_byte(a);if(!active){if(m.read_byte(a+5)==3){m.write_byte(a,1);m.write_byte(a+4,50);}}else{if(m.read_byte(a+5)==12)return false;const auto left=std::uint8_t(m.read_byte(a+4)-1);m.write_byte(a+4,left);if(!left)m.write_byte(a,0);}}
 return m.read_byte(g+0x9c)||m.read_byte(g+0x6c)!=3;
}
template<class M> bool standalone_coin_requests(std::uint32_t g,M&m){return !m.read_byte(g-0x739a)&&!m.read_byte(g-0x7399);}
} // namespace puzzle_fighter
