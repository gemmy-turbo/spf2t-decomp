// SPDX-License-Identifier: GPL-3.0-only
// USA ending first05/subtype5 and first41 palette/figure actors.
#pragma once
#include "scene_actor_tasks.hpp"
#include "arcade_stage_interlude.hpp"
namespace puzzle_fighter {
template<class M,class H> bool source_ending_portrait(std::uint32_t a,std::uint32_t g,M&m,H&h){
 if(m.read_byte(a+2)!=5||m.read_byte(a+3)!=5)return false;
 const auto phase=m.read_byte(a+4);if(!phase){m.write_byte(a+4,2);m.write_word(a+0x30,0x6000);m.write_byte(a+9,0);m.write_word(a+0xc,0xff00);m.write_word(a+0x10,0xc0);m.write_word(a+0x14,0x80);const auto ch=m.read_byte(g+0x130),alt=m.read_byte(g+0x123);SceneActorTask<M,H> helper{m,h,g,a};helper.portrait_palette(ch,alt,0x90c000,10,0xc000);
  // The producer allocates05/05 as its first active first-pool record; source
  // A4 still owns that record when23F1A samples+9F for Dan's special palette.
  if(ch==10){const auto from=m.read_byte(a+0x9f)?0xa0a80u:0xa0a00u;for(unsigned n=0;n<32;n+=2)m.write_word(0x90c000+n,m.read_word(from+n)|0xc000);}helper.portrait_face(ch);helper.static_shape(ch);return true;}
 if(phase==2){h.display(a);return true;}if(phase==4||phase==6){h.free(a);return true;}return false;
}
template<class M,class H> struct EndingPaletteActor {
 M&m;H&h;std::uint32_t g,a;
 unsigned b(unsigned o){return m.read_byte(a+o);}unsigned w(unsigned o){return m.read_word(a+o);}void byte(unsigned o,unsigned v){m.write_byte(a+o,std::uint8_t(v));}void word(unsigned o,unsigned v){m.write_word(a+o,std::uint16_t(v));}void lng(unsigned o,std::uint32_t v){m.write_long(a+o,v);}void next(unsigned o=5){byte(o,b(o)+2);}
 std::uint32_t code_long(std::uint32_t p){return(std::uint32_t(m.opcode_word(p))<<16)|m.opcode_word(p+2);}
 void palette(){auto from=m.read_long(a+0x20);const auto variant=b(3);auto to=0x90c000 + variant*32;const auto bright=std::uint16_t(b(0x28)<<12);for(unsigned n=0;n<32;n+=2)m.write_word(to+n,m.read_word(from+n)|bright);}
 bool tick(){if(b(3)>11)return false;const auto phase=b(4),variant=b(3);if(!phase){next(4);word(0x30,0xa000);byte(0xc,255);byte(0x28,0);byte(0x29,3);lng(0x20,variant==11?0xa0460u:0xa0000u+variant*256);word(0x24,336);const auto index=variant*4;word(0xe,m.opcode_word(0x29a94 + variant*2));byte(9,m.opcode_byte(0x29b0f + index));byte(0xb,m.opcode_byte(0x29b10 + index));byte(0xd,m.opcode_byte(0x29b11 + index));word(0x10,m.opcode_word(0x29b3e + index));word(0x14,m.opcode_word(0x29b40 + index));const auto table=code_long(0x29b6e + index);lng(0x2c,table);h.init(a,m.opcode_byte(0x29b0e + index),table);return true;}
  if(phase==4){next(4);return true;}if(phase==6){h.free(a);return true;}if(phase!=2)return false;
  if(!(w(0x24)&0x8000))word(0x24,w(0x24)-1);
  switch(b(5)){
  case 0:word(0xe,w(0xe)-1);if(w(0xe)&0x8000){next();palette();}return true;
  case 2:byte(0x29,b(0x29)-1);if(!b(0x29)){byte(0x29,3);byte(0x28,b(0x28)+1);}byte(0x28,b(0x28)&15);if(b(0x28)==15)next();palette();[[fallthrough]];
  case 4:if(!w(0x24)){next();if(!variant)queue_stage_interlude_cue(g,m,0x1b);if(variant!=11)h.init(a,m.opcode_byte(0x29c34 + variant),m.read_long(a+0x2c));}h.step(a);h.display(a);return true;
  case 6:h.step(a);h.display(a);return true;
  default:return false;
  }
 }
};
template<class M,class H> bool source_ending_actor(std::uint32_t a,std::uint32_t g,M&m,H&h){if(m.read_byte(a+2)==0x41)return EndingPaletteActor<M,H>{m,h,g,a}.tick();return source_ending_portrait(a,g,m,h);}
template<class M> void allocate_source_ending_palette_actors(std::uint32_t g,M&m,std::uint32_t source_parent){for(unsigned n=0;n<12;++n){const auto a=allocate_scene_first(g,m);if(!a)return;m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,0x4100 + n);m.write_word(a+0x32,source_parent);}}
} // namespace puzzle_fighter
