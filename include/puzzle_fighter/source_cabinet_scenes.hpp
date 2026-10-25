// SPDX-License-Identifier: GPL-3.0-only
// USA17BC,2228..2500: finite boot and idle notice scene owners.
#pragma once
#include "source_startup.hpp"
#include "source_idle_start.hpp"
#include "source_idle_pictures.hpp"
#include "source_scoreboard.hpp"
namespace puzzle_fighter {
template<class M> void copy_source_raw_palettes(M&m,std::uint32_t from,std::uint32_t to,unsigned blocks){for(unsigned n=0;n<blocks*32;n+=4)m.write_long(to+n,m.read_long(from+n));}
template<class M> void source_idle_palettes(std::uint32_t g,M&m){const auto index=m.read_word(g+0x128);const auto offset=std::int16_t(std::uint16_t((index>>7)|(index<<9)));const unsigned sources[]={unsigned(0xa12c0 + offset),0xa1ec0,unsigned(0xa4ec0 + offset),unsigned(0xa86c0 + offset)};for(unsigned bank=0;bank<4;++bank)copy_source_raw_palettes(m,sources[bank],0x90c000 + bank*0x400,32);}
template<class M> void source_scene_object_palettes(std::uint32_t g,M&m,bool score){ //20C5C/20C6C object portions
 if(score)copy_source_raw_palettes(m,0xabba0,0x90c080,7);else copy_source_raw_palettes(m,0xa0b00,0x90c000,12);copy_source_raw_palettes(m,0xa0c80,0x90c280,6);
 if(m.read_word(g+0x100)==16)copy_source_raw_palettes(m,0xaa1e0,0x90c340,2);
 copy_source_raw_palettes(m,std::uint32_t(0xa0d40 + std::int16_t(std::uint16_t(m.read_word(g+0x100)<<6))),0x90c380,4);
 for(unsigned side=0;side<2;++side){const auto p=g+0x300 + side*0x400;const auto offset=std::int16_t(std::uint16_t((m.read_byte(p+0x82)*2+m.read_byte(p+0x9f))<<7));copy_source_raw_palettes(m,0xa0000 + offset,0x90c180 + side*0x80,4);}
 if(score){copy_source_raw_palettes(m,0xa1ec0,0x90c400,32);const auto stage=m.read_word(g+0x100);const auto offset=std::int16_t(std::uint16_t((stage>>7)|(stage<<9)));copy_source_raw_palettes(m,0xa22c0 + offset,0x90c800,32);copy_source_raw_palettes(m,0xa5ac0 + offset,0x90cc00,32);}
}
template<class M> void draw_source_large_picture(M&m,unsigned index,std::uint32_t to){auto from=0xe1080u+(index&255)*256;for(unsigned n=0;n<64;++n){m.write_word(to+n*4,m.read_word(from+n*4)+0x1cc0);m.write_word(to+n*4+2,m.read_word(from+n*4+2));}}
template<class M> void source_attract_scene_palettes(std::uint32_t g,M&m){source_scene_object_palettes(g,m,false);copy_source_raw_palettes(m,0xa1ec0,0x90c400,32);const auto stage=m.read_word(g+0x100);const auto offset=std::int16_t(std::uint16_t((stage>>7)|(stage<<9)));copy_source_raw_palettes(m,0xa22c0+offset,0x90c800,32);copy_source_raw_palettes(m,0xa5ac0+offset,0x90cc00,32);}
enum class SourceBootResult {complete,wait_object_clear,idle,unsupported};
template<class M> struct SourceBoot {
 M&m;std::uint32_t g;enum class Resume {none,second_notice,return_only,idle};Resume pending=Resume::none;
 SourceBootResult resume_after_clear(){const auto was=pending;pending=Resume::none;if(was==Resume::second_notice){draw_source_notice(m,m.opcode_word(0x187e + m.read_word(g+0x90)));return SourceBootResult::complete;}if(was==Resume::return_only)return SourceBootResult::complete;if(was==Resume::idle){initialize_source_base_palettes(m);return SourceBootResult::idle;}return SourceBootResult::unsupported;}
 SourceBootResult run(){if(pending!=Resume::none)return SourceBootResult::unsupported;
  switch(m.read_word(g)){
  case 0:initialize_source_boot_state(g,m);return SourceBootResult::complete;
  case 2:m.write_word(g,4);m.write_word(g+2,180);draw_source_notice(m,m.opcode_word(0x1844 + m.read_word(g+0x90)));return SourceBootResult::complete;
  case 4:case 6:{const auto phase=m.read_word(g);if(!(m.read_word(g+0x58)&0x100)){m.write_word(g+2,m.read_word(g+2)-1);if(std::int16_t(m.read_word(g+2))>=0)return SourceBootResult::complete;}m.write_word(g,phase+2);if(phase==4){m.write_word(g+2,180);pending=Resume::second_notice;}else pending=Resume::return_only;return SourceBootResult::wait_object_clear;}
  case 8:for(unsigned o=0;o<24;o+=4)m.write_long(g+o,0);m.write_long(g-0x734a,0);m.write_byte(g+0xd0,0);if(m.read_byte(g+0xce))throw std::runtime_error("Unbound source card boot profile");pending=Resume::idle;return SourceBootResult::wait_object_clear;
  default:return SourceBootResult::unsupported;
  }
 }
};
enum class IdleNoticeAction {palette,stop_sound,reset_pools,fade_in,fade_out,wide_picture,large_picture,notice_song,logo_sound};
enum class IdleNoticeResult {complete,wait_object_clear,unsupported};
template<class M,class H> struct SourceIdleNotices {
 M&m;H&h;std::uint32_t g;
 void action(IdleNoticeAction a,unsigned x=0,std::uint32_t y=0){h.action(a,x,y);}
 void word(unsigned o,unsigned v){m.write_word(g+o,std::uint16_t(v));}void next(unsigned o){word(o,m.read_word(g+o)+2);}
 void advance(){next(0);m.write_long(g+4,0);}
 IdleNoticeResult transition(){switch(m.read_word(g+4)){case 0:next(4);action(IdleNoticeAction::fade_out);return IdleNoticeResult::complete;case 2:if(!m.read_byte(g+0x4782)){next(4);word(0x20,0x92a0);word(0x48,0x92a0);}return IdleNoticeResult::complete;case 4:prepare_source_idle_transition_clear(g,m);return IdleNoticeResult::wait_object_clear;default:return IdleNoticeResult::unsupported;}}
 IdleNoticeResult notice(){const auto scene=m.read_word(g);if(scene!=4&&scene!=8&&scene!=12&&scene!=16&&scene!=36)return IdleNoticeResult::unsupported;
  if(m.read_word(g+0x90)!=2)throw std::runtime_error("Only USA idle notices recovered");
  switch(m.read_word(g+4)){
  case 0:next(4);word(0x22,0);word(0x24,scene==12?0x140:0x100);word(0x2a,0);word(0x2c,0x700);
   if(scene==4||scene==8){word(0x26,0);word(0x28,0x300);}if(scene==4){action(IdleNoticeAction::large_picture,0x6a,0x908000);action(IdleNoticeAction::large_picture,0x6b,0x908100);}
   else if(scene==8){action(IdleNoticeAction::wide_picture,0x64,0x904000);action(IdleNoticeAction::wide_picture,0x65,0x904400);fill_source_large_tile_page(m,0,29,0x908000);fill_source_large_tile_page(m,0,29,0x908100);}
   else if(scene==12){draw_source_notice(m,0x54);draw_source_big_notice(m,0);fill_source_large_tile_page(m,0,3,0x908000);fill_source_large_tile_page(m,0,3,0x908100);}
   else{draw_source_closing_mark(m,1,0x9007bc);action(IdleNoticeAction::logo_sound);fill_source_large_tile_page(m,0,29,0x908000);fill_source_large_tile_page(m,0,29,0x908100);}
   word(0x32,(scene==4||scene==8)?0x1b0e:0x1e0e);word(0x3a,(scene==4||scene==8)?0x2461:0x4261);word(0x20,0x90c0);word(0x128,0);action(IdleNoticeAction::palette);action(IdleNoticeAction::fade_in);return IdleNoticeResult::complete;
  case 2:if(!m.read_byte(g+0x4782)){next(4);word(6,scene==4||scene==8?360:scene==12?120:180);if(scene==8)action(IdleNoticeAction::notice_song);}return IdleNoticeResult::complete;
  case 4:word(6,m.read_word(g+6)-1);if(!m.read_word(g+6))advance();return IdleNoticeResult::complete;
  default:return IdleNoticeResult::unsupported;
  }
 }
 IdleNoticeResult run(){const auto scene=m.read_word(g);if(!scene){next(0);word(0x128,0);action(IdleNoticeAction::palette);action(IdleNoticeAction::stop_sound);action(IdleNoticeAction::reset_pools);return IdleNoticeResult::complete;}if(scene==48){restart_source_idle_cycle(g,m);return IdleNoticeResult::complete;}if(scene>=2&&scene<=46&&(scene&3)==2)return transition();return notice();}
};
} // namespace puzzle_fighter
