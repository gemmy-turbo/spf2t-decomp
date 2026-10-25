// SPDX-License-Identifier: GPL-3.0-only
// USA cold-start/default ownership. No kernel, EEPROM or cabinet emulation.
#pragma once
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
namespace puzzle_fighter {
// D52 after successful physical RAM tests, used only for a board reset.
template<class M> void clear_source_board_ram(M&m){
 for(unsigned o=0;o<0x10000;o+=4)m.write_long(0xff0000+o,0);
}
// D6E/D82/D96 establish empty text and both background maps. These are
// source-visible tile values, not a host renderer's chosen black texture.
template<class M> void initialize_source_empty_tile_pages(M&m){
 const std::uint16_t tiles[]={0x20,0x5820,0x1cc0};
 for(unsigned page=0;page<3;++page)for(unsigned o=0;o<0x4000;o+=4)m.write_long(0x900000+page*0x4000+o,std::uint32_t(tiles[page])<<16);
}
// BE4..C9C RAM mirrors after the board memory tests. The intervening C4E
// physical804040=F08 output remains a cabinet adapter event, not RAM state.
template<class M> void initialize_source_board_mirrors(std::uint32_t g,M&m){
 const std::uint16_t pages[]={0x9000,0x9040,0x9080,0x90e0,0x90c0};
 for(unsigned i=0;i<5;++i){m.write_word(g+0x18+i*2,pages[i]);m.write_word(g+0x40+i*2,pages[i]);}
 for(unsigned o=0x22;o<=0x2e;o+=2)m.write_word(g+o,0);
 const std::uint16_t registers[]={0xf,0x1b02,0x3f,0x7000,0x807d,0x2461,0x40,0x10};
 for(unsigned i=0;i<8;++i)m.write_word(g+0x30+i*2,registers[i]);
 m.write_word(g+0x6a,0xf00);m.write_byte(g+0x88,8);m.write_byte(g+0xd1,0);
 for(unsigned o=0x4a;o<=0x54;o+=2)m.write_word(g+o,0);
}
// 2B1EE, called F82: original queue records and producer/consumer indices.
template<class M> void initialize_source_sound_queue(std::uint32_t g,M&m){
 for(unsigned o=0x47a0;o<0x57a0;o+=4)m.write_long(g+o,0);
 m.write_long(g+0x57a0,0);
}
// 20BB8: source base palette and two blank palette banks. This is also used
// by the boot terminal18A4; it is not only a board initialization operation.
template<class M> void initialize_source_base_palettes(M&m){
 for(unsigned o=0;o<0x400;o+=4)m.write_long(0x90c400+o,m.read_long(0xa1ec0+o));
 for(unsigned o=0;o<0x800;o+=4)m.write_long(0x90d000+o,0);
}
// 20BEE and20C16: source tile work areas, after physical RAM tests.
template<class M> void initialize_source_tile_work_areas(M&m){
 for(unsigned o=0;o<0x2000;o+=4){m.write_long(0x928000+o,0xffffffff);m.write_long(0x92a000+o,0);}
}
template<class M> std::uint32_t source_operator_default_address(std::uint32_t g,M&m){
 const auto locale=m.read_word(g+0x90);
 if(locale>12||(locale&1))throw std::runtime_error("Unsupported source operator locale");
 return std::uint32_t(0x1e89c+std::int16_t(m.opcode_word(0x1e89c+locale)));
}
// 1781E: cold board only; game-over/idle re-entry must retain this table.
template<class M> void initialize_source_leaderboard(std::uint32_t g,M&m){
 for(unsigned o=0;o<80;o+=4)m.write_long(g+0x57aa+o,m.read_long(0x1783c+o));
}
// 1E5F4: rejected/missing EEPROM defaults in the RAM image. The host owns
// EEPROM transport/persistence and reports validation; this is its source fallback.
template<class M> void initialize_source_operator_defaults(std::uint32_t g,M&m){
 const auto from=source_operator_default_address(g,m),to=g-0x7070;
 for(unsigned copy=0;copy<2;++copy)for(unsigned o=0;o<48;o+=2)m.write_word(to+copy*48+o,m.read_word(from+o));
 for(unsigned o=96;o<128;o+=2)m.write_word(to+o,0);
}
// 1E3D0: repair validated EEPROM contents using original regional bounds.
template<class M> void repair_source_operator_settings(std::uint32_t g,M&m){
 const auto from=source_operator_default_address(g,m),to=g-0x7070;
 const auto locale=m.read_word(g+0x90);const bool regional=locale==4||locale==12;
 auto value=[&](unsigned o){return m.read_byte(to+o);};
 auto repair=[&](unsigned o){const auto v=m.read_byte(from+o);m.write_byte(to+o,v);m.write_byte(to+48+o,v);};
 const unsigned max_coin=locale==4?1:locale==12?2:18;
 if(value(0)>max_coin&&!(regional&&value(0)==9))repair(0);
 if(value(2)&0x7f)repair(2);
 for(unsigned o:{3u,4u})if(value(o)>1)repair(o);
 if(value(5)>2)repair(5);
 if(value(6)>(locale==8?1:0))repair(6);
 if(locale==10?std::uint8_t(value(7)-2)>2:value(7)!=m.read_byte(from+7))repair(7);
 if(value(8)>(regional?1:0))repair(8);
 if(value(9)>(regional?6:0))repair(9);
 if(value(10)>(regional?18:0))repair(10);
 if(value(16)>7)repair(16);
 if(value(18)>3)repair(18);
 if(value(17)>3)repair(17);
 if(value(22)>1)repair(22);
 for(unsigned o:{20u,21u})if(std::uint8_t(value(o)-1)>2)repair(o);
 if(value(19)>3)repair(19);
 if(value(19)==3){if(value(1)!=0)repair(1);}
 else if(value(0)==17){if(value(1)!=1)repair(1);}
 else if(value(1)>1)repair(1);
 if(value(23)>1)repair(23);
}
// 1DF04..1E004: consume the source EEPROM RAM image, not host defaults.
template<class M> void apply_source_operator_settings(std::uint32_t g,M&m){
 const auto a=g-0x7070;auto v=[&](unsigned o){return m.read_byte(a+o);};
 for(unsigned o:{0x98u,0x9cu,0x9du})m.write_byte(g+o,0);
 const auto coin=v(0);m.write_byte(g+0x97,coin);
 if(coin==18){m.write_byte(g+0x9c,1);m.write_word(g+0x72,0x101);m.write_word(g+0x7a,0x101);}
 else if(coin==17){m.write_byte(g+0x98,1);m.write_byte(g+0x9d,1);m.write_word(g+0x72,0x101);m.write_word(g+0x7a,0x101);}
 else{const auto c=m.opcode_word(0x1ea0a+coin*2);m.write_word(g+0x72,c);m.write_word(g+0x7a,c);}
 if(coin!=17)m.write_byte(g+0x98,v(1));
 m.write_byte(g+0x96,v(2));m.write_byte(g+0x99,v(3));m.write_byte(g+0x9a,v(4));m.write_byte(g+0x9b,v(5));
 m.write_byte(g+0x9f,m.opcode_byte(0x1ea42+v(5)*2));m.write_byte(g+0x9e,m.opcode_byte(0x1ea43+v(5)*2));
 m.write_byte(g+0xbc,v(6));m.write_byte(g+0x92,0);m.write_byte(g+0x93,v(7));m.write_byte(g+0xce,v(8));
 m.write_byte(g+0xd2,v(9));m.write_byte(g+0xd3,v(10));m.write_byte(g+0xd4,m.opcode_byte(0x1ea58+v(9)));m.write_byte(g+0xd5,m.opcode_byte(0x1ea58+v(10)));
 m.write_byte(g+0xa0,v(16));m.write_byte(g+0xa1,v(17));m.write_byte(g+0xde,v(18));m.write_byte(g+0xa5,v(19));
 if(v(19)==3)m.write_byte(g+0x98,0);
 m.write_byte(g+0xa2,v(20));m.write_byte(g+0xa3,v(21));m.write_byte(g+0xa4,v(22));m.write_byte(g+0xbb,v(23));
 for(unsigned i=0;i<3;++i)m.write_long(g+0xc0+i*4,m.read_long(a+0x60+i*4));
 m.write_long(g+0xda,m.read_long(a+0x6c));
}
// 17F4 +18EA: first boot state. Statistics86 preserves its earlier battery
// seed; inactive cabinet source writes01C3. PC-relative palette is opcode view.
template<class M> void initialize_source_boot_state(std::uint32_t g,M&m){
 m.write_word(g,m.read_word(g)+2);m.write_byte(g+0x83,1);m.write_byte(g+0xbf,0);
 m.write_word(g+0x22,0);m.write_word(g+0x24,0x100);
 if(!m.read_byte(g+0x86)){m.write_word(g+0xae,0x1c3);m.write_word(g+0x444,0);m.write_word(g+0x844,0);}
 const auto from=0x1912+m.read_word(g+0x90)*2;
 const std::uint32_t color=(std::uint32_t(m.opcode_word(from))<<16)|m.opcode_word(from+2);
 for(unsigned o=0;o<32;o+=4)m.write_long(0x90c420+o,o>=4&&o<=12?color:0xf000f000);
}
// 2156..21B8: idle job entry BEFORE its initial IRQ wait. Not a cold reset:
// seed, credits, operator settings, score table, and retained words survive.
template<class M> void initialize_source_idle_state(std::uint32_t g,M&m){
 m.write_word(g+0x32,0x1b0e);m.write_word(g+0x3a,0x2461);m.write_byte(g+0x83,1);m.write_long(g,0);m.write_word(g+0x100,0);
 for(unsigned o:{0x8eu,0xa8u,0x8du,0xaau,0x4781u,0x4782u,0x4783u,0x8fu,0xbdu,0xabu,0xd8u})m.write_byte(g+o,0);
 m.write_byte(g-0x7398,0);for(unsigned o=0x22;o<=0x2c;o+=2)m.write_word(g+o,0);
}
} // namespace puzzle_fighter
