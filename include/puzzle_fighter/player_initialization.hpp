// SPDX-License-Identifier: GPL-3.0-only
// USA5AD2..5F42 normal (non-attract) player/field initialization.
#pragma once
#include "piece_initialization.hpp"
#include <stdexcept>
namespace puzzle_fighter {
template<class M> void source_score_digits(std::uint32_t value,std::uint32_t out,unsigned palette,M&m){
 for(unsigned place=0;place<7;++place){const auto weight=m.read_long(0x1125c+place*4);unsigned digit=0;for(;;){const auto next=value-weight;if(next&0x80000000u)break;value=next;++digit;}
  m.write_word(out,m.read_word(0x11278+digit*4));m.write_word(out+2,palette);m.write_word(out+4,m.read_word(0x1127a+digit*4));m.write_word(out+6,palette);out+=0x80;
 }
}
template<class M,class H> void initialize_source_players(std::uint32_t g,M&m,H&h){
 if(m.read_byte(g+0x83))throw std::runtime_error("Attract player initialization is outside standalone source scene");
 const auto init=initialize_piece_streams(m.read_word(g+0xae));m.write_word(g+0xae,init.global_seed);
 for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400;m.write_word(p+0x144,init.player_seed);m.write_word(p+0x156,init.next[0]);m.write_word(p+0x158,init.next[1]);m.write_byte(p+0x380,init.order);for(unsigned n=0;n<4;++n)m.write_byte(p+0x350+n,init.colours[n]);}
 for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400,field=g+0x2b00+side*0x200,mirror=g+0x2f00+side*0x200;
  for(unsigned n=0;n<14;++n){const auto a=field+n*16;m.write_long(a,0x00ff0000);m.write_long(a+4,0);m.write_long(a+8,0);m.write_long(a+12,0xff);}
  for(unsigned n=0;n<13;++n){const auto a=mirror+16+n*16;m.write_long(a,0x00ff0000);m.write_long(a+4,0);m.write_long(a+8,0);m.write_long(a+12,0xff);}
  m.write_byte(p+0x171,1);for(unsigned n=0;n<4;++n){m.write_long(field+0xe0+n*4,0x00ff00ff);m.write_long(mirror+0xe0+n*4,0x00ff00ff);}
  m.write_word(p+0x150,side?0x448:0x48);m.write_word(p+0x172,side?0x388:0x248);m.write_long(p+0x382,0x900160+side*0x1000);m.write_byte(p+0x180,side+1);m.write_long(p+0x164,field+16);m.write_long(p+0x398,field+16);m.write_long(p+0x168,mirror+16);
  h.source_cpu_prefix(p);
  m.write_word(p+0x17e,0);m.write_word(p+0x10,3);m.write_word(p+0x14,0);m.write_byte(p+0x15e,0x3c);m.write_byte(p+0x18a,6);
  for(unsigned o:{0xd1u,0x171u,0x1a9u,0x176u,0x1a8u,0x19eu,0x1aau,0x1a1u,0x194u,0x204u,0x206u,0x208u,0x1e8u,0x1e9u,0x1eau,0x1ebu,0x1ecu,0x1edu,0x32cu,0x31au,0x260u,0x32eu,0x32fu,0x32du,0x348u,0x354u})m.write_byte(p+o,0);
  for(unsigned o:{0x184u,0x186u,0x1b0u,0x1b2u,0x1c8u,0x1fcu,0x1feu,0x200u,0x202u,0x1f4u,0x1f8u,0x248u,0x24au,0x152u,0x154u,0x30eu,0x310u,0x340u,0x342u})m.write_word(p+o,0);
  for(unsigned o:{0x376u,0x330u,0x334u,0x338u,0x33cu,0x344u})m.write_long(p+o,0);
  m.write_word(p+0x1f6,25);m.write_word(p+0x242,0xfff0);m.write_word(p+0x27c,1);m.write_word(p+0x312,4);
  for(unsigned o:{0x49eu,0x89eu,0x4eeu,0x8eeu})m.write_byte(g+o,0);
  h.source_fall_speed(p);h.source_initial_pacing(p);source_score_digits(m.read_long(p+0xaa),side?0x900d64:0x9007e4,side+1,m);m.write_long(p+0x160,g+(side?0x300:0x700));h.source_generate_next(p);
  if(side){m.write_byte(p-0x78fd,1);h.source_second_player_tail(p);}
 }
}
} // namespace puzzle_fighter
