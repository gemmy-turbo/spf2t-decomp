// SPDX-License-Identifier: GPL-3.0-only
// Bounded source gameplay view: three original tilemaps and1495A packets.
// Layout, blocked sprites and priorities follow the pinned MAME cps2.cpp
// render_layers/cps2_render_sprites. No CPU/board execution or invented HUD.
#pragma once
#include "sprite_packets.hpp"
#include <array>
#include <vector>
namespace puzzle_fighter {
struct OriginalPlayfieldPicture {
 static constexpr unsigned width=384,height=224;
 std::array<std::uint16_t,width*height> colours{};
};
template<class M> OriginalPlayfieldPicture original_playfield_picture(std::uint32_t g,M&m,const std::vector<std::uint8_t>&graphics,const SpritePacketFrame&objects){
 OriginalPlayfieldPicture frame;std::array<std::uint8_t,384*224>priority{};std::array<bool,384*224>occupied{};
 const auto control=m.read_word(g+0x32),priorities=m.read_word(g+0x3a);
 std::array<unsigned,4>layers{};for(unsigned i=0;i<4;++i)layers[i]=(control>>(6+i*2))&3;
 if(!layers[0]){layers[0]=layers[1];layers[1]=0;}if(!layers[1]){layers[1]=layers[2];layers[2]=0;}if(!layers[2]){layers[2]=layers[3];layers[3]=0;}
 std::array<unsigned,3>lp{};for(unsigned i=0;i<3;++i)lp[i]=(priorities>>(4*layers[i]))&15;
 unsigned mask0=0xaa,mask1=0xcc;if(lp[0]>lp[1])mask0&=~0x88u;if(lp[0]>lp[2])mask0&=~0xa0u;if(lp[1]>lp[2])mask1&=~0xc0u;
 std::array<unsigned,8>masks{};masks[0]=0xff;
 for(unsigned i=1;i<8;++i){if(i<=lp[0]&&i<=lp[1]&&i<=lp[2])masks[i]=0xfe;else{if(i<=lp[0])masks[i]|=mask0;if(i<=lp[1])masks[i]|=mask1;if(i<=lp[2])masks[i]|=0xf0;}}
 const auto pen=[&](unsigned at,unsigned bit){unsigned value=0;if(at+3>=graphics.size())return 15u;for(unsigned plane=0;plane<4;++plane)value|=((graphics[at+plane]>>(7-bit))&1u)<<plane;return value;};
 // Video bases index graphics RAM, with the hardware bank/alignment mask.
 // In particular a zero register before the first IRQ is video offset zero.
 const auto palette=0x900000u|((std::uint32_t(m.read_word(g+0x20))<<8)&0x3fc00u);
 const auto colour=[&](unsigned index){return m.read_word(palette+index*2);};
 frame.colours.fill(colour(0xbff));
 for(unsigned slot=0;slot<3;++slot){const auto layer=layers[slot];if(!layer||!(control&(1u<<layer)))continue;
  const unsigned size=layer==1?8:layer==2?16:32,mask=size*64-1;
  const auto base=0x900000u|((std::uint32_t(m.read_word(g+0x18+(layer-1)*2))<<8)&0x3c000u);
  const auto scrollx=m.read_word(g+0x22+(layer-1)*4),scrolly=m.read_word(g+0x24+(layer-1)*4);
  const unsigned ybase=layer==1?0x100:layer==2?0x300:0x700;
  for(unsigned y=0;y<224;++y)for(unsigned x=0;x<384;++x){
   const auto px=(x+scrollx)&mask,py=(y+16+ybase-scrolly)&mask,col=px/size,row=py/size;
   const auto index=layer==1?(row&31)+(col<<5)+((row&32)<<6):layer==2?(row&15)+(col<<4)+((row&0x30)<<6):(row&7)+(col<<3)+((row&0x38)<<6);
   const auto tile=m.read_word(base+index*4),attr=m.read_word(base+index*4+2);
   const unsigned sx=attr&0x20?size-1-px%size:px%size,sy=attr&0x40?size-1-py%size:py%size;
   const auto at=layer==1?unsigned(tile)*64+sy*8:unsigned(layer==3?tile&0x3fff:tile)*(layer==3?512u:128u)+sy*(layer==3?16u:8u)+(sx/8)*4;
   const auto value=pen(at,sx%8);if(value==15)continue;
   const auto out=y*384+x;frame.colours[out]=colour((layer*32+(attr&31))*16+value);priority[out]|=1u<<slot;
  }
 }
 for(auto it=objects.packets.rbegin();it!=objects.packets.rend();++it){const auto&o=*it;const auto attr=o.attributes;const unsigned across=((attr>>8)&15)+1,down=((attr>>12)&15)+1;const bool fx=attr&0x20,fy=attr&0x40;
  const auto code=unsigned(o.tile)+((o.y&0x6000)<<3);const int ox=int(o.x&1023)-64+(attr&0x80?64:0),oy=int(o.y&1023)-16+(attr&0x80?16:0);
  for(unsigned by=0;by<down;++by)for(unsigned bx=0;bx<across;++bx){const unsigned cx=fx?across-1-bx:bx,cy=fy?down-1-by:by;const unsigned tile=((!fx&&!fy)?(code&~15u)+((code+cx)&15u)+16*cy:code+cx+16*cy)&0x7fff;
   for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x){const int dx=(ox+int(bx*16+x)+1024)%1024,dy=(oy+int(by*16+y)+1024)%1024;if(dx>=384||dy>=224)continue;
    const unsigned sx=fx?15-x:x,sy=fy?15-y:y,value=pen(tile*128+sy*8+(sx/8)*4,sx%8);if(value==15)continue;
    const auto out=unsigned(dy)*384+unsigned(dx);if(occupied[out])continue;occupied[out]=true;
    if(!(masks[o.x>>13]&(1u<<priority[out])))frame.colours[out]=colour((attr&31)*16+value);
   }
  }
 }
 return frame;
}
} // namespace puzzle_fighter
