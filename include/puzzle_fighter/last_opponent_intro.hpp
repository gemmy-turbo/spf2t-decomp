// SPDX-License-Identifier: GPL-3.0-only
// USA BDDA subtype2: BE0E..BE58, including16C3C name labels.
#pragma once
#include "arcade_match_intro.hpp"
#include "sound_requests.hpp"
namespace puzzle_fighter {
//16C3C redraws the two names only: no score digits or third-pool allocation.
template<class M> void source_character_name_labels(std::uint32_t g,M&m){
 const auto base=0x84560u;const auto names=base+std::uint32_t(std::int32_t(std::int16_t(m.read_word(base+(m.read_word(g+0x90)?4:2)))));
 for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400;auto index=std::uint16_t(m.read_byte(p+0x82)*2);index=std::uint16_t((index&0xff00)|std::uint8_t(index+m.read_byte(p+0x81)));index=std::uint16_t(index<<4);auto source=m.read_long(names+std::uint32_t(std::int32_t(std::int16_t(index)))+4);const auto count=unsigned(m.read_word(source+4))+1;source+=6;auto out=0x9007a0u+side*0x500;for(unsigned n=0;n<count;++n){m.write_long(out,m.read_long(source));m.write_long(out+4,m.read_long(source+4));source+=8;out+=0x80;}}
}
namespace last_opponent_detail {struct PaletteHooks {};}
// True means this source subtype handled one call and returned. It does NOT
// mean the outer intro should advance in the same frame. A78C returns directly
// after BDDA; only its NEXT call with13E==0 executes normalA798.
template<class M> bool tick_last_opponent_intro(std::uint32_t g,M&m){
 if(m.read_word(g+0x13e)!=2)return false;
 switch(m.read_word(g+0x14)){
 case 0:{
  m.write_word(g+0x14,2);m.write_byte(g+0x10f,0);m.write_byte(g+0xbd,1);const auto opponent=g+((m.read_byte(g+0xac)&1)?0x700:0x300);m.write_byte(opponent+0x82,9);
  last_opponent_detail::PaletteHooks h;ArcadeMatchIntro<M,decltype(h)>{m,h,g}.player_palettes();start_source_round_music(g,m);return true;
 }
 case 2:source_character_name_labels(g,m);[[fallthrough]];
 case 4:case 6:case 8:m.write_long(g+0x14,0);m.write_word(g+0x13e,0);m.write_word(g+0x102,0);m.write_byte(g+0xbd,0);return true;
 default:return false;
 }
}
} // namespace puzzle_fighter
