// SPDX-License-Identifier: GPL-3.0-only
// USA9692..98CE: source short-course/Dan ending, not a native replacement skit.
#pragma once
#include "source_ending.hpp"
#include "scene_actor_tasks.hpp"
namespace puzzle_fighter {
template<class M> void source_actor_palette_preserving_brightness(std::uint32_t a,M&m){ //14770
 const auto index=std::uint16_t((m.read_byte(a+0x82)*2+m.read_byte(a+0x9f))<<7);auto from=std::uint32_t(0xa0000 + std::int16_t(index));auto to=m.read_byte(a+2)?0x90c200u:0x90c180u;
 for(unsigned n=0;n<128;n+=4)m.write_long(to+n,m.read_long(from+n)|(m.read_long(to+n)&0xf000f000u));
}
enum class EasyEndingAction {reset_pools,wide_picture,large_picture,all_palettes,song,voice,first_pool,third_pool,objects,large_map_palette};
enum class EasyEndingResult {complete,wait_object_clear,unsupported};
template<class M,class H> struct SourceEasyEnding {
 M&m;H&h;std::uint32_t g;bool clearing=false;
 void action(EasyEndingAction a,unsigned x=0,std::uint32_t y=0){h.action(a,x,y);}
 void frame(){actor();action(EasyEndingAction::first_pool);action(EasyEndingAction::third_pool);action(EasyEndingAction::objects);}
 EasyEndingResult resume_after_clear(){if(!clearing)return EasyEndingResult::unsupported;clearing=false;
  m.write_word(g+0x32,0x1b0e);m.write_word(g+0x3a,0x2461);m.write_word(g+0x20,0x90c0);m.write_word(g+0x26,0);m.write_word(g+0x28,0x300);m.write_word(g+0x2a,0x10);m.write_word(g+0x2c,0x710);
  action(EasyEndingAction::wide_picture,0x1e,0x904000);action(EasyEndingAction::wide_picture,0x1f,0x904400);for(unsigned n=0;n<4;++n)action(EasyEndingAction::large_picture,n<2?0x1e + n:0x65 + n,0x908000 + n*0x100);
  m.write_word(g+0x100,6);action(EasyEndingAction::all_palettes);action(EasyEndingAction::song);const auto a=g+0xb00;m.write_word(a,0x101);m.write_word(a+0x30,0xa000);m.write_word(a+12,0);m.write_byte(a+0x82,10);source_actor_palette_preserving_brightness(a,m);m.write_byte(a+11,0);m.write_word(a+0x30,0xe000);m.write_word(a+0x10,0x1c0);m.write_word(a+0x14,0x55);m.write_word(a+0xd6,0x55);m.write_long(a+0x20,0xfffe8000);h.actor_init(a,9,0x7ab6c);frame();return EasyEndingResult::complete;
 }
 bool actor(){const auto a=g+0xb00;switch(m.read_byte(a+4)){
 case 0:m.write_long(a+0x10,m.read_long(a+0x10)+m.read_long(a+0x20));if(std::int16_t(m.read_word(a+0x10))>0xc0){h.actor_step(a);return true;}
  m.write_byte(a+4,2);m.write_byte(a+14,0x60);action(EasyEndingAction::voice);if(const auto child=h.allocate_first()){m.write_byte(child,m.read_byte(child)+1);m.write_word(child+2,0x1e00);m.write_word(g+0x2a,m.read_word(g+0x2a)-0x100);m.write_byte(g+0x125,1);m.write_word(g-0x6cce,0);m.write_word(g+0x32,0x1e0e);m.write_word(g+0x3a,0x4261);m.write_word(g+0x2a,0x240);}m.write_word(a+0x10,0xc0);h.actor_init(a,16,0x7ab6c);return true;
 case 2:m.write_byte(a+14,m.read_byte(a+14)-1);if(m.read_byte(a+14)){h.actor_step(a);return true;}m.write_byte(a+4,4);m.write_word(g+0x2a,m.read_word(g+0x2a)+0x100);m.write_byte(g+0x125,0);m.write_word(g+0x2a,0x10);m.write_word(g+0x32,0x1b0e);m.write_word(g+0x3a,0x2461);action(EasyEndingAction::large_map_palette);for(unsigned n=0;n<32;n+=2)m.write_word(0x90c160 + n,m.read_word(0xa0c20 + n)|0xf000);
  if(const auto child=h.allocate_third()){m.write_byte(child,m.read_byte(child)+1);m.write_word(child+2,2);m.write_word(child+0x1a,m.read_byte(g+0xbf)?0xd9:0xd7);}if(const auto child=h.allocate_first()){m.write_byte(child,m.read_byte(child)+1);m.write_word(child+2,0x2500);}h.actor_init(a,9,0x7ab6c);return true;
 case 4:m.write_long(a+0x10,m.read_long(a+0x10)+m.read_long(a+0x20));if(std::int16_t(m.read_word(a+0x10))<=-64)source_ending_to_game_over(g,m);else h.actor_step(a);return true;
 default:return false;
 }}
 EasyEndingResult run(){if(clearing)return EasyEndingResult::unsupported;switch(m.read_word(g+16)){case 0:m.write_word(g+16,2);action(EasyEndingAction::reset_pools);clearing=true;return EasyEndingResult::wait_object_clear;case 2:frame();return EasyEndingResult::complete;default:return EasyEndingResult::unsupported;}}
};
} // namespace puzzle_fighter
