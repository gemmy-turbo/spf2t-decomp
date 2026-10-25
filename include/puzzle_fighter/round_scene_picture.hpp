// SPDX-License-Identifier: GPL-3.0-only
// USA1788C/17928 text maps and16C0C..16D5A original round labels.
#pragma once
#include "stage_background.hpp"
#include "player_initialization.hpp"
#include "attack_display.hpp"
namespace puzzle_fighter {
// USA2CF96: clear the complete16KiB text plane, including its off-screen page.
template<class M> void clear_source_text_plane(M&m){
 for(unsigned n=0;n<0x4000;n+=4)m.write_long(0x900000+n,0x00200000);
}
template<class M> void initialize_source_text_map(std::uint32_t g,M&m){
 const auto actor=g+0x200;m.write_byte(actor,m.read_byte(actor)+1);
 for(unsigned o:{0x10u,0x18u}){m.write_word(actor+o,0);}
 for(unsigned o:{0x14u,0x1cu}){m.write_word(actor+o,0x100);}
 m.write_word(g+0x22,0);m.write_word(g+0x24,0x100);
 for(unsigned side=0;side<2;++side){const unsigned index=side?1:m.read_word(g+0x90)?17:0;auto blocks=0xc0000u+index*128;
  for(unsigned col=0;col<8;++col)for(unsigned row=0;row<8;++row){const auto source=0xc0a00u+m.read_word(blocks)*64;blocks+=2;const auto out=0x900000u+side*0x1000+col*0x200+row*0x10;
   for(unsigned x=0;x<4;++x)for(unsigned y=0;y<4;++y)m.write_long(out+x*0x80+y*4,m.read_long(source+x*16+y*4));
  }
 }
}
template<class M> void initialize_source_scene_picture(std::uint32_t g,M&m){
 m.write_byte(g+0x10d,0);initialize_source_text_map(g,m);initialize_stage_backgrounds(g,m);
 if(!m.read_byte(g+0x12a)){const auto offset=unsigned(m.read_word(g+0x100))*2;m.write_word(g+0x32,m.opcode_word(0x178fc+offset));m.write_word(g+0x3a,m.opcode_word(0x178feu+offset));if(m.read_byte(g+0x125)){m.write_word(g+0x32,0x1e0e);m.write_word(g+0x3a,0x4261);}}
}
template<class M> void source_round_labels(std::uint32_t g,M&m){
 allocate_attack_display(g,m);source_score_digits(m.read_long(g+0x3aa),0x9007e4,1,m);source_score_digits(m.read_long(g+0x7aa),0x900d64,2,m);
 const auto base=0x84560u;const auto table=base+attack_display_detail::sw(m.read_word(base+(m.read_word(g+0x90)?4:2)));
 for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400;const auto offset=unsigned(std::uint8_t(m.read_byte(p+0x82)*2+m.read_byte(p+0x81)))*16;auto source=m.read_long(table+offset+4);const auto columns=m.read_word(source+4)+1u;source+=6;auto out=side?0x900ca0u:0x9007a0u;for(unsigned n=0;n<columns;++n,out+=0x80){m.write_long(out,m.read_long(source));m.write_long(out+4,m.read_long(source+4));source+=8;}}
}
template<class M> void source_course_label(std::uint32_t g,M&m){
 const auto index=m.read_byte(g+0xac)==3?3u:m.read_byte(g-0x7374);const auto base=0x84560u;auto table=base+attack_display_detail::sw(m.read_word(base+0x18));auto source=m.read_long(table+index*16+4);const auto columns=m.read_word(source+4)+1u;source+=6;auto out=0x900a58u;for(unsigned n=0;n<columns;++n,out+=0x80){for(unsigned y=0;y<3;++y)m.write_long(out+y*4,m.read_long(source+y*4));source+=12;}
}
template<class M> void source_match_win_labels(std::uint32_t g,M&m){
 for(unsigned side=0;side<2;++side){if(!(m.read_byte(g+0xac)&(1u<<side)))continue;const auto count=m.read_byte(g+0x3a2+side*0x400);if(!count)continue;auto out=0x9002f0u+side*0x1000;
  auto digit=[&](unsigned v){m.write_word(out,0x1e0+v);m.write_word(out+2,2);m.write_word(out+4,0x1f0+v);m.write_word(out+6,2);out+=0x80;};
  const auto extended=std::uint16_t(count&0x80?0xff00|count:count);const auto high=extended>>4;if(high)digit(high);else out+=0x80;digit(count&15);out+=0x80;for(unsigned n=0;n<(extended==1?3u:4u);++n)digit(10+n);
 }
}
} // namespace puzzle_fighter
