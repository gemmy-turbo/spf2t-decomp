// SPDX-License-Identifier: GPL-3.0-only
// Original USA finite normal-character auxiliary closure. See special-013 inventory.
#pragma once
#include "character_controller.hpp"
namespace puzzle_fighter {
template<class M,class H> struct CharacterSpecialTask {
 M&m;H&h;std::uint32_t g,a,p;
 static int sw(std::uint16_t v){return v&0x8000?int(v)-65536:int(v);}
 static std::uint32_t ptr(std::uint16_t v){return std::uint32_t(sw(v));}
 unsigned b(unsigned o){return m.read_byte(a+o);}unsigned w(unsigned o){return m.read_word(a+o);}std::uint32_t l(unsigned o){return m.read_long(a+o);}
 void byte(unsigned o,unsigned v){m.write_byte(a+o,std::uint8_t(v));}void word(unsigned o,unsigned v){m.write_word(a+o,std::uint16_t(v));}void lng(unsigned o,std::uint32_t v){m.write_long(a+o,v);}
 void next(unsigned o=4){byte(o,b(o)+2);}bool dec(unsigned o){byte(o,b(o)-1);return !b(o);}
 void copy(unsigned o){word(o,m.read_word(p+o));}void face(){byte(0xb,m.read_byte(p+0xb));}
 void coordinates(){copy(0x10);copy(0x14);}void attributes(){copy(0x30);copy(0xc);}
 std::uint32_t cl(std::uint32_t at){return(std::uint32_t(m.opcode_word(at))<<16)|m.opcode_word(at+2);}
 unsigned random(){const auto r=global_random(m.read_word(g+0xae));m.write_word(g+0xae,r.seed);return r.value;}
 void init(unsigned index,std::uint32_t table){h.init(a,index,table);}void render(){h.step(a);h.display(a);}
 void move(){lng(0x10,l(0x10)+l(0x20));lng(0x20,l(0x20)+l(0x28));lng(0x14,l(0x14)+l(0x24));lng(0x24,l(0x24)+l(0x2c));}
 void sound(std::uint32_t table){h.sound(m.opcode_word(table+2*(m.read_byte(p+0x81)&1)),(m.read_byte(p+0x81)?20u:12u)<<8,0);}
 void alternate_sound(unsigned index){if(!m.read_byte(g+0xbc))h.sound(m.opcode_word(0x2ce60+index*2),0x1000,0);}
 bool endflag(){return m.read_byte(g+0x12a)!=0;}
 void end16(){next();for(unsigned o:{0x36u,0x38u})if(w(o))m.write_byte(ptr(w(o))+4,4);}
 void spawn_attachment(unsigned variant,unsigned back){
  if(!m.read_byte(g+0x581d))return;
  m.write_byte(g+0x581d,m.read_byte(g+0x581d)-1);const auto stack=ptr(m.read_word(g+0x580c)),child=ptr(m.read_word(stack));m.write_word(g+0x580c,std::uint16_t(stack+2));
  m.write_byte(child,m.read_byte(child)+1);m.write_word(child+2,0x400+variant);m.write_word(child+0x32,std::uint16_t(a));word(back,child);
 }
 bool tick(){
  const auto kind=b(2),state=b(4),variant=b(3);
  switch(kind){case 8:case 9:case 0xe:case 0xf:case 0x10:case 0x11:case 0x12:case 0x14:case 0x16:case 0x20:case 0x26:case 0x29:case 0x2c:case 0x35:case 0x3b:case 0x40:break;default:return false;}
  if(state==6){h.free(a);return true;}
  if(state==4){if(kind==8||kind==9)h.free(a);else if(kind==0x10){next();h.free(a);}else if(kind==0x16)end16();else next();return true;}
  if(state!=0&&state!=2)return false;
  if(kind==8){
   if(!state){next();face();attributes();coordinates();byte(0x3a,1);init(10,0x6c2d0);return true;}
   if(endflag())next();
   if(!m.read_byte(g+0x127)){if(b(0x1a))next();h.step(a);}h.display(a);return true;
  }
  if(kind==9){if(!state){next();coordinates();attributes();face();byte(9,4);init(0,0x61826);return true;}if(b(0x1a)&0x80)h.free(a);else render();return true;}
  if(kind==0xe){
   if(!state){next();coordinates();attributes();face();byte(9,4);byte(0xe,0x20);const auto offset=variant>>4;auto vx=cl(0x246c0+offset),ax=cl(0x246c8+offset);if(b(0xb)){vx=0u-vx;ax=0u-ax;}lng(0x20,vx);lng(0x28,ax);lng(0x24,cl(0x246c4+offset));lng(0x2c,cl(0x246cc+offset));if(!variant){word(0x10,w(0x10)+(b(0xb)?-45:45));word(0x14,w(0x14)+64);}init(9+variant,0x674ba);return true;}
   if(endflag()||dec(0xe)){next();return true;}move();render();return true;
  }
  if(kind==0xf){
   if(!state){next();copy(0x30);face();word(0xc,0);byte(9,4);byte(0x3a,1);if(b(0x28))alternate_sound(0x1c);else sound(0x2c850);init(5,0x81766);return true;}
   if(endflag()||(b(0x1a)&0x80)){next();return true;}render();return true;
  }
  if(kind==0x10){
   if(!state){next();face();attributes();byte(9,2);const auto other=ptr(m.read_word(p+0x34));word(0x10,m.read_word(other+0x10));word(0x14,m.read_word(other+0x14));word(0x20,w(0x14));word(0x14,w(0x14)+0x90);byte(0x3a,1);byte(3,random()&1);lng(0x24,0xfff60000);init(5+b(3),0x674ba);return true;}
   if(endflag())next();
   if(!m.read_byte(g+0x127))switch(b(5)){
    case 0:lng(0x14,l(0x14)+l(0x24));if(sw(w(0x20))>sw(w(0x14))){next(5);sound(0x2c9a0);m.write_byte(g+0x124,0x30);word(0x14,w(0x20));byte(0xe,0x20);init(7+b(3),0x674ba);}break;
    case 2:if(dec(0xe)){next(5);lng(0x24,0x90000);init(5+b(3),0x674ba);}break;
    case 4:lng(0x14,l(0x14)+l(0x24));if(sw(std::uint16_t(w(0x20)+0x90))<sw(w(0x14)))next();break;
    default:return false;
   }
   h.display(a);return true;
  }
  if(kind==0x11){if(!state){next();coordinates();attributes();byte(9,6);byte(0xb,0);byte(0x3a,1);init(4,0x77962);return true;}if(endflag()||(b(0x1a)&0x80)){next();return true;}render();return true;}
  if(kind==0x12){
   if(!state){
    next();if(b(0x28))alternate_sound(0x1d);else sound(0x2c7fc);face();attributes();byte(9,2);byte(0x3a,1);coordinates();const auto offset=variant*4;auto dx=m.opcode_word(0x24a96+offset);auto vx=cl(0x24aba+offset);if(b(0xb)){dx=std::uint16_t(-dx);vx=0u-vx;}word(0x10,w(0x10)+dx);lng(0x20,vx);word(0x14,w(0x14)+m.opcode_word(0x24a98+offset));byte(0xa,m.opcode_byte(0x24a32+variant*2));byte(0xe,m.opcode_byte(0x24a33+variant*2));lng(0x24,cl(0x24a44+offset));init(b(0xa),l(0x24));return true;
   }
   if(endflag()){next();return true;}
   if(!b(5)){lng(0x10,l(0x10)+l(0x20));if(dec(0xe)){next(5);init(std::uint8_t(b(0xa)+1),l(0x24));return true;}}
   else if(b(5)==2){if(b(0x1a)){next();return true;}}else return false;
   render();return true;
  }
  if(kind==0x14){
   if(!state){next();face();attributes();byte(9,4);byte(0x3a,1);coordinates();word(0x10,w(0x10)+(b(0xb)?-14:14));word(0x14,w(0x14)+33);init(8+variant,0x6e9fc);return true;}
   if((!b(0xa)&&(endflag()||m.read_byte(p+5)!=10))||m.read_byte(p+0x1b)){next();return true;}
   if(m.read_byte(p+0x1a)){word(0x10,m.read_word(p+0x10)+(b(0xb)?10:-10));word(0x14,m.read_word(p+0x14)+26);}render();return true;
  }
  if(kind==0x16){
   if(!state){next();attributes();byte(9,4);byte(0x3a,1);byte(0xa,m.opcode_byte(0x24e30+(variant?0:m.read_byte(p+0xd0)/2)));word(0x20,16);coordinates();word(0x24,m.read_word(p+0x14));word(0x10,w(0x10)+(m.read_byte(p+0xb)?50:-50));word(0x14,w(0x14)+8);init(7+m.read_byte(p+0xb),0x61826);spawn_attachment(0,0x36);spawn_attachment(1,0x38);sound(0x2c976);return true;}
   if(!variant&&endflag()){end16();return true;}
   if(!m.read_byte(g+0x127)){
    if(!b(5)){h.step(a);if(b(0x1a)){next(5);init(9+m.read_byte(p+0xb),0x61826);}else if(b(0x1b)){byte(0x1b,0);word(0x14,w(0x14)+16);}}
    else if(b(5)==2){h.step(a);if(b(0x1a)&0x80){if(dec(0xa))end16();else{byte(5,0);word(0x10,w(0x10)+(m.read_byte(p+0xb)?24:-24));init(7+m.read_byte(p+0xb),0x61826);sound(0x2c976);}}else if(b(0x1b)){byte(0x1b,0);word(0x14,w(0x14)-16);}}
    else return false;
   }
   h.display(a);return true;
  }
  if(kind==0x20){
   if(!state){next();coordinates();attributes();byte(9,4);byte(0x3a,1);if(variant){word(0x10,w(0x10)+(m.read_byte(p+0xb)?88:-88));word(0x14,w(0x14)+0x90);word(0x50,88);}else{word(0x10,m.read_word(ptr(m.read_word(p+0x34))+0x10));word(0x14,m.read_word(p+0x14)+0x90);word(0x50,m.read_word(p+0xd6));}byte(0xf,3);const auto r=random();int dx=r&31;if(r&32)dx=-dx;word(0x10,w(0x10)+dx);byte(0xe,(r&31)+1);byte(0xb,b(0xe)&1);lng(0x20,0);lng(0x28,0);lng(0x24,0xfff80000);lng(0x2c,0xffffe000);init(6,0x61826);return true;}
   if(!variant&&endflag()){next();return true;}
   if(!b(5)){if(dec(0xe))next(5);return true;}
   if(b(5)!=2)return false;
   move();if(sw(w(0x50))>sw(w(0x14))){if(dec(0xf)){next();return true;}word(0x14,w(0x50));if(variant)alternate_sound(0x1e);else sound(0x2c9a0);lng(0x20,b(0xb)?0xfffe0000:0x20000);lng(0x24,0x20000);lng(0x2c,0xffffe000);}render();return true;
  }
  if(kind==0x26){
   if(!state){next();copy(0xc);face();copy(0x30);byte(0x3a,1);if(variant==1)byte(0xb,b(0xb)^1);byte(0xe,32);const auto other=ptr(m.read_word(p+0x34));word(0x10,m.read_word(other+0x10));word(0x14,m.read_word(other+0xd6));word(0x50,w(0x14));auto dx=m.opcode_word(0x266aeu+variant*4);if(b(0xb))dx=std::uint16_t(-dx);word(0x10,w(0x10)+dx);word(0x14,w(0x14)+m.opcode_word(0x266b0+variant*4));auto vx=cl(0x266beu+variant*16),ax=cl(0x266c2+variant*16);if(b(0xb)){vx=0u-vx;ax=0u-ax;}lng(0x20,vx);lng(0x28,ax);lng(0x24,cl(0x266c6+variant*16));lng(0x2c,cl(0x266ca+variant*16));init(variant,0x6c2d0);return true;}
   if(endflag()){next();return true;}
   auto bounce=[&]{move();if(variant!=2&&sw(w(0x50))>sw(w(0x14))){word(0x14,w(0x50));lng(0x24,0x10000);lng(0x2c,0xffffe000);}};
   switch(b(5)){
    case 0:if(dec(0xe)){next(5);word(0x14,w(0x50));byte(0xe,48);init(4+variant,0x6c2d0);}else bounce();break;
    case 2:if(dec(0xe)){next(5);byte(0xe,32);byte(0xb,b(0xb)^1);lng(0x20,0u-l(0x20));if(variant==2)lng(0x24,0u-l(0x24));else{lng(0x24,0x10000);lng(0x2c,0xffffe000);}init(variant,0x6c2d0);}break;
    case 4:if(dec(0xe))next();else bounce();break;
    default:return false;
   }
   render();return true;
  }
  if(kind==0x29){
   if(!state){next();copy(0x30);byte(9,4);byte(0x3a,1);init((random()&1)+8,0x6c2d0);const auto r=random()&7;word(0x18,r+1);byte(0xb,r&1);const auto other=ptr(m.read_word(p+0x34));word(0x10,m.read_word(other+0x10));word(0x14,m.read_word(other+0xd6));auto dx=m.opcode_word(0x26a6a+variant*4);if(m.read_byte(other+0xb))dx=std::uint16_t(-dx);word(0x10,w(0x10)+dx);word(0x14,w(0x14)+m.opcode_word(0x26a6c+variant*4));return true;}
   if((m.read_byte(p+7)!=2&&m.read_byte(p+7)!=4)||endflag()){next();return true;}render();return true;
  }
  if(kind==0x2c||kind==0x35){
   if(!state){next();face();copy(0x30);byte(0xc,0xff);byte(0xd,m.read_byte(p+0xd));lng(0x1c,m.read_long(p+0x1c));byte(9,kind==0x2c?6:8);byte(0x3a,kind==0x2c?1:0);byte(0xe,20);coordinates();auto index=m.read_byte(p+0x82);if(kind==0x35&&variant)index=variant==2?7:3;byte(0xd,b(0xd)+m.opcode_byte((kind==0x2c?0x26e60u:0x2810au)+index));}
   if((kind==0x2c||!variant)&&(endflag()||m.read_byte(p+5)!=(kind==0x2c?10:8))){next();return true;}
   if(dec(0xe)){next();return true;}h.display(a);return true;
  }
  if(kind==0x3b){
   if(!state){next();copy(0x30);byte(3,m.read_byte(p+0x82));copy(0xc);byte(0x3a,1);byte(0xa,0);byte(9,4);}
   if(endflag())next();
   if(m.read_byte(p+0x82)!=b(3)){next();return true;}const auto index=m.read_byte(m.read_long(p+0x1c)+12);if(!index){byte(0xa,0);return true;}coordinates();face();if(index!=b(0xa)){byte(0xa,index);init(index,0x674d4);}render();return true;
  }
  if(!state){
   next();coordinates();attributes();word(0x50,m.read_word(p+0xd6));face();byte(9,(std::int8_t(variant)<4)?8:4);byte(0x3a,1);byte(0xf,3);const auto index=m.opcode_byte(0x298b4+variant);auto dx=m.opcode_word(0x298bc+variant*4);if(b(0xb))dx=std::uint16_t(-dx);word(0x10,w(0x10)+dx);word(0x14,w(0x14)+m.opcode_word(0x298beu+variant*4));auto vx=cl(0x298dc+variant*16),ax=cl(0x298e0+variant*16);if(b(0xb)){vx=0u-vx;ax=0u-ax;}lng(0x20,vx);lng(0x28,ax);lng(0x24,cl(0x298e4+variant*16));lng(0x2c,cl(0x298e8+variant*16));init(index,0x77962);h.display(a);return true;
  }
  if(endflag()){next();return true;}if(b(5))return false;move();if(sw(w(0x50))>sw(w(0x14))){if(dec(0xf)){next();return true;}word(0x14,w(0x50));lng(0x24,0x10000);lng(0x2c,0xffffe000);}render();return true;
 }
};
template<class M,class H> bool character_special_task(std::uint32_t a,std::uint32_t g,M&m,H&h){return CharacterSpecialTask<M,H>{m,h,g,a,CharacterSpecialTask<M,H>::ptr(m.read_word(a+0x32))}.tick();}
// Second-pool4 child of first-pool16, original2A406..2A4E1.
template<class M,class H> bool character_special_attachment(std::uint32_t a,std::uint32_t g,M&m,H&h){
 using T=CharacterSpecialTask<M,H>;if(m.read_byte(a+2)!=4)return false;const auto p=T::ptr(m.read_word(a+0x32));T t{m,h,g,a,p};const auto state=t.b(4);
 if(state==4){t.next();return true;}
 if(state==6){const auto linked=T::ptr(m.read_word(p+(t.b(3)?0x38:0x36)));if(std::uint16_t(linked)==std::uint16_t(a))m.write_word(linked,0);free_character_second_pool(a,g,m);return true;}
 if(state!=0&&state!=2)return false;
 if(!state){t.next();t.copy(0x30);t.byte(3,m.read_byte(p+0x82));t.copy(0xc);t.byte(0xa,0);t.word(0x14,m.read_word(p+0x24));if(t.b(3))t.word(0x14,t.w(0x14)+1);}
 t.byte(1,0);if(t.endflag()){t.next();return true;}const auto event=m.read_byte(m.read_long(p+0x1c)+10);if(!event){t.byte(0xa,0);return true;}
 t.copy(0x10);t.face();t.byte(1,1);if(event!=t.b(0xa)){const auto index=t.b(3)?event:16;t.byte(0xa,index);t.init(index,0x61826);}else h.step(a);return true;
}
} // namespace puzzle_fighter
