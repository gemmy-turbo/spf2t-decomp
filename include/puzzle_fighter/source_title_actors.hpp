// SPDX-License-Identifier: GPL-3.0-only
// USA26FCE: finite title logo actors, first-pool kind2E, subtypes0..9.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
template<class M,class H> bool source_title_actor(std::uint32_t a,std::uint32_t g,M&m,H&h){
 if(m.read_byte(a+2)!=0x2e||m.read_byte(a+3)>9)return false;
 const unsigned variant=m.read_byte(a+3),state=m.read_byte(a+4);
 auto b=[&](unsigned o,unsigned v){m.write_byte(a+o,v);};
 auto w=[&](unsigned o,unsigned v){m.write_word(a+o,v);};
 auto l=[&](unsigned o,std::uint32_t v){m.write_long(a+o,v);};
 auto signal=[&]{m.write_byte(g-0x6ccc,255);};
 auto move=[&]{l(0x10,m.read_long(a+0x10)+m.read_long(a+0x20));l(0x20,m.read_long(a+0x20)+m.read_long(a+0x28));l(0x14,m.read_long(a+0x14)+m.read_long(a+0x24));l(0x24,m.read_long(a+0x24)+m.read_long(a+0x2c));};
 auto expired=[&]{b(0xe,m.read_byte(a+0xe)-1);return std::int8_t(m.read_byte(a+0xe))<0;};
 if(state==0){b(4,2);w(0xc,0);b(0xb,0);w(0x30,0xa000);w(0x10,0xc0);w(0x14,0x80);constexpr unsigned layers[]={6,4,0,2,0};b(9,layers[variant%5]);h.init(a,0x22+variant%5+(m.read_word(g+0x90)?5:0),0x7da5e);return true;}
 if(state==4||state==6){h.free(a);return true;}if(state!=2)return false;
 const unsigned sub=m.read_byte(a+5);
 if(variant==3||variant==8){
  if(variant==8&&std::int16_t(m.read_word(a+0x1a))<0)signal();
  if(m.read_word(g+0x90))switch(sub){
   case 0:if(m.read_byte(g-0x6ccc)){b(5,2);b(0xe,15);w(0x10,0x180);w(0x14,0x40);l(0x20,0xfff40000);l(0x28,0);l(0x24,0);l(0x2c,0);}break;
   case 2:move();if(expired()){b(5,4);signal();w(0x10,0xc0);}break;
   case 4:break;default:return false;
  }
  h.step(a);h.display(a);return true;
 }
 if(variant==5){switch(sub){
  case 0:if(m.read_byte(g-0x6ccc)){b(5,2);b(0xe,0x30);w(0x10,0xc0);w(0x14,0xffc0);l(0x20,0);l(0x28,0);l(0x24,0x100000);l(0x2c,0xffff8000);}return true;
  case 2:move();if(expired()){b(5,4);signal();w(0x14,0x80);}break;
  case 4:break;default:return false;
 }h.display(a);return true;}
 if(variant==6||variant==7){switch(sub){
  case 0:if(m.read_byte(g-0x6ccc)){b(5,2);b(0xe,0x2c);w(0x10,0x200);w(0x14,0x120);l(0x20,0xffe00000);l(0x28,0x12000);l(0x24,0xfff00000);l(0x2c,0x9000);}return true;
  case 2:move();if(expired()){b(5,4);b(0xe,0x11);l(0x28,0u-2*m.read_long(a+0x28));l(0x2c,0u-2*m.read_long(a+0x2c));}break;
  case 4:move();if(expired()){b(5,6);signal();w(0x10,0xc0);w(0x14,0x80);}break;
  case 6:break;default:return false;
 }h.display(a);return true;}
 h.step(a);h.display(a);return true;
}
} // namespace puzzle_fighter
