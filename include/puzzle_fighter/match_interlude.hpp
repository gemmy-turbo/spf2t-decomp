// SPDX-License-Identifier: GPL-3.0-only
// USA6634(index3),61CA->653E and BA44. No menu/ladder scene execution.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
// In the normal A78C->BDDA caller, preceding61CA leaves A6 at the overlay.
// Request deliberately does not reset its state, duration or palette script.
template<class M> bool request_source_interlude(std::uint32_t g,M&m){
 const auto a=g+0x2900;if(m.read_byte(a))return false;
 m.write_byte(a,1);m.write_byte(a+2,3);m.write_byte(a+0xa,0);return true;
}
namespace match_interlude_detail {
inline int sw(std::uint16_t v){return v&0x8000?int(v)-65536:int(v);}
//2D3B6: independent eight-byte palette animation state at overlay+20.
// Unlike2D334 it ORs F000 into source colors, not preserved destination bits.
template<class M> void palette_step(std::uint32_t a,M&m){
 const auto left=std::uint16_t(m.read_word(a+4)-1);m.write_word(a+4,left);if(left)return;
 auto frame=m.read_long(a)+8;if(m.read_byte(a+6)&0x80)frame=m.read_long(frame);
 m.write_long(a,frame);m.write_long(a+4,m.read_long(frame));auto blocks=m.read_long(frame+4);
 for(;;){
  auto dest=0x90c000u+std::uint32_t(sw(m.read_word(blocks)));const auto index=m.read_word(blocks+2),descriptor=m.read_word(blocks+4);blocks+=6;
  const auto table=0x2d3a6u+std::uint32_t(sw(index));auto source=((std::uint32_t(m.opcode_word(table))<<16)|m.opcode_word(table+2))+(descriptor&0xfffeu);
  for(unsigned i=0;i<16;++i,source+=2,dest+=2)m.write_word(dest,m.read_word(source)|0xf000);
  if(descriptor&1)return;
 }
}
}
// True is inactive or handled source state. False exposes an unsupported
// overlay kind/state without replacing it. H supplies audited ActorTasks.step.
template<class M,class H> bool tick_source_overlay(std::uint32_t g,M&m,H&h){
 const auto a=g+0x2900;if(!m.read_byte(a))return true;if(m.read_byte(a+2)!=3)return false;
 switch(m.read_byte(a+4)){
 case 0:
  m.write_byte(a+4,2);m.write_byte(a+1,1);m.write_byte(a+9,0);m.write_byte(a+0xb,0);m.write_word(a+0xc,0);m.write_word(a+0x10,0);m.write_word(a+0x14,0);m.write_word(a+0x30,0xe000);m.write_word(a+0xe,0x80);m.write_long(a+0x1c,0x67b6);
  for(unsigned block=0;block<4;++block)for(unsigned i=0;i<0x400;i+=4){const auto p=0x90c000+block*0x400+i;m.write_long(p,m.read_long(p)&0x3fff3fff);}
  m.write_long(a+0x20,0x6708);m.write_long(a+0x24,m.read_long(0x6708));return true;
 case 2:{
  const auto sub=m.read_byte(a+5);if(sub!=0&&sub!=2)return false;
  const auto timer=std::uint16_t(m.read_word(a+0xe)-1);m.write_word(a+0xe,timer);
  if(sub==0){if(timer&0x8000)m.write_byte(a+5,2);match_interlude_detail::palette_step(a+0x20,m);}
  else if(timer&0x8000){m.write_byte(a+4,4);m.write_byte(a+1,0);m.write_byte(g+0x8a,0);}
  h.step(a);return true;
 }
 case 4:case 6:
  for(unsigned i=0;i<16;i+=4)m.write_long(a+i,0);
  m.write_byte(a+8,0x14);return true;
 default:return false;
 }
}
enum class InterludeSceneRequest {two_player_scene_selected,arcade_opponent_selected};
// BA44..BB14 is only selection RAM. The caller has already finished2CEE4
// and written20/48=9280. Starting the selected scene remains the host boundary.
template<class M> InterludeSceneRequest finish_source_interlude_scene(std::uint32_t g,M&m){
 auto player=g+0x300,opponent=g+0x700;const bool two_player=m.read_byte(g+0xac)==3;
 auto swap=[&]{const auto p=player;player=opponent;opponent=p;};
 if(two_player){if(m.read_byte(g+0x8d)==3){if(!m.read_byte(g+0x131))swap();}else if(m.read_byte(g+0x8d)&1)swap();}
 else{
  if(!(m.read_byte(g+0xac)&1))swap();
  const auto offset=m.read_byte(g+0x122);m.write_byte(g+0x132,m.read_byte(player+0x81));
  for(unsigned i=0;i<16;i+=4)m.write_long(g-0x735a+i,m.read_long(0xbc78+offset+i));
  const auto defeated=m.read_word(g+0x110);
  if(m.read_byte(g+0x139)&&!(defeated&0x100))m.write_byte(g-0x7354,8);
  if(m.read_byte(g+0x138))m.write_byte(g-0x7352,9);
  std::uint16_t index=0;std::uint8_t character=0;
  for(;;){character=m.read_byte(g-0x735a+std::uint32_t(match_interlude_detail::sw(index)));if(index>=m.read_word(g+0x114)||!(defeated&(1u<<(character&31))))break;++index;}
  m.write_word(g+0x112,index);m.write_byte(opponent+0x82,character);
 }
 m.write_word(g+0x100,unsigned(m.read_byte(opponent+0x82))*2);
 if(!two_player&&m.read_word(g+0x112)==8)m.write_word(g+0x100,0x12);
 return two_player?InterludeSceneRequest::two_player_scene_selected:InterludeSceneRequest::arcade_opponent_selected;
}
} // namespace puzzle_fighter
