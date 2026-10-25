// SPDX-License-Identifier: GPL-3.0-only
// USA2724C..273FA: four lower leaderboard rows, source picture and slide timing.
#pragma once
#include "scene_actor_tasks.hpp"
namespace puzzle_fighter {
template<class M> void source_palette_preserving_brightness(M&m,std::uint32_t from,std::uint32_t to,unsigned blocks=1){
 for(unsigned b=0;b<blocks;++b){const auto bright=m.read_long(to)&0xf000f000u;for(unsigned n=0;n<32;n+=4)m.write_long(to+n,m.read_long(from+n)|bright);from+=32;to+=32;}
}
template<class M,class H> bool source_score_row(std::uint32_t a,std::uint32_t g,M&m,H&h){
 if(m.read_byte(a+2)!=0x2f||m.read_byte(a+3)>3)return false;
 const auto variant=m.read_byte(a+3),phase=m.read_byte(a+4);
 if(!phase){m.write_byte(a+4,2);m.write_word(a+0xc,0);m.write_byte(a+0xb,0);m.write_byte(a+9,0);m.write_word(a+0x10,0x180);m.write_word(a+0x30,0xa000);m.write_long(a+0x20,0xfff80000);m.write_long(a+0x28,0);m.write_word(a+0x14,0x98 - variant*32);m.write_long(a+0x1c,a+0x20);m.write_long(a+0x24,a+0x3c);
  auto out=a+0x3c;auto word=[&](unsigned v){m.write_word(out,std::uint16_t(v));out+=2;};auto lng=[&](std::uint32_t v){m.write_long(out,v);out+=4;};
  word(8);word(20);word(13);lng(0x273fc);lng(m.read_long(0x2744a + variant*8));lng(m.read_long(0x2744e + variant*8));
  const auto row=g+0x57ba + variant*16;const auto character=m.read_byte(row+8);word(m.read_word(0x27434 + character*2));word(0x1100|variant);
  for(int n=2;n>=0;--n){lng(m.read_long(0x171e8 + m.read_byte(row+4+n)*4));}m.write_byte(a+0xa,m.read_byte(row+11));
  const auto offset=std::uint16_t((character*2+m.read_byte(row+9))<<7);source_palette_preserving_brightness(m,std::uint32_t(0xa0000 + std::int16_t(offset)),0x90c000 + variant*32);
  attack_display_detail::bcd(m.read_long(row),g,m);auto digits=m.read_long(g-0x6ca8);for(unsigned n=0;n<7;++n){word(0x190 + (digits&15));word(23);digits>>=4;}
  lng(m.read_long(0x2746a + m.read_byte(a+0xa)*4));return true;
 }
 if(phase==4||phase==6){h.free(a);return true;}if(phase!=2)return false;
 const auto sub=m.read_byte(a+5);auto expired=[&](){m.write_word(a+0xe,m.read_word(a+0xe)-1);return std::int16_t(m.read_word(a+0xe))<0;};auto next=[&](){m.write_byte(a+5,m.read_byte(a+5)+2);};
 if(sub==2||sub==6){m.write_long(a+0x10,m.read_long(a+0x10)+m.read_long(a+0x20));m.write_long(a+0x20,m.read_long(a+0x20)+m.read_long(a+0x28));}
 switch(sub){case 0:if(expired()){next();m.write_word(a+0xe,43);}break;case 2:if(expired()){next();m.write_word(a+0xe,130);m.write_word(a+0x10,24);}break;case 4:if(expired()){next();m.write_word(a+0xe,43);}break;case 6:if(expired())m.write_byte(a+4,4);break;default:return false;}
 h.display(a);return true;
}
// USA2AE02: top-score name and source cycling decimal digits (third pool03).
template<class M> bool source_top_score_text(std::uint32_t a,std::uint32_t g,M&m){
 if(m.read_byte(a+2)!=3){return false;}const auto phase=m.read_byte(a+4);
 if(phase==4||phase==6){free_attack_display(a,g,m);return true;}if(phase!=0&&phase!=2)return false;
 const auto render=[&](std::uint32_t from){for(unsigned i=0;i<7;++i){m.write_word(a+0xb2 + i*4,m.opcode_word(0x2af86 + (m.read_byte(from+i)&15)*2));m.write_word(a+0xb4 + i*4,0x1105);}};
 if(!phase){m.write_byte(a+4,2);m.write_byte(a+1,1);m.write_byte(a+9,0);for(unsigned o:{0x10u,0x14u,0xcu})m.write_word(a+o,0);m.write_word(a+0x30,0xa000);m.write_word(a+14,60);m.write_long(a+0x1c,a+0x80);m.write_long(a+0x86,0x2afb2);m.write_word(a+0x80,2);m.write_word(a+0x82,41);m.write_word(a+0x84,16);
  for(unsigned i=0;i<24;i+=4){m.write_long(a+0x8a + i,m.read_long(0x2af9a + i));}for(unsigned i=0;i<3;++i){m.write_long(a+0xa2 + i*4,m.read_long(0x171e8 + m.read_byte(g+0x57ae + i)*4));}m.write_byte(a+10,m.read_byte(g+0x57b5));m.write_long(a+0xae,m.read_long(0x2746a + m.read_byte(a+10)*4));
  attack_display_detail::bcd(m.read_long(g+0x57aa),g,m);const auto digits=m.read_long(g-0x6ca8);for(unsigned i=0;i<7;++i){const auto d=(digits>>(24-i*4))&15;m.write_byte(a+0x20 + i,d);m.write_byte(a+0x28 + i,d);}render(a+0x20);
 }
 auto first=a+0x20;unsigned count=7;
 switch(m.read_byte(a+5)){
 case 0:m.write_word(a+14,m.read_word(a+14)-1);if(std::int16_t(m.read_word(a+14))<0){m.write_byte(a+5,2);m.write_byte(a+14,0);m.write_byte(a+15,0);}break;
 case 2:m.write_byte(a+14,m.read_byte(a+14)-1);if(std::int8_t(m.read_byte(a+14))<0){m.write_byte(a+14,9);const auto i=m.read_byte(a+15);m.write_byte(a+0x20 + i,m.read_byte(a+0x28 + i));m.write_byte(a+15,i+1);if(m.read_byte(a+15)>=8){m.write_byte(a+5,4);render(a+0x28);return true;}}
  first+=m.read_byte(a+15);count=8-m.read_byte(a+15);break;
 case 4:return true;default:return false;
 }
 for(unsigned i=0;i<count;++i){auto d=unsigned(m.read_byte(first+i))+1;if(d>=10)d=0;m.write_byte(first+i,d);}render(a+0x20);return true;
}
} // namespace puzzle_fighter
