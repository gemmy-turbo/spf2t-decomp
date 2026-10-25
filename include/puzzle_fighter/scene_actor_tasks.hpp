// SPDX-License-Identifier: GPL-3.0-only
// Original USA scene-only actors. Unlisted kinds explicitly return false.
#pragma once
#include "actor_tasks.hpp"
#include "match_interlude.hpp"
#include "attack_display.hpp"
#include "scene_text_picture.hpp"
#include "round_announcements.hpp"
namespace puzzle_fighter {
template<class M,class H> bool continue_announcement_task(std::uint32_t a,std::uint32_t g,M&m,H&h){
 if(m.read_byte(a+2)!=3||m.read_byte(a+3)!=3)return false;
 if(m.read_byte(a+4)!=2)return round_announcement_task(a,g,m,h);
 if(m.read_byte(g+0x104)){m.write_byte(a+4,m.read_byte(a+4)+2);return true;}
 const auto player=std::uint32_t(std::int32_t(std::int16_t(m.read_word(g+0x116))));const auto seconds=m.read_byte(player+0x8c);
 if(seconds!=m.read_byte(a+7)){m.write_byte(a+7,seconds);h.sound(m.opcode_word(0x2b9d6+(seconds&15)*2),0x1000,0);h.sound(0xea,0x1000,0);}
 h.init(a,std::uint8_t(seconds+10),0x7da5e);h.display(a);return true;
}
template<class M> std::uint32_t allocate_scene_first(std::uint32_t g,M&m){if(!m.read_byte(g+0x581c))return 0;m.write_byte(g+0x581c,m.read_byte(g+0x581c)-1);const auto stack=std::uint32_t(std::int32_t(std::int16_t(m.read_word(g+0x580a))));const auto a=std::uint32_t(std::int32_t(std::int16_t(m.read_word(stack))));m.write_word(g+0x580a,std::uint16_t(stack+2));return std::uint16_t(stack+2)?a:0;}
template<class M> std::uint32_t allocate_scene_third(std::uint32_t g,M&m){if(!m.read_byte(g+0x581e))return 0;m.write_byte(g+0x581e,m.read_byte(g+0x581e)-1);const auto stack=std::uint32_t(std::int32_t(std::int16_t(m.read_word(g+0x580e))));const auto a=std::uint32_t(std::int32_t(std::int16_t(m.read_word(stack))));m.write_word(g+0x580e,std::uint16_t(stack+2));return std::uint16_t(stack+2)?a:0;}
template<class M> void initialize_source_quote_extra(std::uint32_t g,M&m){const auto quote=m.read_word(g+0x136);if(quote!=0x40&&quote!=0x4e)return;const auto a=allocate_scene_first(g,m);if(!a)return;m.write_byte(a,m.read_byte(a)+1);m.write_byte(a+2,0x42);m.write_byte(a+3,quote==0x4e);for(unsigned n=0;n<16;++n)m.write_word(0x90c2a0+n*2,m.read_word((quote==0x4e?0xa0400u:0xab840u)+n*2)|0xf000);}
// USA2A730: quote, tutorial caption and short-course English caption.
template<class M> bool source_quote_task(std::uint32_t a,std::uint32_t g,M&m){
 const auto variant=m.read_byte(a+3);
 if(m.read_byte(a+2)!=0||variant>2)return false;
 switch(m.read_byte(a+4)){
 case 0:m.write_byte(a+4,2);m.write_byte(a+1,0);m.write_byte(a+9,0);m.write_word(a+0xc,0);m.write_word(a+0x42,0);m.write_word(a+0x30,0xe000);m.write_long(a+0x1c,a+0x40);m.write_word(a+0x40,0);m.write_long(a+0x48,a+0xe0);return true;
 case 2:if(m.read_byte(a+5)==2)return true;if(m.read_byte(a+5))return false;{
  m.write_byte(a+5,2);m.write_byte(a+1,1);m.write_word(a+0x42,0);m.write_word(a+0x10,variant==1?0xa0:0x38);m.write_word(a+0x14,variant==1?0xc0:0x38);
  const auto language=std::uint16_t(variant==2?0:m.read_word(g+0x92)*2);auto quote=m.read_word(a+0x1a);if(variant==2&&m.read_word(g+0x92))++quote;
  const auto table=variant==1?0x2a9d8u:0x2a9b8u;auto source=m.read_long(table+language);const auto descriptor=m.read_long(table+16+language)+std::int16_t(quote);const auto lines=m.read_byte(descriptor);if(!lines){m.write_byte(a+1,0);return true;}
  auto offset=std::uint16_t(quote*(variant==1?96:68));if(language)offset=std::uint16_t(offset*2);source+=offset;m.write_word(a+0x46,variant==1?9:11);SceneTextCursor cursor{source,a+0x4c,a+0xe0};
  const auto count=std::uint16_t(variant==1?(language?31:15):(language?33:16));append_scene_text_line_mode(a,m,cursor,count,8,0,language!=0);
  if(variant==1){if(lines!=1){append_scene_text_line_mode(a,m,cursor,count,8,16,language!=0);if(lines!=2)append_scene_text_line_mode(a,m,cursor,count,8,32,language!=0);}}
  else if(lines==2){m.write_word(a+0x14,m.read_word(a+0x14)+8);append_scene_text_line_mode(a,m,cursor,count,8,16,language!=0);}
  m.write_word(a+0x44,m.read_word(a+0x42)-1);return true;}
 case 4:m.write_byte(a+4,6);return true;case 6:free_attack_display(a,g,m);return true;default:return false;
 }
}
template<class M> void scene_palette_apply(std::uint32_t state,M&m){
 auto blocks=m.read_long(m.read_long(state)+4);
 for(;;){auto destination=0x90c000u+std::uint32_t(std::int32_t(std::int16_t(m.read_word(blocks))));const auto index=m.read_word(blocks+2),descriptor=m.read_word(blocks+4);blocks+=6;const auto table=0x2d3a6u+std::uint32_t(std::int32_t(std::int16_t(index)));auto source=((std::uint32_t(m.opcode_word(table))<<16)|m.opcode_word(table+2))+(descriptor&0xfffeu);for(unsigned n=0;n<16;++n,source+=2,destination+=2)m.write_word(destination,m.read_word(source)|0xf000);if(descriptor&1)return;}
}
template<class M,class H> struct SceneActorTask {
 M&m;H&h;std::uint32_t g,a;
 static int sw(std::uint16_t v){return std::int16_t(v);}static std::uint32_t ptr(std::uint16_t v){return std::uint32_t(sw(v));}
 unsigned b(unsigned o){return m.read_byte(a+o);}unsigned w(unsigned o){return m.read_word(a+o);}std::uint32_t l(unsigned o){return m.read_long(a+o);}
 void byte(unsigned o,unsigned v){m.write_byte(a+o,std::uint8_t(v));}void word(unsigned o,unsigned v){m.write_word(a+o,std::uint16_t(v));}void lng(unsigned o,std::uint32_t v){m.write_long(a+o,v);}void next(unsigned o=4){byte(o,b(o)+2);}
 std::uint8_t cb(std::uint32_t at){auto v=m.opcode_word(at&~1u);return std::uint8_t(at&1?v:v>>8);}std::uint32_t cl(std::uint32_t at){return(std::uint32_t(m.opcode_word(at))<<16)|m.opcode_word(at+2);}
 bool dec(unsigned o){byte(o,b(o)-1);return(b(o)&0x80)!=0;}
 void move(){for(unsigned n:{0u,4u}){lng(0x10+n,l(0x10+n)+l(0x20+n));lng(0x20+n,l(0x20+n)+l(0x28+n));}}
 void static_shape(unsigned index){const auto table=0x7dac6u;lng(0x1c,table+sw(m.read_word(table+2*(index&255)))-4);}
 void portrait_palette(unsigned character,unsigned alternate,std::uint32_t to,unsigned rows=10,unsigned bright=0xf000){const auto table=0x23f40u+(alternate?22:0);auto from=0xaa1c0u+sw(m.read_word(table+character*2));for(unsigned n=0;n<rows*16;++n)m.write_word(to+n*2,m.read_word(from+n*2)|bright);}
 void portrait_face(unsigned character){byte(0xb,cb(0x23ee6+character)&b(0xa));}
 void portrait_dan(unsigned character,unsigned alternate){if(character!=10)return;for(unsigned n=0;n<16;++n)m.write_word(0x90c000+n*2,m.read_word((alternate?0xa0a80u:0xa0a00u)+n*2)|0xf000);}
 void portrait_child(){if(!m.read_byte(g+0x581c))return;const auto stack=ptr(m.read_word(g+0x580a)),child=ptr(m.read_word(stack));m.write_byte(g+0x581c,m.read_byte(g+0x581c)-1);m.write_word(g+0x580a,std::uint16_t(stack+2));if(!std::uint16_t(stack+2))return;m.write_byte(child,m.read_byte(child)+1);m.write_word(child+2,0x503);m.write_word(child+0xc,0xff0d);m.write_word(child+0x10,w(0x10));m.write_word(child+0x14,w(0x14));m.write_word(child+0x32,std::uint16_t(a));}
 bool portrait(){
  const auto phase=b(4),variant=b(3);if(variant>4)return false;if(phase==4||phase==6){h.free(a);return true;}
  if(!phase){next();word(0x30,0xe000);if(variant==2||variant==3){byte(9,2);static_shape(0x21);return true;}word(0xc,0xff00);byte(9,variant==0?4:0);const auto p=ptr(w(0x32));const auto ch=m.read_byte(p+0x82),alt=m.read_byte(p+0x9f);
   if(variant==0){byte(0xe,31);word(0x10,b(0xa)?0xd0:0xb0);word(0x14,0xc0);word(0x50,b(0xa)?0x100:0x80);word(0x52,0x80);lng(0x20,b(0xa)?0u-0xc8000u:0xc8000);lng(0x28,b(0xa)?0xe758:0xffff18a8);lng(0x24,0xfff40000);lng(0x2c,0xa500);portrait_palette(ch,alt,0x90c000);portrait_dan(ch,alt);portrait_face(ch);static_shape(ch);}
   else if(variant==1){byte(0xe,23);word(0x10,b(0xa)?0xffc0:0x1c0);word(0x14,0x88);word(0x50,b(0xa)?0x130:0x50);word(0x52,0x88);lng(0x20,b(0xa)?0x2c0000:0xffd40000);lng(0x28,b(0xa)?0xfffd8000:0x28000);lng(0x24,0);lng(0x2c,0);word(0xc,b(0xa)?0xff0a:0xff00);portrait_palette(ch,alt,b(0xa)?0x90c140:0x90c000);portrait_face(ch);static_shape(ch+11);portrait_child();}
   else{word(0x10,0x80);word(0x14,0x80);portrait_palette(ch,0,0x90c000);portrait_face(ch);static_shape(ch);}return true;
  }
  if(phase!=2){return false;}const auto sub=b(5);
  if(variant==0){if(!sub){bool done=m.read_byte(g+0x14d)!=0;if(!done){move();done=dec(0xe);}if(done){next(5);word(0x10,w(0x50));word(0x14,w(0x52));}}else if(sub==2){if(m.read_byte(g+0x2900)||m.read_byte(g+0x118))byte(4,4);}else return false;h.display(a);}
  else if(variant==1){if(m.read_byte(g+0x8a)){byte(4,4);return true;}if(!sub){if(m.read_byte(g+0x14d)==1||dec(0xe)){next(5);word(0x10,w(0x50));word(0x14,w(0x52));}else move();}else if(sub==2){if(m.read_byte(g+0x14d)==2){next(5);byte(0xe,15);lng(0x20,b(0xa)?0x80000:0xfff80000);lng(0x28,b(0xa)?0xffffc000:0x4000);lng(0x24,0);lng(0x2c,0);}}else if(sub==4){if(dec(0xe))byte(4,4);else move();}else return false;h.display(a);}
  else if(variant==2){byte(0xb,b(0xb)^1);h.display(a);}
  else if(variant==3){if(m.read_byte(g+0x8a)){byte(4,4);return true;}const auto parent=ptr(w(0x32));word(0x10,m.read_word(parent+0x10));word(0x14,m.read_word(parent+0x14));if(m.read_byte(g+0x14d)==2){byte(4,4);return true;}byte(0xb,b(0xb)^1);h.display(a);}
  else {h.display(a);}return true;
 }
 bool panel(){const auto phase=b(4);if(!phase){next();word(0xc,0xff0d);byte(0xb,0);byte(9,4);word(0x30,0xe000);word(0x10,b(0xa)?0xfe96:0x18a);word(0x14,0xf0);word(0x20,b(0xa)?12:0xfff4);word(0x28,b(0xa)?0xfffe:2);byte(0xe,31);lng(0x24,0x242b4);lng(0x28,m.read_long(0x242b4));scene_palette_apply(a+0x24,m);h.init(a,0x1b,0x7da5e);return true;}
  if(phase==6){h.free(a);return true;}if(phase==4){if(m.read_byte(g+0x2900)||m.read_byte(g+0x118))next();h.display(a);return true;}if(phase!=2)return false;
  if(m.read_byte(g+0x14d)){byte(5,6);}switch(b(5)){case 0:if(dec(0xe)){next(5);byte(0xe,1);}word(0x10,w(0x10)+w(0x20));break;case 2:if(dec(0xe)){byte(0xe,1);auto v=std::uint16_t(w(0x20)+w(0x28));word(0x20,v);if(!v)next(5);if(v&2)v=std::uint16_t(0-v);word(0x10,w(0x10)+v);}break;case 4:next();break;case 6:next();word(0x10,0x10);break;default:return false;}h.display(a);return true;
 }
 unsigned continue_frame(){return cb(0x24436+m.read_byte(ptr(m.read_word(g+0x116))+0x8c));}
 void continue_shape(unsigned frame){byte(0xa,frame);const auto table=cl(0x24464+b(3)*4);const auto address=table+sw(m.read_word(table+frame*2));lng(0x1c,address);lng(0x18,m.read_long(address));}
 bool continue_character(){switch(b(4)){case 0:next();word(0x30,0xe000);byte(0xb,1);word(0x10,0x80);word(0x14,0x60);continue_shape(continue_frame());return true;case 2:if(m.read_byte(g+0x104)){next();return true;}if(const auto frame=continue_frame();frame!=b(0xa))continue_shape(frame);h.step(a);h.display(a);return true;case 4:h.free(a);return true;default:return false;}}
 bool versus_emblem(){const auto variant=b(3);if(variant>1)return false;switch(b(4)){
 case 0:next();byte(9,0);byte(0xb,0);word(0xc,0xff0d);word(0x30,0xe000);lng(0x50,0x2396c);lng(0x54,m.read_long(0x2396c));byte(0xe,31);word(0x10,variant?0:0x180);word(0x14,variant?0x40:0xc0);word(0x32,0xc0);word(0x34,0x80);lng(0x20,variant?0u-0xffe80005u:0xffe80005);lng(0x28,variant?0u-0x1294au:0x1294a);lng(0x24,variant?0u-0xffeffffdu:0xffeffffd);lng(0x2c,variant?0u-0xe73au:0xe73a);h.init(a,0x15+variant,0x7da5e);match_interlude_detail::palette_step(a+0x50,m);return true;
 case 2:
  if(m.read_byte(g+0x8a)){byte(4,4);return true;}
  switch(b(5)){
  case 0:if(m.read_byte(g+0x14d)==1||dec(0xe)){next(5);if(!variant)h.sound(0xf1,0x1000,0);word(0x10,w(0x32));word(0x14,w(0x34));match_interlude_detail::palette_step(a+0x50,m);}else move();break;
  case 2:if(m.read_byte(g+0x14d)==1||(w(0x56)&0x8000)){next(5);byte(0xe,15);lng(0x20,variant?0u-0x180000u:0x180000);lng(0x28,variant?0x18000:0xfffe8000);lng(0x24,variant?0u-0xc0000u:0xc0000);lng(0x2c,variant?0xc000:0xffff4000);lng(0x50,0x23a4c);word(0x54,1);word(0x56,m.read_word(0x23a4e));h.init(a,0x17+variant,0x7da5e);}match_interlude_detail::palette_step(a+0x50,m);break;
  case 4:if(m.read_byte(g+0x14d)==2){if(dec(0xe))next();move();}break;default:return false;}
  h.display(a);return true;
 case 4:case 6:h.free(a);return true;default:return false;}
 }
 bool tick(){switch(b(2)){case 5:return portrait();case 0xa:return panel();case 0xc:return continue_character();case 0x42:switch(b(4)){case 0:next();word(0xc,0);byte(9,0);byte(0xb,0);word(0x10,0x140);word(0x14,0x30);byte(0x3a,1);word(0x30,0xe000);h.init(a,b(3),0x29e8a);return true;case 2:if(m.read_byte(g+0x2900)||m.read_byte(g+0x118)){next();return true;}h.step(a);h.display(a);return true;case 4:h.free(a);return true;default:return false;}case 0x25:switch(b(4)){case 0:next();byte(9,2);byte(0xb,0);word(0x30,0xa000);word(0x10,0xc0);word(0x14,0x30);h.init(a,0x1c,0x7da5e);return true;case 2:if(m.read_byte(g+0x2900)||m.read_byte(g+0x118))next();byte(0xb,b(0xb)^1);h.display(a);return true;case 4:case 6:h.free(a);return true;default:return false;}default:return false;}}
};
template<class M,class H> bool scene_actor_task(std::uint32_t a,std::uint32_t g,M&m,H&h){auto task=SceneActorTask<M,H>{m,h,g,a};if(m.read_byte(a+2)==4)return task.versus_emblem();return task.tick();}
} // namespace puzzle_fighter
