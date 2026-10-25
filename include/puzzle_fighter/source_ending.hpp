// SPDX-License-Identifier: GPL-3.0-only
// USA875E..89FE,8A04/8A3C/8C0A. Source ending/staff-roll coordinator.
#pragma once
#include "global_rng.hpp"
#include <cstdint>
#include <stdexcept>
namespace puzzle_fighter {
enum class EndingAction {reset_pools,first_pool,second_pool,objects,reset_first_pool,blank_scroll2,ending_song,stop_and_voice,stage_picture,stage_tick,stage_actors,all_palettes,interlude_scene,palette_actors,caption,closing_mark,packed_sound,easy_ending};
enum class EndingResult {complete,wait_object_clear,unsupported};
template<class M> void source_ending_palettes(M&m){
 constexpr unsigned sources[]={0x8aca,0x8aea,0x8b0a,0x8b2a,0x8b4a},destinations[]={0x90cbe0,0x90cb80,0x90cba0,0x90cbc0,0x90cb60};
 for(unsigned b=0;b<5;++b)for(unsigned n=0;n<32;n+=2)m.write_word(destinations[b]+n,m.opcode_word(sources[b]+n));
}
//8C0A: named source tile-data consumer. This is picture/text data, not code.
template<class M> void source_staff_roll_line(std::uint32_t g,M&m){
 const auto old=m.read_byte(g-0x735d);m.write_byte(g-0x735d,std::uint8_t(old+1));const bool erase=old&128;const unsigned index=std::uint8_t(old*2);auto from=std::uint32_t(0x8c52 + std::int16_t(m.opcode_word(0x8c52 + index)));
 for(unsigned budget=0;budget<4096;){const auto offset=std::int16_t(m.read_word(from));from+=2;const auto attr=m.read_word(from);from+=2;auto to=std::uint32_t(0x904000 + offset);
  for(;;){if(++budget>=4096)throw std::runtime_error("Source staff-roll picture budget");const auto tile=m.read_word(from);from+=2;if(!tile)return;if(tile&0x8000)break;m.write_word(to,erase?0x5820:std::uint16_t(tile+0x5800));m.write_word(to+2,attr);to+=0x40;}
 }throw std::runtime_error("Source staff-roll data unterminated");
}
//5670 source byte-text format; signed attributes and erase descriptors retained.
template<class M> std::uint32_t draw_source_notice(M&m,unsigned index){
 const auto input=std::uint8_t(index),doubled=std::uint8_t(input*2);const bool erase=input&128;auto from=std::uint32_t(0x1f88e + std::int16_t(m.read_word(0x1f88e + doubled)));
 for(unsigned budget=0;budget<4096;){const auto x=m.read_byte(from++),y=m.read_byte(from++);auto to=0x900000u+(unsigned(x)<<7)+y;const auto attribute=m.read_byte(from++);const auto attr=std::uint16_t(std::int16_t(std::int8_t(attribute)));
  for(;;){if(++budget>=4096)throw std::runtime_error("Source notice picture budget");const auto glyph=m.read_byte(from++);if(!glyph)return to;if(glyph==127)break;m.write_long(to,erase?0x00200000u|attribute:(std::uint32_t(glyph)<<16)|attr);to+=0x80;}
 }throw std::runtime_error("Source notice data unterminated");
}
//2AAA source three-row16-column closing mark, preserving its two variants.
template<class M> void draw_source_closing_mark(M&m,unsigned alternate,std::uint32_t to){
 unsigned first=alternate?0x1b0:0x150;for(unsigned n=0;n<16;++n,to+=0x80,++first)for(unsigned row=0;row<3;++row)m.write_long(to+row*4,((first+row*16)<<16)|0x17);
}
template<class M> void source_ending_to_game_over(std::uint32_t g,M&m){
 m.write_word(g+4,8);m.write_word(g+6,0);for(unsigned o=8;o<0x18;o+=4)m.write_long(g+o,0);for(unsigned o:{0xb8u,0xbdu,0x10fu,0x106u})m.write_byte(g+o,0);m.write_word(g+0x110,0);m.write_byte(g+0x14d,1);m.write_byte(g+0x8f,1);m.write_byte(g+0x104,1);m.write_word(g+0x20,0x90c0);
}
// H actions are original named units. Each active callback needs a native body.
template<class M,class H> struct SourceEnding {
 M&m;H&h;std::uint32_t g;
 enum class Resume {none,opening,return_only,closing};Resume pending=Resume::none;
 unsigned b(unsigned o){return m.read_byte(g+o);}unsigned w(unsigned o){return m.read_word(g+o);}
 void byte(unsigned o,unsigned v){m.write_byte(g+o,std::uint8_t(v));}void word(unsigned o,unsigned v){m.write_word(g+o,std::uint16_t(v));}
 void next(){word(0xc,w(0xc)+2);}bool zero(){word(0xe,w(0xe)-1);return !w(0xe);}
 void action(EndingAction a,unsigned x=0,unsigned y=0){h.action(a,x,y);}
 void frame(){action(EndingAction::first_pool);action(EndingAction::objects);}
 void opening_tail(){const auto random=global_random(w(0xae));word(0xae,random.seed);m.write_byte(g-0x735c,random.value&3);action(EndingAction::ending_song);for(unsigned o:{0x12au,0x125u,0x142u})byte(o,0);m.write_byte(g-0x735e,0);m.write_byte(g-0x735d,0);m.write_long(g+0x10,0);m.write_long(g+0x14,0);}
 void select_scene(){const unsigned index=std::uint8_t(m.read_byte(g-0x735c)*6+m.read_byte(g-0x735e));byte(0x120,m.opcode_byte(0x8a24 + index));m.write_byte(g-0x735e,m.read_byte(g-0x735e)+1);}
 EndingResult resume_after_clear(){const auto at=pending;pending=Resume::none;switch(at){case Resume::opening:opening_tail();break;case Resume::return_only:break;case Resume::closing:action(EndingAction::reset_pools);source_ending_to_game_over(g,m);break;default:return EndingResult::unsupported;}return EndingResult::complete;}
 EndingResult run(){
  if(pending!=Resume::none)return EndingResult::unsupported;
  if(b(0xbf)||!m.read_byte(g-0x7374)){action(EndingAction::easy_ending);return EndingResult::complete;}
  const auto bit=(b(0xa8)&1)?1u:2u;if(b(0x60)&bit){action(EndingAction::stop_and_voice);pending=Resume::closing;return EndingResult::wait_object_clear;}
  switch(w(0xc)){
  case 0:next();word(0x20,0x92a0);word(0x48,0x92a0);action(EndingAction::reset_pools);pending=Resume::opening;return EndingResult::wait_object_clear;
  case 2:{next();word(0x20,0x90c0);word(0x48,0x90c0);word(0x26,0);word(0x28,0x300);m.write_long(g+0x10,0);word(0xe,360);source_ending_palettes(m);word(0x32,0x1b0e);word(0x3a,0x2461);if(!b(0xad)){const auto a=h.allocate_first();if(a){m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,0x505);}}source_staff_roll_line(g,m);return EndingResult::complete;}
  case 4:if(!zero()){frame();return EndingResult::complete;}next();action(EndingAction::blank_scroll2);if(m.read_byte(g-0x735e)==4){word(0xc,16);action(EndingAction::reset_pools);pending=Resume::return_only;return EndingResult::wait_object_clear;}word(0xe,360);word(0x32,0x1b0e);word(0x3a,0x2461);source_staff_roll_line(g,m);return EndingResult::complete;
  case 6:if(zero()){word(0x20,0x92a0);word(0x48,0x92a0);next();action(EndingAction::reset_first_pool);select_scene();}frame();return EndingResult::complete;
  case 8:next();word(0x100,m.opcode_word(0x88f4 + b(0x120)*2));action(EndingAction::stage_picture);action(EndingAction::stage_tick);action(EndingAction::stage_actors);action(EndingAction::all_palettes);return EndingResult::complete;
  case 10:next();word(0x20,0x90c0);word(0x48,0x90c0);return EndingResult::complete;
  case 12:action(EndingAction::interlude_scene);action(EndingAction::first_pool);action(EndingAction::second_pool);action(EndingAction::stage_tick);action(EndingAction::objects);return EndingResult::complete;
  case 14:word(0x20,0x92a0);word(0x48,0x92a0);word(0xc,2);action(EndingAction::reset_pools);pending=Resume::return_only;return EndingResult::wait_object_clear;
  case 16:next();word(0x20,0x90c0);word(0x48,0x90c0);word(0x22,0);word(0x24,0);if(m.read_byte(g-0x7374)==2){word(0xe,600);action(EndingAction::palette_actors);action(EndingAction::caption,0x59);action(EndingAction::closing_mark,1,0x902830);}else{word(0xe,180);action(EndingAction::caption,0x5a);action(EndingAction::closing_mark,1,0x90283c);}return EndingResult::complete;
  case 18:if(zero()){next();word(0xe,180);action(EndingAction::packed_sound,0x01000000);}else frame();return EndingResult::complete;
  case 20:if(zero()){action(EndingAction::stop_and_voice);pending=Resume::closing;return EndingResult::wait_object_clear;}frame();return EndingResult::complete;
  default:return EndingResult::unsupported;
  }
 }
};
} // namespace puzzle_fighter
