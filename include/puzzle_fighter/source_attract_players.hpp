// SPDX-License-Identifier: GPL-3.0-only
// USA5AD2 complete attract profile, including source5F44 board presets.
#pragma once
#include "player_initialization.hpp"
namespace puzzle_fighter {
namespace source_attract_detail {
template<class M> void left_plain(std::uint32_t g,M&m){
 const std::uint32_t rows0[]={0xff0001u,0xb0000u,0x30000u,0xffu,0xff0001u,0x40000u,0x30004u,0x200ffu,0xff0001u,0x40002u,0x30009u,0x200ffu,0xff0001u,0x10001u,0x10003u,0x200ffu};
 for(unsigned i=0;i<16;++i)m.write_long(g+0x2ba0+i*4,rows0[i]);
}
template<class M> void right_plain(std::uint32_t g,M&m){
 const std::uint32_t rows0[]={0xff0009u,0x30000u,0x0u,0xffu,0xff0002u,0x1000au,0x10002u,0x200ffu,0xff0001u,0x30003u,0x40001u,0x200ffu,0xff0002u,0x20002u,0x20002u,0x200ffu};
 for(unsigned i=0;i<16;++i)m.write_long(g+0x2da0+i*4,rows0[i]);
}
template<class M> void left_big(std::uint32_t g,M&m){
 const std::uint32_t rows0[]={0xff0002u,0x40000u,0x30000u,0xffu,0xff0002u,0x46083u,0xc0830004u,0x300ffu,0xff0002u,0x43083u,0x90830004u,0x200ffu,0xff0002u,0x10001u,0x10001u,0x100ffu};
 for(unsigned i=0;i<16;++i)m.write_long(g+0x2ba0+i*4,rows0[i]);
 const std::uint32_t rows1[]={0xff0000u,0x0u,0x0u,0xffu,0xff0000u,0x1702u,0x17080000u,0xffu,0xff0000u,0x1701u,0x17040000u,0xffu};
 for(unsigned i=0;i<12;++i)m.write_long(g+0x2fa0+i*4,rows1[i]);
}
template<class M> void right_big(std::uint32_t g,M&m){
 const std::uint32_t rows0[]={0xff0003u,0x30000u,0x0u,0xffu,0xff6082u,0xc0820004u,0x10002u,0x100ffu,0xff3082u,0x90820003u,0x60812081u,0xc08100ffu,0xff0002u,0x10003u,0x30815081u,0x908100ffu};
 for(unsigned i=0;i<16;++i)m.write_long(g+0x2da0+i*4,rows0[i]);
 const std::uint32_t rows1[]={0xff0000u,0x0u,0x0u,0xffu,0xff1902u,0x19080000u,0x0u,0xffu,0xff1901u,0x19040000u,0x18021800u,0x180800ffu,0xff0000u,0x0u,0x18011800u,0x180400ffu};
 for(unsigned i=0;i<16;++i)m.write_long(g+0x31a0+i*4,rows1[i]);
}
} // namespace source_attract_detail
template<class M> void initialize_source_attract_presets(std::uint32_t g,M&m){
 if(!m.read_byte(g+0x83))return;
 auto random=[&](){const auto r=global_random(m.read_word(g+0xae));m.write_word(g+0xae,r.seed);return r.value;};
 if(random()&1)source_attract_detail::left_big(g,m);else source_attract_detail::left_plain(g,m);
 if(random()&1)source_attract_detail::right_big(g,m);else source_attract_detail::right_plain(g,m);
}
// Same ordered5B7A/5C62 body as normal initialization;83 stays active
// throughout, score drawing is source-skipped, then5F44 adds two RNG draws.
template<class M,class H> void initialize_source_attract_players(std::uint32_t g,M&m,H&h){
 if(!m.read_byte(g+0x83))throw std::runtime_error("Source attract initialization requires active83");
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
  h.source_fall_speed(p);h.source_initial_pacing(p);/*11208 returns before score drawing with83 set.*/m.write_long(p+0x160,g+(side?0x300:0x700));h.source_generate_next(p);
  if(side){m.write_byte(p-0x78fd,1);h.source_second_player_tail(p);}
 }
 initialize_source_attract_presets(g,m);
}
} // namespace puzzle_fighter
