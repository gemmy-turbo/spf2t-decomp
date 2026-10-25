// SPDX-License-Identifier: GPL-3.0-only
// USA17BD4/17CE0 special scroll3 picture selected by17C78 and14A0.
#pragma once
#include <cstdint>
#include <stdexcept>
namespace puzzle_fighter {
struct LargeFinishPictureSample {
    std::uint16_t tile,attribute;
    std::uint8_t x,y;
    unsigned palette_bank()const{return 0x60u+(attribute&31u);}
    // Prepared private graphics.bin represents the original populated4MiB
    // bank at board address800000.32px scroll3 tiles have512bytes each.
    std::uint32_t graphics_offset()const{return std::uint32_t(tile&0x3fffu)*512u+y*16u+(x/8u)*4u;}
};
// Exactly17CE0 for one source block (D0 lowbyte,A1 destination).
template<class M> void initialize_large_finish_scroll_block(unsigned block,std::uint32_t destination,M&m){
    auto source=0xe1080u+(block&255u)*256u;
    for(unsigned i=0;i<64;++i,source+=4,destination+=4){m.write_word(destination,std::uint16_t(m.read_word(source)+0x1cc0));m.write_word(destination+2,m.read_word(source+2));}
}
// Read the original selected picture at a native384x224 viewport pixel.
// stage is the ORIGINAL globals+100 word offset (0,2,...20), not character ID.
// Original17C78 forces source scroll(40,700), transformed by14A0 to hardware
// scroll(0,0); hardware visible raster starts(64,16). Only draw while source
// globals+125 is nonzero. Palette contents come from live90CFC0, never a wash.
template<class M> LargeFinishPictureSample large_finish_picture_sample(unsigned x,unsigned y,std::uint16_t stage,M&m){
    if(x>=384||y>=224)throw std::out_of_range("Original finish picture viewport");
    const unsigned raster_x=x+64,raster_y=y+16,column=raster_x>>5,row=raster_y>>5;
    const unsigned index=(row&7u)+(column<<3)+((row&0x38u)<<6);
    const auto offset=m.read_word(0x17dceu+stage);
    const auto table=0x17dceu+(offset&0x8000u?std::int32_t(offset)-65536:std::int32_t(offset));
    const auto block=m.read_byte(table+index/64u);
    const auto source=0xe1080u+block*256u+(index%64u)*4u;
    const auto attribute=m.read_word(source+2);
    const auto px=std::uint8_t(raster_x&31),py=std::uint8_t(raster_y&31);
    return {std::uint16_t(m.read_word(source)+0x1cc0),attribute,std::uint8_t(attribute&0x20?31-px:px),std::uint8_t(attribute&0x40?31-py:py)};
}
} // namespace puzzle_fighter
