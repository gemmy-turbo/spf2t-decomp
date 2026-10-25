// SPDX-License-Identifier: GPL-3.0-only
// USA173CC..1781C: original top-five scoreboard scene and source text helpers.
#pragma once
#include "source_score_actors.hpp"
namespace puzzle_fighter {
template<class M> void draw_source_big_notice(M&m,unsigned index){
 const bool erase=index&128;const auto doubled=std::uint8_t(index*2);auto from=std::uint32_t(0x20aec + std::int16_t(m.read_word(0x20aec + doubled)));
 for(unsigned budget=0;budget<4096;){const auto x=m.read_byte(from++),y=m.read_byte(from++);auto out=0x900000u+(x<<7)+y;const auto attr=m.read_byte(from++);
  for(;;){if(++budget>=4096)throw std::runtime_error("Source large text budget");const auto glyph=m.read_byte(from++);if(!glyph)return;if(glyph==127)break;
   if(erase){m.write_long(out,0x00200000u|attr);m.write_long(out+4,0x00200000u|attr);}
   else{const auto value=std::uint8_t(glyph-32);const auto tile=(std::int8_t(value)<0)?0:m.read_word(0x5952 + value*2);if(tile){const auto attribute=std::uint16_t(std::int16_t(std::int8_t(attr)));m.write_word(out,tile);m.write_word(out+2,attribute);m.write_word(out+4,tile+16);m.write_word(out+6,attribute);}else{m.write_long(out,0x20000000);m.write_long(out+4,0x20000000);}}
   out+=128;
  }
 }throw std::runtime_error("Source large text unterminated");
}
template<class M> void source_score_actor_shape(std::uint32_t a,M&m,unsigned index){const auto at=0x24464 + m.read_byte(a+3)*4;const auto table=(std::uint32_t(m.opcode_word(at))<<16)|m.opcode_word(at+2);const auto animation=std::uint32_t(table+std::int16_t(m.read_word(table+index*2)));m.write_long(a+0x1c,animation);m.write_long(a+0x18,m.read_long(animation));}
template<class M> void draw_source_score_stage_totals(std::uint32_t g,M&m){ //17760
 auto from=0x84560u+std::int16_t(m.read_word(0x84574));if(m.read_word(g+0x90))from+=16;from=m.read_long(from+4);const auto count=m.read_word(from+4);from+=6;auto out=0x902ec8u;
 for(unsigned n=0;n<=count;++n,out+=128,from+=16){for(unsigned j=0;j<16;j+=4)m.write_long(out+j,m.read_long(from+j));}out+=128;
 const auto digit=[&](std::uint32_t to,unsigned n){const auto tile=m.opcode_word(0x177fe + n*2);m.write_word(to,tile);m.write_word(to+2,22);m.write_word(to+4,tile+16);m.write_word(to+6,22);};
 for(unsigned field=0;field<2;++field){attack_display_detail::bcd(m.read_byte(g+0x57b6 + field),g,m);const auto packed=m.read_byte(g-0x6ca5);if(packed>>4)digit(out+field*8,packed>>4);digit(out+128+field*8,packed&15);}
}
enum class ScoreAction {reset_pools,reset_pools_retained,score_song,stage_picture,stage_tick,score_palettes,gem_coordinates,stage_actors,first_pool,second_pool,third_pool,objects,fade_in,fade_out,score_reveal_song};
enum class ScoreResult {complete,wait_object_clear,unsupported};
template<class M,class H> struct SourceScoreboard {
 M&m;H&h;std::uint32_t g;bool clearing=false,clear_fresh=false;
 unsigned b(unsigned o){return m.read_byte(g+o);}unsigned w(unsigned o){return m.read_word(g+o);}void byte(unsigned o,unsigned v){m.write_byte(g+o,std::uint8_t(v));}void word(unsigned o,unsigned v){m.write_word(g+o,std::uint16_t(v));}void next(unsigned o=12){word(o,w(o)+2);}void action(ScoreAction a){h.action(a);}
 void pools(){action(ScoreAction::first_pool);action(ScoreAction::second_pool);action(ScoreAction::third_pool);action(ScoreAction::objects);}
 bool expired(){word(14,w(14)-1);return std::int16_t(w(14))<0;}
 void row(unsigned kind,unsigned variant=0){const auto a=h.allocate_first();if(!a)return;m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,(kind<<8)|variant);}
 ScoreResult leave(){if(!b(0x83)){word(4,12);clear_fresh=false;}else{next(0);m.write_long(g+4,0);clear_fresh=true;}for(unsigned o=8;o<24;o+=4)m.write_long(g+o,0);byte(0x4781,0);byte(0x4782,0);word(0x20,0x92a0);word(0x48,0x92a0);clearing=true;return ScoreResult::wait_object_clear;}
 ScoreResult resume_after_clear(){if(!clearing)return ScoreResult::unsupported;clearing=false;action(clear_fresh?ScoreAction::reset_pools:ScoreAction::reset_pools_retained);return ScoreResult::complete;}
 ScoreResult run(){
  if(clearing)return ScoreResult::unsupported;
  if(w(8)==4)return leave();
  if(w(8)==0){next(8);word(0x20,0x92a0);word(0x48,0x92a0);if(!b(0x83)&&!b(0x150))return leave();action(ScoreAction::reset_pools);action(ScoreAction::score_song);word(0x100,b(0x57b4));const auto a=g+0xb00;m.write_byte(a,1);for(unsigned o:{1u,2u,11u})m.write_byte(a+o,0);m.write_word(a+0x30,0xa000);m.write_word(a+0xc,0);m.write_word(a+0x10,0xc0);m.write_word(a+0x14,0x58);m.write_word(a+0xd6,0x58);for(unsigned o:{3u,0x82u})m.write_byte(a+o,b(0x57b2));byte(0x382,b(0x57b2));m.write_byte(a+0x9f,b(0x57b3));byte(0x39f,b(0x57b3));source_score_actor_shape(a,m,23);action(ScoreAction::stage_picture);byte(0x10d,3);action(ScoreAction::stage_tick);action(ScoreAction::score_palettes);action(ScoreAction::gem_coordinates);action(ScoreAction::stage_actors);
   if(b(0x581c)>=5){for(unsigned v=0;v<4;++v){const auto child=h.allocate_first();m.write_byte(child,m.read_byte(child)+1);m.write_word(child+2,0x2f00 + v);m.write_word(child+14,24+v*4);}const auto child=h.allocate_first();m.write_byte(child,m.read_byte(child)+1);m.write_word(child+2,0x304);m.write_byte(child+10,0x2c);m.write_word(child+0x10,0);m.write_word(child+0x14,0);source_palette_preserving_brightness(m,0xab200,0x90c080);}return ScoreResult::complete;
  }
  if(w(8)!=2){return ScoreResult::unsupported;}if(!b(0x83)&&(b(0x60)&3))return leave();const auto a=g+0xb00;
  switch(w(12)){
  case 0:next();word(0x20,0x90c0);action(ScoreAction::fade_in);break;
  case 2:if(!b(0x4782)){next();word(14,180);draw_source_big_notice(m,1);}break;
  case 4:if(expired()){next();word(14,60);draw_source_big_notice(m,0x81);}break;
  case 6:if(expired()){next();word(0x20,0x9280);word(0x48,0x9280);}break;
  case 8:{next();byte(0x10d,0);action(ScoreAction::stage_tick);action(ScoreAction::score_reveal_song);m.write_byte(a+1,1);draw_source_big_notice(m,2);draw_source_score_stage_totals(g,m);const auto caption=h.allocate_third();if(caption){m.write_byte(caption,m.read_byte(caption)+1);m.write_word(caption+2,0x300);if(b(0x581c)>=2){const auto label=h.allocate_first();m.write_byte(label,m.read_byte(label)+1);m.write_word(label+2,0x304);m.write_byte(label+10,11);m.write_word(label+12,0xff1c);m.write_word(label+0x10,0x40);m.write_word(label+0x14,0x78);const auto actor=h.allocate_first();m.write_byte(actor,m.read_byte(actor)+1);m.write_word(actor+2,0x2201);m.write_word(actor+0x10,0x110);m.write_word(actor+0x14,0x80);m.write_word(actor+12,0xff06);m.write_word(actor+0x32,0x8300);}}break;}
  case 10:next();word(0x20,0x90c0);word(14,180);h.actor_step(a);break;
  case 12:if(expired()){next();word(14,180);byte(0x107,1);byte(0x10c,1);word(0x13a,0x8300);word(0x13c,0x8700);source_score_actor_shape(a,m,24);}h.actor_step(a);break;
  case 14:if(expired()){next();action(ScoreAction::fade_out);}h.actor_step(a);break;
  case 16:if(!b(0x4782)){next(8);word(0x20,0x92a0);word(0x48,0x92a0);}h.actor_step(a);break;
  default:return ScoreResult::unsupported;
  }pools();return ScoreResult::complete;
 }
};
} // namespace puzzle_fighter
