// SPDX-License-Identifier: GPL-3.0-only
// Original USA 16DDC..17116: post-loss name entry and leaderboard insertion.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
// The caller owns player job +84. This routine advances it only at 16E96.
// Leaderboard records g+579A..57FA are existing source state, not host defaults.
template<class M> struct ArcadeNameEntry {
 M&m;std::uint32_t g,p;
 unsigned b(unsigned o){return m.read_byte(p+o);}unsigned w(unsigned o){return m.read_word(p+o);}
 void byte(unsigned o,unsigned v){m.write_byte(p+o,std::uint8_t(v));}void word(unsigned o,unsigned v){m.write_word(p+o,std::uint16_t(v));}
 void insert_score(){ // 16EAE, unsigned score ordering; equal scores insert first.
  auto to=g+0x57fa,from=to-16;const auto score=m.read_long(p+0x90);int remaining=4;
  while(score>=m.read_long(from)){for(unsigned n=0;n<16;n+=4)m.write_long(to+n,m.read_long(from+n));to=from;from-=16;if(--remaining<0)break;}
  const auto rank=unsigned(remaining+2);byte(0xf4,rank);m.write_long(to+4,m.read_long(p+0xf0));m.write_long(to,score);m.write_long(to+8,m.read_long(p+0xf6));m.write_long(to+12,m.read_long(p+0xfa));
  const auto other=g+(b(0x81)?0x300:0x700);const auto old=m.read_byte(other+0xf4);if(old&&old>=rank&&old!=6)m.write_byte(other+0xf4,old+1);
 }
 void finish(){ // 16E96
  byte(0x84,b(0x84)+2);byte(0x85,0);word(0x8a,0);m.write_long(p+0x8c,0);byte(0xf4,0);
 }
 struct TextCursor {std::uint32_t front,back;std::uint16_t front_tile=0,back_tile=0,front_attr=0,back_attr=0;};
 TextCursor cursor(unsigned offset){const auto side=b(0x81)?0x1000u:0u;return {0x902000+side+offset,0x900000+side+offset};}
 void lower_cell(TextCursor&c){ // 16FCC, also entered directly after rank cells.
  c.front_tile=std::uint16_t(c.front_tile-15);c.back_tile=std::uint16_t(c.back_tile-15);
  m.write_word(c.front,c.front_tile);m.write_word(c.front+2,c.front_attr);m.write_word(c.back,c.back_tile);m.write_word(c.back+2,c.back_attr);
  c.front_tile=std::uint16_t(c.front_tile+16);c.back_tile=std::uint16_t(c.back_tile+16);
  m.write_word(c.front+4,c.front_tile);m.write_word(c.front+6,c.front_attr);m.write_word(c.back+4,c.back_tile);m.write_word(c.back+6,c.back_attr);c.front+=128;c.back+=128;
 }
 void cell(TextCursor&c,unsigned tile){ // 16F9E
  c.front_tile=tile;c.back_tile=std::uint16_t(tile+0x180);
  m.write_word(c.front,c.front_tile);m.write_word(c.front+2,c.front_attr);m.write_word(c.back,c.back_tile);m.write_word(c.back+2,c.back_attr);
  c.front_tile=std::uint16_t(c.front_tile+16);c.back_tile=std::uint16_t(c.back_tile+16);
  m.write_word(c.front+4,c.front_tile);m.write_word(c.front+6,c.front_attr);m.write_word(c.back+4,c.back_tile);m.write_word(c.back+6,c.back_attr);c.front+=128;c.back+=128;lower_cell(c);
 }
 void draw_name(){ // 16F1E: LEA-derived 17118 table is DATA, not OPCODE.
  auto c=cursor(0x370);for(unsigned n=0;n<4;++n){const auto at=0x17118u+b(0xf0+n)*4;c.front_attr=c.back_attr=m.read_word(at+2);cell(c,m.read_word(at));}
 }
 void draw_rank(){ // 16F60: includes the source's extra 16FCC tail cell.
  auto c=cursor(0x170);c.front_attr=c.back_attr=31;const auto at=0x172b8u+std::uint32_t(std::int32_t(std::int8_t(b(0xf4)))*4);cell(c,m.read_word(at));cell(c,m.read_word(at+2));lower_cell(c);
 }
 void input(){ // 16FFA: original held-input repeat and cursor control.
  const auto keys=w(0x9a);if(!keys){byte(0x8c,0xf0);return;}word(0x8a,600);const auto repeat=b(0x8c)+16;byte(0x8c,repeat);if(repeat<256)return;
  auto at=p+0xf0+std::uint32_t(std::int32_t(std::int8_t(b(0xf5))));const auto value=m.read_byte(at);
  if(keys&0x7700){
   if(value==0x2f){do{m.write_byte(at++,0x2d);byte(0xf5,b(0xf5)+1);}while(b(0xf5)!=4);byte(0xf3,0x2d);return;}
   if(value==0x2e){m.write_byte(at,b(0xf5)==3?0x2d:0x30);byte(0xf5,b(0xf5)-1);if(b(0xf5)&128){byte(0xf5,0);m.write_byte(at,0x2e);}return;}
   m.write_byte(at+1,value);byte(0xf5,b(0xf5)+1);if(b(0xf5)==3)m.write_byte(at+1,0x2f);return;
  }
  if(keys&0x8000){word(0x8e,0);return;}
  if(keys&2){const auto v=std::uint8_t(value-1);m.write_byte(at,v);if(v&128)m.write_byte(at,0x2f);else if(b(0xf5)==3&&v<0x2e)m.write_byte(at,0x2f);return;}
  if(keys&1){const auto v=std::uint8_t(value+1);m.write_byte(at,v);if(v>0x2f)m.write_byte(at,b(0xf5)==3?0x2e:0);}
 }
 void sanitize_name(){ // 170E0: preserve the original 31 exact substitutions.
  for(unsigned n=0;n<4;++n)if(b(0xf0+n)>=0x2e)byte(0xf0+n,0x2d);
  const auto name=m.read_long(p+0xf0);for(unsigned n=0;n<31;++n){const auto at=0x172d4+n*8;if(m.read_long(at)==name){m.write_long(p+0xf0,m.read_long(at+4));return;}}
 }
 void commit(){byte(0x85,b(0x85)+2);byte(0x8a,60);byte(0xf3,0x2d);sanitize_name();const auto rank=b(0xf4);if(rank<6)m.write_long(g+0x579a+rank*16+4,m.read_long(p+0xf0));}
 bool tick(){switch(b(0x85)){
  case 0:byte(0x85,2);byte(0xf5,0);m.write_long(p+0xf0,0x0030302d);word(0x8e,2400);word(0x8a,600);byte(0x8c,0xf0);insert_score();if(b(0xf4)>=6){finish();return true;}m.write_byte(g+0x150,255);draw_rank();draw_name();return true;
  case 2:input();if(b(0xf5)==4){commit();return true;}word(0x8e,w(0x8e)-1);if(w(0x8e)&0x8000){commit();return true;}word(0x8a,w(0x8a)-1);if(w(0x8a)&0x8000){commit();return true;}draw_rank();draw_name();return true;
  case 4:byte(0x8a,b(0x8a)-1);if(b(0x8a)&128)byte(0x85,b(0x85)+2);if(m.read_byte(g+0x81)&1){draw_rank();draw_name();}return true;
  case 6:finish();return true;
  default:return false;
 }}
};
template<class M> bool tick_arcade_name_entry(std::uint32_t player,std::uint32_t g,M&m){return ArcadeNameEntry<M>{m,g,player}.tick();}
} // namespace puzzle_fighter
