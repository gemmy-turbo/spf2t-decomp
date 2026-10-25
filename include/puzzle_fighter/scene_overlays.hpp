// SPDX-License-Identifier: GPL-3.0-only
// USA61CA kinds0/1/2,6634 and6650. Kind3 reuses delivered interlude recovery.
#pragma once
#include "match_interlude.hpp"
#include "arcade_match_flow.hpp"
namespace puzzle_fighter {
template<class M> bool request_scene_overlay(std::uint32_t g,M&m,unsigned kind,std::uint32_t owner){
 if(m.read_byte(g+0x2900)){return false;}m.write_byte(g+0x2900,1);m.write_byte(g+0x2902,std::uint8_t(kind));m.write_byte(owner+0xa,0);return true;
}
template<class M> void finish_scene_fade_screen(std::uint32_t g,M&m){
 m.write_word(g+0x32,0x270e);m.write_word(g+0x3a,0x2641);for(unsigned o:{0x200u,0x240u,0x280u})m.write_byte(g+o,0);m.write_word(g+0x26,0);m.write_word(g+0x28,0x300);
 for(unsigned plane=0;plane<3;++plane)for(unsigned o=0;o<0x4000;o+=4)m.write_long(0x900000+plane*0x4000+o,plane==0?0x00200000:plane==1?0x58200000:0x1cc00000);
 for(unsigned n=0;n<16;++n)m.write_word(0x90c800+n*2,m.read_word(0xa0be0+n*2)|0xf000);
 auto source=(m.read_word(g+0x90)?0x6800u:0x67ecu)+12;
 for(unsigned start:{0x904004u,0x904304u,0x904020u,0x904320u}){auto tile=std::uint16_t(m.read_word(source)+0x5800);source+=2;for(unsigned x=0;x<12;++x)for(unsigned y=0;y<7;++y){const auto at=start+x*0x40+y*4;m.write_word(at,std::uint16_t(tile+x+y*16));m.write_word(at+2,0);}}
}
template<class M,class H> bool tick_scene_overlay(std::uint32_t g,M&m,H&h){
 const auto a=g+0x2900;if(!m.read_byte(a))return true;const auto kind=m.read_byte(a+2),phase=m.read_byte(a+4);if(kind==3)return tick_source_overlay(g,m,h);if(kind>2)return false;
 if(phase==4||phase==6){for(unsigned n=0;n<16;n+=4)m.write_long(a+n,0);m.write_byte(a+8,0x14);return true;}
 if(phase==0){
  m.write_byte(a+4,2);m.write_byte(a+1,1);m.write_byte(a+9,0);m.write_byte(a+0xb,0);m.write_word(a+0x10,0);m.write_word(a+0x14,kind==2?0:0xf0);m.write_word(a+0x30,0xe000);m.write_word(a+0xe,kind==2?0xc0:kind==0?0x1f:0xf);
  m.write_long(a+0x1c,kind==2?0x67b6:m.read_word(g+0x90)?0x6800:0x67ec);
  if(kind==2){m.write_word(a+0xc,0);for(unsigned bank=0;bank<4;++bank)for(unsigned n=0;n<0x400;n+=4){const auto at=0x90c000+bank*0x400+n;m.write_long(at,m.read_long(at)&0x3fff3fff);}m.write_long(a+0x20,0x6708);m.write_long(a+0x24,m.read_long(0x6708));return true;}
  auto coords=kind==0?0x6814u:0x6824u,script=kind==0?0x6834u:0x6874u;
  for(unsigned n=0;n<4;++n){const auto state=a+0x80+n*24;for(unsigned j=0;j<16;j+=4){m.write_long(state+j,m.read_long(script));script+=4;}for(unsigned j=0;j<2;++j){const auto v=m.read_word(coords);coords+=2;m.write_word(a+0x100+n*4+j*2,v);m.write_word(state+16+j*4,v);if(kind==0)m.write_word(state+18+j*4,0);}}return true;
 }
 if(phase!=2){return false;}const auto sub=m.read_byte(a+5);if(sub!=0&&sub!=2)return false;
 auto expired=[&]{const auto left=std::uint16_t(m.read_word(a+0xe)-1);m.write_word(a+0xe,left);return(left&0x8000)!=0;};
 auto endpoints=[&](std::uint32_t source){for(unsigned n=0;n<16;n+=4)m.write_long(a+0x100+n,m.read_long(source+n));};
 auto move=[&]{for(unsigned n=0;n<4;++n){const auto state=a+0x80+n*24;for(unsigned j=0;j<2;++j){const auto velocity=state+j*8,position=state+16+j*4;m.write_long(position,m.read_long(position)+m.read_long(velocity));m.write_long(velocity,m.read_long(velocity)+m.read_long(velocity+4));m.write_word(a+0x100+n*4+j*2,m.read_word(position));}}};
 if(kind==2){if(!sub){if(expired())m.write_byte(a+5,2);match_interlude_detail::palette_step(a+0x20,m);}else if(expired()){m.write_byte(a+4,4);m.write_byte(a+1,0);m.write_byte(g+0x8a,0);}h.step(a);return true;}
 if(kind==0){if(!sub){if(arcade_flow_detail::pressed(g,m,true)||expired()){m.write_byte(a+5,2);m.write_word(a+0xe,15);m.write_byte(a+0xa,0xff);endpoints(0x6824);}else move();}else if(expired()){m.write_byte(a+4,4);m.write_byte(a+1,0);m.write_byte(g+0x8c,0);}}
 else if(!sub){if(expired()){m.write_byte(a+5,2);m.write_word(a+0xe,31);}}
 else if(arcade_flow_detail::pressed(g,m,true)||expired()){m.write_byte(a+4,4);m.write_byte(a+1,0);m.write_byte(g+0x8c,0);endpoints(0x6814);}else move();
 return true;
}
} // namespace puzzle_fighter
