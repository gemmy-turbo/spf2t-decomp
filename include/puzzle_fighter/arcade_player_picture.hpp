// SPDX-License-Identifier: GPL-3.0-only
// USA16D5C/16DD4 and9E94: source credit/join/continue prompt tiles.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
template<class M> void draw_arcade_player_prompt(std::uint32_t player,std::uint32_t g,M&m,unsigned selector){
 if(m.read_byte(g+0xbd))selector=0;
 m.write_byte(player+0xa0,std::uint8_t(selector));const auto base=0x84560u;const auto table=base+std::int16_t(m.read_word(base+(m.read_word(g+0x90)?0x16:0)));const auto record=table+(selector&255)*16;
 auto first=m.read_long(record+4)+6,second=m.read_long(record+8);const auto count=m.read_word(second+4);second+=6;const auto side=m.read_byte(player+0x81)?0x1000:0;
 for(unsigned n=0;n<=count;++n,first+=8,second+=8){const auto out=0x900070+side+n*0x80;for(unsigned j=0;j<8;j+=4){m.write_long(out+j,m.read_long(first+j));m.write_long(out+0x2000+j,m.read_long(second+j));}}
}
template<class M> void draw_arcade_continue_digits(std::uint32_t player,std::uint32_t g,M&m){
 if(m.read_byte(g+0xbd))return;
 const unsigned index=m.read_byte(player+0x8c)*2,side=m.read_byte(player+0x81)?0x1000:0;
 for(unsigned page=0;page<2;++page){const auto tile=m.opcode_word((page?0x9f02:0x9eee)+index);const auto out=0x900670u+side+page*0x2000;for(unsigned row=0;row<2;++row){m.write_word(out+row*4,std::uint16_t(tile+row*16));m.write_word(out+row*4+2,0x16);}}
}
} // namespace puzzle_fighter
