// SPDX-License-Identifier: GPL-3.0-only
// Original USA first-pool character attachments/particles 242CE,24490,24B42,
// 26388 and second-pool attachments29FC8..2A405, including their26D36 child.
#pragma once
#include "character_controller.hpp"
namespace puzzle_fighter {
template<class M,class H> bool character_cosmetic_task(std::uint32_t a,std::uint32_t g,M&m,H&h){
 const auto sw=[](std::uint16_t v){return v&0x8000?int(v)-65536:int(v);};
 const auto p=std::uint32_t(sw(m.read_word(a+0x32)));
 const auto kind=m.read_byte(a+2),state=m.read_byte(a+4);
 if(kind!=0xb&&kind!=0xd&&kind!=0x13&&kind!=0x23&&kind!=0x2b)return false;
 auto next=[&]{m.write_byte(a+4,m.read_byte(a+4)+2);};
 auto copy=[&](unsigned o){m.write_word(a+o,m.read_word(p+o));};
 auto face=[&]{m.write_byte(a+0xb,m.read_byte(p+0xb));};
 auto animation=[&]{h.step(a);h.display(a);};
 auto random=[&]{const auto r=global_random(m.read_word(g+0xae));m.write_word(g+0xae,r.seed);return r.value;};
 if(state==4){next();return true;}if(state==6){h.free(a);return true;}if(state!=0&&state!=2)return false;
 if(kind==0xb){
  if(!state){next();copy(0x30);m.write_byte(a+3,m.read_byte(p+0x82));copy(0xc);m.write_byte(a+0x3a,1);m.write_byte(a+0xa,0);const auto ch=m.read_byte(a+3);m.write_byte(a+9,m.opcode_byte(0x2437c+ch));m.write_long(a+0x20,(std::uint32_t(m.opcode_word(0x24398+ch*4))<<16)|m.opcode_word(0x2439a+ch*4));}
  if(m.read_byte(g+0x12a))next();
  if(m.read_byte(p+0x82)!=m.read_byte(a+3)){next();return true;}
  const auto index=m.read_byte(m.read_long(p+0x1c)+0xa);
  if(!index){m.write_byte(a+0xa,0);return true;}
  copy(0x10);copy(0x14);face();if(index!=m.read_byte(a+0xa)){m.write_byte(a+0xa,index);h.init(a,index,m.read_long(a+0x20));}animation();return true;
 }
 if(kind==0xd){
  if(!state){
   next();face();copy(0x30);copy(0xc);const auto index=m.read_byte(a+3);
   auto dx=m.opcode_word(0x2451a+index*4);if(m.read_byte(a+0xb))dx=std::uint16_t(-dx);
   m.write_word(a+0x10,m.read_word(p+0x10)+dx);m.write_word(a+0x14,m.read_word(p+0x14)+m.opcode_word(0x2451c+index*4));m.write_byte(a+9,4);m.write_byte(a+0x3a,1);h.init(a,m.opcode_byte(0x244e6+index),0x54ce4);return true;
  }
  if(m.read_byte(g+0x12a)){next();return true;}
  const auto subtype=m.read_byte(a+3);const auto signed_subtype=subtype&0x80?int(subtype)-256:int(subtype);
  if(signed_subtype>1){if(m.read_byte(p+0x1b)){next();return true;}}
  else {if(m.read_word(p+4)!=0x200){next();return true;}if(m.read_byte(a+0x1a))next();}
  animation();return true;
 }
 if(kind==0x13){
  if(!state){next();copy(0x30);m.write_word(a+0xc,0);m.write_byte(a+9,2);m.write_byte(a+0x3a,1);face();auto dx=random()&31;if(!m.read_byte(a+0xb))dx=std::uint16_t(-dx);m.write_word(a+0x10,m.read_word(p+0x10)+dx);m.write_word(a+0x14,m.read_word(p+0x14)+(random()&31)+16);h.init(a,1,0x81766);return true;}
  if(m.read_byte(g+0x12a)){next();return true;}
  if(!m.read_byte(g+0x127)){if(m.read_byte(a+0x1a)){next();return true;}h.step(a);}h.display(a);return true;
 }
 if(kind==0x23){
  if(!state){next();face();copy(0x30);m.write_word(a+0xc,0);m.write_byte(a+9,4);m.write_byte(a+0x3a,1);copy(0x10);copy(0x14);const auto index=m.read_byte(p+0x82)*8+m.read_byte(a+0xe)*4;auto dx=m.opcode_word(0x2640eu+index);if(m.read_byte(a+0xb))dx=std::uint16_t(-dx);m.write_word(a+0x10,m.read_word(a+0x10)+dx);m.write_word(a+0x14,m.read_word(a+0x14)+m.opcode_word(0x26410+index));h.init(a,m.read_byte(a+3)?10:9,0x81766);return true;}
  if(m.read_byte(g+0x12a)||(m.read_byte(a+0x1a)&0x80)){next();return true;}animation();return true;
 }
 if(!state){next();copy(0x30);copy(0xc);face();m.write_byte(a+9,4);m.write_byte(a+0x3a,1);copy(0x10);m.write_word(a+0x14,m.read_word(p+0xd6));const auto subtype=m.read_byte(a+3);if(subtype){auto dx=m.opcode_word(0x26db0+subtype*2);if(m.read_byte(a+0xb))dx=std::uint16_t(-dx);m.write_word(a+0x10,m.read_word(a+0x10)+dx);}h.init(a,subtype?3:4,0x725f8);h.display(a);return true;}
 if(m.read_byte(a+0x1a)&0x80){next();return true;}animation();return true;
}

// These actors retain display mode12 and expose byte+1/script+1C to the source
// second-pool renderer. They never enter the first-pool display list.
template<class M,class H> bool character_attachment_task(std::uint32_t a,std::uint32_t g,M&m,H&h){
 const auto ptr=[](std::uint16_t v){return std::uint32_t(v&0x8000?int(v)-65536:int(v));};
 const auto p=ptr(m.read_word(a+0x32));const auto kind=m.read_byte(a+2),state=m.read_byte(a+4);
 if(kind>3)return false;
 auto next=[&]{m.write_byte(a+4,m.read_byte(a+4)+2);};
 auto copy=[&](unsigned o){m.write_word(a+o,m.read_word(p+o));};
 auto face=[&]{m.write_byte(a+0xb,m.read_byte(p+0xb));};
 if(state==4){next();return true;}
 if(state==6){const auto backlink=(kind==0||kind==3)?0x38u:0x36u;if(m.read_word(p+backlink)==std::uint16_t(a))m.write_word(p+backlink,0);free_character_second_pool(a,g,m);return true;}
 if(state!=0&&state!=2)return false;
 if(!state){
  next();copy(0x30);copy(0xc);
  if(kind==1){face();m.write_word(a+0x10,m.read_byte(a+0xb)?0x80:0x100);m.write_word(a+0x14,0x54);m.write_byte(a+9,4);}
  else {m.write_byte(a+3,m.read_byte(p+0x82));m.write_byte(a+0xa,0);if(kind>=2)m.write_byte(a+9,6);if(kind==2){m.write_word(a+0x10,m.read_byte(p+0x81)?0x100:0x80);copy(0x14);}}
 }
 if(kind==1){
  m.write_byte(a+1,m.read_byte(p+1));if(m.read_byte(p+0x82)!=5){next();return true;}
  const auto parent_phase=m.read_byte(p+5),phase=parent_phase==0xa?2:parent_phase;
  if((parent_phase==0||parent_phase==2||parent_phase==4||parent_phase==6||parent_phase==8||parent_phase==0xa)&&phase!=m.read_byte(a+5))m.write_long(a+4,0x02000000u|(std::uint32_t(phase)<<16));
  const auto mode=m.read_byte(a+5);if(mode>8||(mode&1))return false;
  if(!mode&&m.read_byte(g+0x12a)){next();return true;}
  const auto sub=m.read_byte(a+6);if(!sub){m.write_byte(a+6,2);const unsigned index=mode==6?4:mode==8?3:mode/2;h.init(a,index,0x674ba);return true;}
  if(sub!=2)return false;
  h.step(a);return true;
 }
 if(kind==0){if(m.read_byte(g+0x12a)){next();return true;}m.write_byte(a+1,0);if(m.read_byte(p+0x82)!=m.read_byte(a+3)){next();return true;}}
 else {m.write_byte(a+1,0);if(m.read_byte(g+0x12a)||m.read_byte(p+0x82)!=(kind==2?4:5)){next();return true;}}
 const auto index=m.read_byte(m.read_long(p+0x1c)+0xa);
 if(!index){m.write_byte(a+0xa,0);return true;}
 if(kind==0){copy(0x10);copy(0x14);}
 if(kind==3){if(index==0x18){const auto other=ptr(m.read_word(p+0x34));m.write_word(a+0x10,m.read_word(other+0x10));m.write_word(a+0x14,m.read_word(other+0x14)+0x26);}else{copy(0x10);copy(0x14);}}
 face();m.write_byte(a+1,1);
 if(index!=m.read_byte(a+0xa)){m.write_byte(a+0xa,index);h.init(a,index,kind==0?0x725f8:kind==2?0x61850:0x674d4);if(kind==0)return true;}
 h.step(a);
 if(kind==0&&m.read_byte(a+0x1b)&&m.read_byte(g+0x581c)){
  m.write_byte(g+0x581c,m.read_byte(g+0x581c)-1);const auto stack=ptr(m.read_word(g+0x580a)),child=ptr(m.read_word(stack));m.write_word(g+0x580a,std::uint16_t(stack+2));
  m.write_byte(child,m.read_byte(child)+1);m.write_byte(child+2,0x2b);m.write_byte(child+3,m.read_byte(a+0x1b));m.write_word(child+0x32,m.read_word(a+0x32));m.write_byte(a+0x1b,0);
 }
 return true;
}
} // namespace puzzle_fighter
