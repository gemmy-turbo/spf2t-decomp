// SPDX-License-Identifier: GPL-3.0-only
// USA A03C..A1DA,20DE0 and A1F8..A248. Native source-entry continuation.
#pragma once
#include "arcade_match_intro.hpp"
#include "global_rng.hpp"
namespace puzzle_fighter {
enum class ArcadeEntryResult {complete,clear_screen,unsupported};
// BAB2 consumes only byte+81 of its inherited source A0. Keep that observable
// value as typed context, not a CPU register or an invented player pointer.
struct ArcadeOrderContext {
 enum class Origin {unavailable,entry_palette,object_list_reset,join_overlay_cleanup,explicit_source_owner};
 Origin origin=Origin::unavailable;
 std::uint8_t owner=0;
 static ArcadeOrderContext explicit_owner(std::uint8_t value){return {Origin::explicit_source_owner,value};}
};
// 1495A always ends at2D890, whose final LEA leaves the raw-list pointer.
// Use only after that caller and before an A0-changing overlay/statistics path.
template<class M> ArcadeOrderContext arcade_order_after_object_composition(std::uint32_t g,M&m){
 return {ArcadeOrderContext::Origin::object_list_reset,m.read_byte(g+0x5a95)};
}
// The live-join handoff is the main frame which executes overlay kind2's
// terminal4/6 cleanup.61CA selects A0=6448;6526 does not alter it.
template<class M> ArcadeOrderContext arcade_order_after_join_overlay_cleanup(M&m){
 return {ArcadeOrderContext::Origin::join_overlay_cleanup,m.read_byte(0x64c9)};
}
namespace arcade_entry_detail {struct PaletteHooks {};}
template<class M> void initialize_arcade_entry_palettes(std::uint32_t g,M&m){
 arcade_entry_detail::PaletteHooks h;ArcadeMatchIntro<M,decltype(h)>{m,h,g}.intro_palettes();
}
// H::player_join_accounting() is source1F15A's active hardware body. Source
// g86==0 returns before all hardware access, so that profile needs no callback
// action. Other profiles must bind the real body or explicitly reject it.
template<class M,class H> struct ArcadeEntry {
 M&m;H&h;std::uint32_t g;
 bool awaiting_object_clear=false;
 ArcadeOrderContext order{};
 unsigned b(unsigned o){return m.read_byte(g+o);}unsigned w(unsigned o){return m.read_word(g+o);}
 void byte(unsigned o,unsigned v){m.write_byte(g+o,std::uint8_t(v));}void word(unsigned o,unsigned v){m.write_word(g+o,std::uint16_t(v));}
 unsigned random(){const auto r=global_random(w(0xae));word(0xae,r.seed);return r.value;}
 // 1565A, DATA table reached by LEA. This consumes no random draw.
 void start_skill(){const auto level=m.read_word(0x15834+b(0xbe)*2);word(0x12e,level);m.write_word(g-0x733a,std::uint16_t(level<<3));}
 ArcadeEntryResult begin(){
  if(awaiting_object_clear)return ArcadeEntryResult::unsupported;
  word(0,w(0)+2);clear_scene_records(g,m,true);
  for(unsigned o:{0x8au,0x8bu,0xacu,0xadu,0x124u,0xb8u,0xbau,0x104u,0xd8u,0x148u,0xd6u,0x14fu,0x150u,0xe0u})byte(o,0);
  word(0xb4,0);word(0x138,0);word(0x13e,0);byte(0x8d,b(0xa8));byte(0x8f,1);byte(0xbd,1);word(0x20,0x92a0);byte(0x132,255);byte(0xdf,b(0xde));byte(0x144,b(0xd4));byte(0x14e,b(0xd5));word(0x114,8);m.write_byte(g-0x7374,1);byte(0xbe,b(0xa0));byte(0x11f,b(0x98));
  byte(0x122,random()&0x70);byte(0x153,random()&0x3f);
  for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400,a=g+0xb00+side*0x100;m.write_byte(p,1);m.write_byte(p+2,side);m.write_byte(p+0x81,side);m.write_byte(p+0x82,side+2);m.write_byte(p+0x9e,1);m.write_byte(p+0x98,0);m.write_long(p+0x90,0);m.write_long(p+0xae,0);m.write_byte(p+0xb2,0);m.write_byte(p+0xb3,0);m.write_byte(p+0xb4,0);m.write_word(p+0x9a,w(0x58+side*4));m.write_word(p+0x32,std::uint16_t(a));m.write_byte(a,1);m.write_byte(a+2,side);m.write_word(a+0x32,std::uint16_t(p));}
  word(0x334,0x8700);word(0x734,0x8300);
  for(unsigned side=0;side<2;++side)if(b(0xa8)&(1u<<side)){const auto p=g+0x300+side*0x400;m.write_byte(p+0x80,2);m.write_byte(p+0x84,6);m.write_byte(p+0x9e,0);m.write_byte(p+0xa0,0);if(b(0x86))h.player_join_accounting();}
  word(0x32,0x1b0e);word(0x3a,0x2461);word(0x20,0x92a0);word(0x22,0);word(0x24,0x100);start_skill();awaiting_object_clear=true;order={};return ArcadeEntryResult::clear_screen;
 }
 // Caller completes the shared MatchObjectClear2CEE4 continuation first.
 ArcadeEntryResult resume_after_object_clear(){
  if(!awaiting_object_clear)return ArcadeEntryResult::unsupported;
  initialize_arcade_entry_palettes(g,m);awaiting_object_clear=false;
  const auto stage=std::uint16_t(w(0x100));const auto offset=std::int16_t(std::uint16_t((stage>>7)|(stage<<9)));const auto final_palette_cursor=0xa5ac0u+std::uint32_t(std::int32_t(offset))+0x400;
  order={ArcadeOrderContext::Origin::entry_palette,m.read_byte(final_palette_cursor+0x81)};return ArcadeEntryResult::complete;
 }
 // A1F8. The explicit context overload supports separately proven source
 // handoffs without assuming P1 or P2 when the incoming A0 owner differs.
 ArcadeEntryResult prepare_selection(ArcadeOrderContext incoming){
  if(awaiting_object_clear||incoming.origin==ArcadeOrderContext::Origin::unavailable)return ArcadeEntryResult::unsupported;
  word(4,w(4)+2);byte(0xab,1);for(unsigned o:{0x125u,0x14du,0x133u,0x301u,0x701u,0xb01u,0xc01u})byte(o,0);
  byte(0x132,incoming.owner);const auto index=b(0x122);for(unsigned n=0;n<16;n+=4)m.write_long(g-0x735a+n,m.read_long(0xbc78+index+n));
  if(b(0x139)&&!(w(0x110)&0x100))m.write_byte(g-0x7354,8);
  if(b(0x138))m.write_byte(g-0x7352,9);
  clear_scene_records(g,m,false);order={};return ArcadeEntryResult::complete;
 }
 ArcadeEntryResult prepare_selection(){
  // A002/1760, inactive1F104,12F6 saved registers,2CE9E and inactive61CA
  // preserve the20DE0 palette cursor. Active stats/overlay need new context.
  if(b(0x86)||b(0x2900))return ArcadeEntryResult::unsupported;
  return prepare_selection(order);
 }
};
} // namespace puzzle_fighter
