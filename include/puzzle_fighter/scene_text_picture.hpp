// SPDX-License-Identifier: GPL-3.0-only
// USA2A960: append one original text line to a third-pool actor's RAM picture.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct SceneTextCursor {std::uint32_t source,tiles,coordinates;};
template<class M> void append_scene_text_line_mode(std::uint32_t actor,M&m,SceneTextCursor&cursor,std::uint16_t count_minus_one,std::uint16_t step,std::uint16_t y,bool narrow){
 std::uint16_t x=0;
 for(unsigned n=0;n<=count_minus_one;++n){auto tile=m.read_word(cursor.source);cursor.source+=2;auto row=y;
  if(!narrow){
   if(tile&0x8000){row=std::uint16_t(row-4);tile&=0x3fff;if(!tile){x=std::uint16_t(x+step);continue;}}
   if(tile){m.write_word(cursor.tiles,tile);cursor.tiles+=2;m.write_word(cursor.coordinates,x);m.write_word(cursor.coordinates+2,row);cursor.coordinates+=4;m.write_word(actor+0x42,m.read_word(actor+0x42)+1);}
   x=std::uint16_t(x+2*step);
  }else{
   if(tile&0x8000)tile&=0x7fff;
   if(tile){m.write_word(cursor.tiles,tile);cursor.tiles+=2;m.write_word(cursor.coordinates,x);m.write_word(cursor.coordinates+2,row);cursor.coordinates+=4;m.write_word(actor+0x42,m.read_word(actor+0x42)+1);if(m.read_word(actor+0x42)>=0x48)return;}
   x=std::uint16_t(x+step);
  }
 }
}
template<class M> void append_scene_text_line(std::uint32_t g,std::uint32_t actor,M&m,SceneTextCursor&cursor,std::uint16_t count_minus_one,std::uint16_t step,std::uint16_t y){append_scene_text_line_mode(actor,m,cursor,count_minus_one,step,y,m.read_word(g+0x92)!=0);}
} // namespace puzzle_fighter
