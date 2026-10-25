// SPDX-License-Identifier: GPL-3.0-only
// USA2630,2AD30,54D8: idle credit marker, its original blink and credit totals.
#pragma once
#include "source_ending.hpp"
#include "scene_actor_tasks.hpp"
namespace puzzle_fighter {
template<class M> void allocate_source_idle_credit_marker(std::uint32_t g,M&m){
 if(!m.read_byte(g+0x83)){return;}const auto a=allocate_scene_third(g,m);if(!a)return;m.write_byte(a,m.read_byte(a)+1);m.write_byte(a+2,2);unsigned index=12;if(!m.read_byte(g+0x9c)){index=13;if(!m.read_byte(g+0x9d))index+=m.read_byte(g+0x72);}m.write_byte(a+3,index);
}
template<class M> bool source_idle_credit_marker(std::uint32_t a,std::uint32_t g,M&m){
 if(m.read_byte(a+2)!=2){return false;}const auto phase=m.read_byte(a+4);if(phase==4||phase==6){free_attack_display(a,g,m);return true;}if(phase!=0&&phase!=2)return false;
 if(!phase){m.write_byte(a+4,2);m.write_byte(a+1,1);m.write_byte(a+9,0);m.write_word(a+0x10,0);m.write_word(a+0x14,0x108);m.write_byte(a+14,16);m.write_byte(a+15,1);m.write_word(a+0x30,0xe000);m.write_long(a+0x1c,a+0x80);m.write_word(a+0x80,4);auto from=std::uint32_t(0x1f88e + std::int16_t(m.read_word(0x1f88e + m.read_byte(a+3)*2)));auto out=a+0x86;unsigned count=0;
  bool done=false;for(unsigned budget=0;!done&&budget<4096;){auto xy=std::uint32_t(m.read_byte(from++))<<19;xy|=unsigned(m.read_byte(from++))*2;xy&=0x03ff03ff;from++;
   for(;;){if(++budget>=4096)throw std::runtime_error("Source credit marker text budget");const auto glyph=m.read_byte(from++);if(!glyph){done=true;break;}if(glyph==127)break;if(glyph!=32){m.write_word(out,glyph);m.write_word(out+2,20);m.write_long(out+4,xy);out+=8;++count;}xy+=0x80000;}
  }m.write_word(a+0x82,count);m.write_word(a+0x84,count-1);
 }
 m.write_byte(a+14,m.read_byte(a+14)-1);if(!m.read_byte(a+14)){m.write_byte(a+14,16);m.write_byte(a+15,m.read_byte(a+15)^1);m.write_byte(a+1,m.read_byte(a+1)^1);}return true;
}
template<class M> void draw_source_credit_totals(std::uint32_t g,M&m){
 const bool separate=m.read_byte(g+0x9e)!=0;for(int side=separate?1:0;side>=0;--side){const auto c=g+0x70 + unsigned(side)*8;unsigned index=separate?0x1a:0x17;if(!m.read_byte(g+0x9d)){index+=separate?2:1;if(m.read_byte(c+2)>1)index+=separate?2:1;}if(separate)index+=unsigned(side);auto out=draw_source_notice(m,index);m.write_word(out,m.read_byte(c+6));m.write_word(out+2,31);out+=128;
  if(m.read_byte(g+0x9c)||m.read_byte(g+0x9d)||m.read_byte(c+2)<=1){continue;}m.write_long(out,0x0028001f);m.write_word(out+128,m.read_byte(c+1));m.write_word(out+130,31);m.write_long(out+256,0x002f001f);m.write_word(out+384,m.read_byte(c+2));m.write_word(out+386,31);m.write_long(out+512,0x0029001f);
 }
}
template<class M> void fill_source_large_tile_page(M&m,unsigned tile,unsigned attr,std::uint32_t to){const auto value=std::uint32_t((((tile+1)&15)+0x1cc0)<<16)|(attr&31);for(unsigned n=0;n<96;++n)m.write_long(to+n*4,value);}
} // namespace puzzle_fighter
