// SPDX-License-Identifier: GPL-3.0-only
// USA original selection actors and252C4 producer. Unlisted states return false.
#pragma once
#include "actor_tasks.hpp"
#include "attack_display.hpp"
#include "scene_text_picture.hpp"
namespace puzzle_fighter {
//17CE0, original64-tile large-map block. Caller retains source coordinates.
template<class M> void selection_large_map_block(unsigned index,std::uint32_t to,M&m){const auto from=0xe1080u+((index&255u)<<8);for(unsigned n=0;n<64;++n){m.write_word(to+n*4,m.read_word(from+n*4)+0x1cc0);m.write_word(to+n*4+2,m.read_word(from+n*4+2));}}
template<class M,class H> void initialize_selection_scene(std::uint32_t g,M&m,H&h){
 const auto a=h.allocate_first();if(!a)return;m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,0x1a00);if(m.read_byte(g+0x152))m.write_byte(a+3,1);
 for(unsigned column=0;column<8;++column)for(unsigned side=0;side<2;++side)selection_large_map_block(0x66,0x908000+column*0x800+side*0x100,m);
}
template<class M,class H> struct SelectionActorTask {
 M&m;H&h;std::uint32_t g,a;
 static int sw(std::uint16_t v){return std::int16_t(v);}static std::uint32_t ptr(std::uint16_t v){return std::uint32_t(sw(v));}
 unsigned b(unsigned o){return m.read_byte(a+o);}unsigned w(unsigned o){return m.read_word(a+o);}std::uint32_t l(unsigned o){return m.read_long(a+o);}
 void byte(unsigned o,unsigned v){m.write_byte(a+o,std::uint8_t(v));}void word(unsigned o,unsigned v){m.write_word(a+o,std::uint16_t(v));}void lng(unsigned o,std::uint32_t v){m.write_long(a+o,v);}void next(unsigned o=4){byte(o,b(o)+2);}
 unsigned cb(std::uint32_t p){const auto v=m.opcode_word(p&~1u);return p&1?v&255:v>>8;}
 bool down(unsigned o=0xe){byte(o,b(o)-1);return b(o)==0;}
 void mark(){const auto p=g-0x7376;m.write_byte(p,m.read_byte(p)|(1u<<(b(3)&7)));}
 void position_add(std::int32_t multiplier=1){lng(0x10,l(0x10)+l(0x20)*std::uint32_t(multiplier));}
 void init(unsigned index){h.init(a,index,0x7dbfe);}
 void static_portrait(){const auto index=(m.read_byte(l(0x32))+0x23)&255;lng(0x1c,0x7dac6u+sw(m.read_word(0x7dac6+index*2))-4);}
 void portrait_palette(){const auto parent=g+(b(3)?0xc00:0xb00);const auto table=0x23f40u+(m.read_byte(parent+0x9f)?22:0);auto from=0xaa1c0u+sw(m.read_word(table+m.read_byte(parent+0x82)*2));auto to=0x90c000u+(b(0xd)<<5);for(unsigned n=0;n<80;++n)m.write_word(to+n*2,m.read_word(from+n*2)|0xf000);}
 void stand_shape(unsigned index){const auto table=m.read_word(g+0x90)?0x7db78u:0x7db20u;const auto frame=table+index*4;lng(0x1c,frame);lng(0x18,m.read_long(frame));}
 void handicap_position(){auto index=m.read_byte(l(0x32))*4u;if(b(3)!=2)index+=2;word(0x10,m.opcode_word(0x26cf8+index));}
 void handicap_frame(unsigned base){init(std::uint8_t(m.read_byte(l(0x32))+base));}
 bool tick(){const auto kind=b(2),state=b(4),sub=b(5),variant=b(3);
  if(kind!=0x17&&kind!=0x18&&kind!=0x19&&kind!=0x1a&&kind!=0x1f&&kind!=0x21&&kind!=0x22&&kind!=0x27&&kind!=0x2a)return false;
  if(state==4||state==6){h.free(a);return true;}if(state!=0&&state!=2)return false;
  if(kind==0x17){
   if(!state){next();word(0x30,0xe000);byte(9,2);byte(0xb,0);byte(0xe,cb(0x24f64+variant));byte(0xf,b(0xe));word(0x10,m.opcode_word(0x24f6c+variant*4));word(0x14,m.opcode_word(0x24f6eu+variant*4));init(5);return true;}
   switch(sub){case 0:if(!b(0xe)){next(5);byte(0xe,0x11);}byte(0xe,b(0xe)-1);break;case 2:if(down()){next(5);mark();}word(0x14,w(0x14)+12);break;case 4:if(!m.read_byte(g-0x7376)){next();byte(5,0);}break;default:return false;}h.display(a);return true;
  }
  if(kind==0x18){
   if(!state){next();word(0x30,0x8000);byte(0xb,0);
    if(!b(0xa)){byte(9,4);byte(0xc,255);byte(0x50,m.read_byte(l(0x32)));word(0x14,0xb8);word(0x10,variant?0x140:0x40);byte(0xd,variant?0x1b:0x16);if(variant)byte(0xb,1);const auto parent=g+(variant?0xc00:0xb00);word(0x52,parent);byte(0x51,m.read_byte(parent+0x9f));static_portrait();portrait_palette();}
    else{byte(9,6);word(0x14,0xc0);if(!b(0xe)){word(0x10,variant?0x1d0:0xffb0);lng(0x20,variant?0xfffd0000:0x30000);byte(0xe,0x2d);}else{byte(5,4);word(0x10,variant?0x140:0x40);}lng(0x1c,0x251a4);}return true;
   }
   if(!b(0xa)){const auto ch=m.read_byte(l(0x32));if(ch!=b(0x50)){byte(0x50,ch);static_portrait();portrait_palette();}else if(m.read_byte(ptr(w(0x52))+0x9f)!=b(0x51))portrait_palette();}
   else{switch(sub){case 0:if(down()){next(5);byte(0xe,0x30);}break;case 2:if(down())next(5);position_add();break;case 4:break;default:return false;}byte(0xb,b(0xb)^1);}h.display(a);return true;
  }
  if(kind==0x19){
   if(!state){next();word(0x30,0xc000);byte(9,0);byte(0xb,0);byte(0xe,0x10);word(0x14,0x90);word(0x10,variant?0x1c0:0xffc0);lng(0x20,variant?0xfff40000:0xc0000);init(std::uint8_t(variant+(m.read_word(g+0x90)?0x12:0)));return true;}
   switch(sub){case 0:if(down()){next(5);mark();}position_add();break;case 2:if(!m.read_byte(g-0x7376)){next(5);byte(0xe,0x10);}if(variant==m.read_byte(l(0x32)))h.step(a);break;case 4:if(down()){next();byte(5,0);mark();}position_add(-2);break;default:return false;}h.display(a);return true;
  }
  if(kind==0x1a){
   if(!state){next();byte(0xa,255);word(0x14,(m.read_word(g+0x2c)+15)&31);}
   m.write_word(g+0x2c,m.read_word(g+0x2c)-2);
   if(b(0xa)){byte(0xa,0);const auto table=0x25450u+sw(m.read_word(0x25450+variant*2));const auto frame=table+sw(w(0x16)>>2);auto silent=[](unsigned,unsigned,unsigned){};ActorTasks<M,decltype(silent)>{m,silent,g}.palette_start(a,frame);}
   const auto scroll=std::uint16_t(m.read_word(g+0x2c)+15);const unsigned position=scroll&31;if(position>=w(0x14)){byte(0xa,255);word(0x16,scroll&0xe0);}word(0x14,position);return true;
  }
  if(kind==0x1f){
   if(!state){next();word(0x30,0xc000);byte(9,0);byte(0xb,0);byte(0xe,0x10);word(0x10,m.opcode_word(0x25edc+variant*4));word(0x14,m.opcode_word(0x25edeu+variant*4));init(std::uint8_t(variant+2)+(m.read_word(g+0x90)?0x12:0));return true;}
   switch(sub){case 0:if(down()){next(5);mark();}word(0x14,w(0x14)-8);break;case 2:if(!m.read_byte(g-0x7376))next(5);break;case 4:word(0x14,w(0x14)+8);if(w(0x14)>=0x140){next();byte(5,0);}break;default:return false;}h.display(a);return true;
  }
  if(kind==0x21){
   if(!state){next();word(0x30,0xe000);byte(9,4);byte(0xb,0);unsigned base=12;if(m.read_byte(g+0xac)&(1u<<(variant&7))){base=16;byte(5,4);}init(std::uint8_t(variant+base));return true;}
   switch(sub){case 0:if(m.read_byte(g-0x7373)&(1u<<(variant&7))){next(5);byte(0xe,20);init(std::uint8_t(variant+14));}break;case 2:if(down()){next(5);init(std::uint8_t(variant+16));}break;case 4:if(m.read_byte(g-0x7370)){next();byte(5,0);}break;default:return false;}
   const auto ch=m.read_byte(l(0x32));if(ch<8){word(0x10,m.opcode_word(0x26270+ch*4));word(0x14,m.opcode_word(0x26272+ch*4));}h.step(a);if(m.read_byte(l(0x32))<8)h.display(a);return true;
  }
  if(kind==0x22){
   if(variant>3)return false;
   if(!state){next();byte(9,0);byte(0xb,0);word(0x30,0xe000);const auto ch=m.read_byte(ptr(w(0x32))+0x82);byte(0x50,ch);stand_shape(ch+(variant==1?11:0));return true;}
   if(variant==0){if(m.read_byte(g+0x14d)==2||m.read_byte(g+0x2900))next();}
   else if(variant==2){if(m.read_byte(g+0x14d)==2)next();}
   if(variant==0||variant==2){const auto ch=m.read_byte(ptr(w(0x32))+0x82);if(ch!=b(0x50)){byte(0x50,ch);stand_shape(ch);}}
   h.display(a);return true;
  }
  if(kind==0x27){
   if(!state){next();word(0x30,0xc000);byte(9,0);byte(0xb,0);word(0x10,0xc0);byte(0xe,0x10);word(0x14,0xff70);h.init(a,0x1c,0x7da5e);return true;}
   switch(sub){case 0:if(down())next(5);word(0x14,w(0x14)+12);break;case 2:if(!m.read_byte(g-0x736f)){next(5);byte(0xe,0x10);}break;case 4:if(down()){next();byte(5,0);}word(0x14,w(0x14)-12);break;default:return false;}h.display(a);return true;
  }
  if(variant>3)return false;
  if(!state){next();word(0x30,0xc000);byte(0xb,0);
   if(variant<2){byte(9,2);word(0x14,0xc0);byte(0xe,8);word(0x10,variant?0x1c0:0xffc0);lng(0x20,variant?0xfff00000:0x100000);byte(0x50,m.read_byte(l(0x32)));init(0x17);}
   else{byte(9,0);word(0x14,0xbb);lng(0x20,variant==2?0x80000:0xfff80000);byte(0x50,m.read_byte(l(0x32)));handicap_position();handicap_frame(7);}return true;
  }
  if(variant<2){switch(sub){case 0:if(down()){next(5);mark();}position_add();break;case 2:if(m.read_byte(g-0x7373)&(1u<<(variant&7))){next(5);handicap_frame(0x18);}break;case 4:if(!m.read_byte(g-0x7376)){next(5);byte(0xe,0x2d);}h.step(a);break;case 6:if(down()){next(5);byte(0xe,8);}h.step(a);break;case 8:if(down()){next();byte(5,0);}position_add(-1);h.step(a);break;default:return false;}}
  else{switch(sub){case 0:if(m.read_byte(g-0x7373)&(1u<<((variant-2)&7))){next();byte(5,0);}handicap_position();if(m.read_byte(l(0x32))!=b(0x50)){byte(0x50,m.read_byte(l(0x32)));handicap_frame(7);}else h.step(a);break;case 2:if(down()){next();byte(5,0);}position_add(-1);break;default:return false;}}
  h.display(a);return true;
 }
};
template<class M,class H> bool selection_actor_task(std::uint32_t a,std::uint32_t g,M&m,H&h){return SelectionActorTask<M,H>{m,h,g,a}.tick();}
template<class M> bool selection_help_task(std::uint32_t a,std::uint32_t g,M&m){
 if(m.read_byte(a+2)!=4){return false;}const auto state=m.read_byte(a+4);
 if(state==6){free_attack_display(a,g,m);return true;}if(state==4){m.write_byte(a+4,6);return true;}
 if(!state){m.write_byte(a+4,2);m.write_byte(a+1,0);m.write_byte(a+9,0);m.write_word(a+0xc,0);m.write_word(a+0x42,0);m.write_word(a+0x30,0xc000);m.write_long(a+0x1c,a+0x40);m.write_word(a+0x40,0);m.write_long(a+0x48,a+0xe0);return true;}
 if(state!=2){return false;}m.write_byte(a+1,0);const auto index=m.read_word(g-0x736c);if(!index)return true;if(index&0x8000){m.write_byte(a+4,4);return true;}
 m.write_byte(a+1,1);m.write_word(a+0x42,0);m.write_word(a+0x10,0x38);m.write_word(a+0x14,0x38);
 const auto language=std::uint16_t(m.read_word(g+0x92)*2);auto source=m.read_long(0x2b0f8u+std::int16_t(language));const auto lines=m.read_byte(m.read_long(0x2b108u+std::int16_t(language))+std::int16_t(index));
 if(!lines){m.write_byte(a+1,0);return true;}auto offset=std::uint16_t(index*68);if(language)offset=std::uint16_t(offset*2);source+=offset;m.write_word(a+0x46,9);
 SceneTextCursor cursor{source,a+0x4c,a+0xe0};append_scene_text_line(g,a,m,cursor,m.read_word(g+0x92)?33:16,8,0);
 if(lines==2){m.write_word(a+0x14,m.read_word(a+0x14)+8);append_scene_text_line(g,a,m,cursor,m.read_word(g+0x92)?33:16,8,16);}m.write_word(a+0x44,m.read_word(a+0x42)-1);return true;
}
} // namespace puzzle_fighter
