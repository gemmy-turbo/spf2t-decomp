// SPDX-License-Identifier: GPL-3.0-only
// USA A262/A3A2 match setup/intro and B372/B4D0/B522 static result graphics.
#pragma once
#include "scene_reset.hpp"
#include "round_scene_picture.hpp"
#include "attack_display.hpp"
#include "arcade_match_flow.hpp"
#include "sound_requests.hpp"
namespace puzzle_fighter {
enum class MatchIntroAction {initialize_players,cpu_match_level,stage_setup,round_labels,course_label,announcements_and_stage_actors,first_pool,second_pool,stage_tick,objects};
// Real native bodies are required for each typed action. No silent fallback.
template<class M,class H> struct ArcadeMatchIntro {
 M&m;H&h;std::uint32_t g;
 unsigned b(unsigned o){return m.read_byte(g+o);}unsigned w(unsigned o){return m.read_word(g+o);}
 void byte(unsigned o,unsigned v){m.write_byte(g+o,std::uint8_t(v));}void word(unsigned o,unsigned v){m.write_word(g+o,std::uint16_t(v));}void next(unsigned o){word(o,w(o)+2);}
 void action(MatchIntroAction a){h.action(a);}
 bool below(){word(0x12,w(0x12)-1);return(w(0x12)&0x8000)!=0;}
 static int sw(std::uint16_t v){return std::int16_t(v);}
 void copy_palette(std::uint32_t source,std::uint32_t to,unsigned blocks){for(unsigned n=0;n<blocks*32;n+=4)m.write_long(to+n,m.read_long(source+n)|0xf000f000u);}
 //20E80 common object palettes,20E16 characters,20E4A stage object palettes.
 void common_palette(){copy_palette(0xa0b00,0x90c000,10);copy_palette(0xaa620,0x90c140,1);copy_palette(0xaa600,0x90c160,1);copy_palette(0xa0c80,0x90c280,6);}
 std::uint32_t character_palette(unsigned character,unsigned alternate){auto index=std::uint16_t(character*2);index=std::uint16_t((index&0xff00)|std::uint8_t(index+alternate));return 0xa0000u+sw(std::uint16_t(index<<7));}
 void player_palettes(){for(unsigned side=0;side<2;++side)copy_palette(character_palette(b(0x382+side*0x400),b(0x39f+side*0x400)),0x90c180+side*0x80,4);}
 void stage_object_palette(){if(w(0x100)==16)copy_palette(0xaa1e0,0x90c340,2);copy_palette(0xa0d40u+sw(std::uint16_t(w(0x100)<<6)),0x90c380,4);}
 void gameover_palette(){common_palette();stage_object_palette();player_palettes();}
 void stage_map_palette(bool large){const auto index=std::uint16_t(w(0x100));const auto offset=sw(std::uint16_t((index>>7)|(index<<9)));copy_palette((large?0xa5ac0u:0xa22c0u)+offset,large?0x90cc00u:0x90c800u,32);}
 void intro_palettes(){gameover_palette();copy_palette(0xa1ec0,0x90c400,32);stage_map_palette(false);stage_map_palette(true);}
 void brightness(std::uint32_t to,unsigned bytes,std::uint32_t mask,bool brighten){for(unsigned n=0;n<bytes;n+=4){const auto value=m.read_long(to+n);m.write_long(to+n,brighten?value|mask:value&mask);}}
 void meet_last_opponent(){if(b(0xac)==3)return;const auto opponent=g+((b(0xac)&1)?0x700:0x300);if(m.read_byte(opponent+0x82)==10){word(0x13e,2);byte(0x156,255);}}
 void meet_challenger(){
  if(b(0xbf)||b(0xac)==3){return;}const auto player=g+((b(0xac)&1)?0x300:0x700),opponent=g+((b(0xac)&1)?0x700:0x300);
  if(w(0x110)&0x100)return;
  if(b(0x139)){word(0x13e,0);m.write_byte(opponent+0x82,8);word(0x100,16);return;}
  if(b(0xad)||w(0x112)!=6||!m.read_byte(player+0xb3)||m.read_byte(player+0xae)<4||m.read_byte(player+0xb0)<20||!m.read_byte(player+0xb2))return;
  word(0x13e,4);
 }
 // Original2CF96 only empties the text plane, preserving both background maps.
 void clear_text(){clear_source_text_plane(m);}
 bool start(){if(b(0x83))return false;start_match_state(g,m);meet_last_opponent();meet_challenger();clear_text();action(MatchIntroAction::initialize_players);action(MatchIntroAction::cpu_match_level);return true;}
 std::uint32_t first(unsigned kind){const auto a=h.allocate_first();if(a){m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,kind);}return a;}
 void stands(){for(unsigned side=0;side<2;++side){const auto a=first(0x2200);if(!a)return;m.write_word(a+0x10,side?0x130:0x50);m.write_word(a+0x14,0x40);m.write_word(a+0xc,0xff15);m.write_word(a+0x32,std::uint16_t(g+0x300+side*0x400));}}
 void shown(){next(0x10);word(0x20,0x9280);byte(0x14d,1);}
 bool tick(){
  if(!w(0xc)){if(h.overlay(1,g+0x2900)){next(0xc);m.write_long(g+0x10,0);m.write_long(g+0x14,0);byte(0x14d,0);word(0x290c,0);byte(0x8c,255);if(!b(0xa6)){queue_sound_request(g,m,{0xff05,0,0});queue_sound_request(g,m,{15,0,0},true);}}return true;}
  if(w(0xc)!=2)return false;
  if(b(0x8a))word(0x20,0x90c0);
  else switch(w(0x10)){
   case 0:case 2:next(0x10);break;
   case 4:next(0x10);word(0x20,0x90c0);action(MatchIntroAction::stage_setup);action(MatchIntroAction::round_labels);action(MatchIntroAction::course_label);intro_palettes();copy_palette(0xab7e0,0x90c2a0,1);copy_palette(0xa0c20,0x90c2c0,1);action(MatchIntroAction::announcements_and_stage_actors);break;
   case 6:if(!b(0x8c)){next(0x10);word(0x12,32);byte(0x8f,0);byte(0xbd,0);byte(0x14d,0);if(!first(0x400)||!first(0x401))break;for(unsigned side=0;side<2;++side){const auto a=first(0x501);if(!a)break;m.write_byte(a+0xa,side);m.write_word(a+0x32,std::uint16_t(g+0x300+side*0x400));}}break;
   case 8:if(arcade_flow_detail::pressed(g,m)){next(0x10);stands();shown();}else if(below()){next(0x10);word(0x12,63);word(0x48,0x9280);stands();brightness(0x90c340,6*32,0x3fff3fff,false);brightness(0x90c800,0x800,0x3fff3fff,false);}break;
   case 10:if(arcade_flow_detail::pressed(g,m)||below())shown();break;
   case 12:case 16:next(0x10);word(0x20,0x92a0);break;
   case 14:next(0x10);word(0x20,0x9280);break;
   case 18:next(0x10);word(0x20,0x9280);brightness(0x90c000,0x1000,0xf000f000,true);break;
   case 20:next(0x10);word(0x20,0x90c0);word(0x12,120);break;
   case 22:if(arcade_flow_detail::pressed(g,m)||below()){next(0x10);word(0x12,24);byte(0x14d,2);}break;
   case 24:if(below())next(0x10);break;
   case 26:next(8);for(unsigned o:{0xcu,0x10u,0x14u})m.write_long(g+o,0);byte(0x14d,0);byte(0x8f,1);byte(0xbd,255);player_palettes();common_palette();break;
   default:return false;
  }
  action(MatchIntroAction::first_pool);action(MatchIntroAction::second_pool);action(MatchIntroAction::stage_tick);action(MatchIntroAction::objects);return true;
 }
 // Original totals use an eight-digit BCD scratch and do not erase skipped
 // leading zero cells; the background panel supplies those cells.
 void digit(std::uint32_t to,unsigned value){const auto tile=m.opcode_word(0xb4b0+(value&15)*2);m.write_word(to,tile);m.write_word(to+2,0x16);m.write_word(to+4,tile+0x10);m.write_word(to+6,0x16);}
 std::uint32_t number(std::uint32_t to,std::uint32_t packed){bool begun=false;for(int place=7;place>0;--place,to+=0x80){const auto d=(packed>>(place*4))&15;if(d||begun){begun=true;digit(to,d);}}digit(to,packed&15);return to;}
 std::uint32_t decimal(std::uint32_t value){attack_display_detail::bcd(value,g,m);return m.read_long(g-0x6ca8);}
 void show_totals(){const auto p=std::uint32_t(sw(w(0x13a)));if(m.read_byte(p+0x9e))return;
  auto table=0x84560u+sw(m.read_word(0x84572));if(w(0x90))table+=16;auto source=m.read_long(table+4);const auto rows=unsigned(m.read_word(source+4))+1;source+=6;auto to=m.read_byte(p+0x81)?0x902130u:0x902e30u;
  for(unsigned row=0;row<rows;++row,to+=0x80)for(unsigned tile=0;tile<8;++tile,source+=4)m.write_long(to+tile*4,m.read_long(source));
  const auto base=to-0x200;number(base,decimal(m.read_long(p+0xaa)));number(base+8,decimal(m.read_byte(p+0xaf)));number(base+16,decimal(m.read_byte(p+0xb1)));decimal(b(0x108));to=number(base-0x168,m.read_byte(g-0x6ca5))+0x80;m.write_long(to,0x04dd0014);decimal(b(0x109));to+=0x80;digit(to,m.read_byte(g-0x6ca5)>>4);digit(to+0x80,m.read_byte(g-0x6ca5)&15);
 }
 void clear_totals(){copy_palette(0xa0be0,0x90c0e0,1);for(unsigned o:{0x301u,0x701u,0xb01u,0xc01u})byte(o,0);for(unsigned row=0;row<44;++row)for(unsigned tile=0;tile<8;++tile)m.write_long(0x902130+row*0x80+tile*4,0x00200000);}
 void loser_colours(std::uint32_t actor){copy_palette(character_palette(b(0x154),b(0x155)),m.read_byte(actor+0x81)?0x90c200:0x90c180,4);copy_palette(0xa0be0,0x90c140,1);copy_palette(0xa0c20,0x90c160,1);}
};
} // namespace puzzle_fighter
