// SPDX-License-Identifier: GPL-3.0-only
// USA27BA0: the three tutorial input indicators. Visibility is source owned.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
template<class M,class H> bool source_howto_indicator(std::uint32_t a,std::uint32_t g,M&m,H&h){
 if(m.read_byte(a+2)!=0x32||m.read_byte(a+3)>2)return false;
 const auto variant=m.read_byte(a+3);
 switch(m.read_byte(a+4)){
 case 0:m.write_byte(a+4,2);m.write_word(a+0xc,0);m.write_byte(a+0xb,0);m.write_byte(a+9,0);m.write_byte(a+0xe,0);m.write_word(a+0x30,0xa000);m.write_word(a+0x10,variant==0?0xf0:variant==1?0x128:0x150);m.write_word(a+0x14,0x40);h.init(a,0,0x7dbd4);return true;
 case 2:
  if(!m.read_byte(g-0x738a)){m.write_byte(a+4,4);return true;}
  m.write_byte(a+0xe,m.read_byte(a+0xe)-1);
  if(std::int8_t(m.read_byte(a+0xe))<0){m.write_byte(a+0xe,0);unsigned index;
   if(!variant)index=m.read_word(g+0x39a)&15;
   else{index=m.read_byte(g+0x39a)&(variant==1?1:2);if(variant==2)index>>=1;}
   if(index){m.write_byte(a+0xe,8);}h.init(a,index+(variant==0?0:variant==1?16:18),0x7dbd4);
  }
  h.display(a);return true;
 case 4:case 6:h.free(a);return true;default:return false;
 }
}
} // namespace puzzle_fighter
