// SPDX-License-Identifier: GPL-3.0-only
// USA17AAE/17B2C,17BD4/17C52 and20F34/20F64 normal-match backgrounds.
#pragma once
#include <cstdint>
#include <stdexcept>
namespace puzzle_fighter {
namespace stage_detail {
inline std::int32_t sw(std::uint16_t w){return w&0x8000u?int(w)-65536:int(w);}
template<class M> void valid_stage(std::uint32_t g,M&m){const auto stage=m.read_word(g+0x100);if(stage>20||(stage&1))throw std::runtime_error("Invalid original stage selector");}
}
template<class M> void initialize_stage_backgrounds(std::uint32_t g,M&m){
    using namespace stage_detail;valid_stage(g,m);const auto stage=m.read_word(g+0x100);
    for(unsigned layer=0;layer<2;++layer){
        const auto actor=g+(layer?0x280:0x240),position=layer?0x17c26u:0x17b00u;
        m.write_byte(actor,std::uint8_t(m.read_byte(actor)+1));
        const auto x=m.opcode_word(position+stage*2),y=m.opcode_word(position+stage*2+2);
        m.write_word(actor+0x10,x);m.write_word(actor+0x18,x);m.write_word(g+(layer?0x2a:0x26),x);
        m.write_word(actor+0x14,y);m.write_word(actor+0x1c,y);m.write_word(g+(layer?0x2c:0x28),y);
        const auto table=layer?0x17dceu:0x17d08u;auto blocks=table+sw(m.read_word(table+stage));auto destination=layer?0x908000u:0x904000u;
        const unsigned count=layer?64:16,tiles=layer?64:256;
        for(unsigned b=0;b<count;++b){const auto block=m.read_byte(blocks++);auto source=(layer?0xe1080u:0xc5880u)+block*(layer?256u:1024u);
            for(unsigned t=0;t<tiles;++t,source+=4,destination+=4){m.write_word(destination,std::uint16_t(m.read_word(source)+(layer?0x1cc0:0x5800)));m.write_word(destination+2,m.read_word(source+2));}
        }
    }
}
template<class M> void initialize_stage_palettes(std::uint32_t g,M&m){
    stage_detail::valid_stage(g,m);const auto offset=unsigned(m.read_word(g+0x100))*512u;
    for(unsigned layer=0;layer<2;++layer){const auto source=(layer?0xa5ac0u:0xa22c0u)+offset,dest=layer?0x90cc00u:0x90c800u;
        for(unsigned i=0;i<512;++i)m.write_word(dest+i*2,std::uint16_t(m.read_word(source+i*2)|0xf000));}
}
template<class M> void step_stage_backgrounds(std::uint32_t g,M&m){
    using namespace stage_detail;valid_stage(g,m);const auto stage=m.read_word(g+0x100);
    for(unsigned layer=0;layer<2;++layer){const auto actor=g+(layer?0x280:0x240);if(!m.read_byte(actor))continue;
        const auto dispatch=layer?0x17c8cu:0x17b54u;const auto target=dispatch+sw(m.opcode_word(dispatch+stage));
        const auto entry=layer?0x17ca2u:0x17b6au;
        if(target==entry){const auto table=layer?0x17cccu:0x17b94u,index=unsigned(m.read_byte(g+0x10d))*4u;
            const auto dx=m.opcode_word(table+index),dy=m.opcode_word(table+index+2);
            m.write_word(actor+0x20,dx);m.write_word(actor+0x24,dy);m.write_word(actor+0x10,std::uint16_t(m.read_word(actor+0x18)+dx));m.write_word(actor+0x14,std::uint16_t(m.read_word(actor+0x1c)+dy));
        }else throw std::runtime_error("Unrecovered original stage scroll handler");
        m.write_word(g+(layer?0x2a:0x26),m.read_word(actor+0x10));m.write_word(g+(layer?0x2c:0x28),std::uint16_t(m.read_word(actor+0x14)-m.read_word(g+0x134)));
    }
    if(m.read_byte(g+0x280)&&m.read_byte(g+0x125)){m.write_word(g+0x2a,0x40);m.write_word(g+0x2c,0x700);}
}
template<class M> void step_source_text_background(std::uint32_t g,M&m){
    const auto actor=g+0x200;if(!m.read_byte(actor))return;
    const auto stage=m.read_word(g+0x100);const auto dispatch=0x17990u;const auto target=dispatch+stage_detail::sw(m.opcode_word(dispatch+stage));
    if(target!=0x179a6)throw std::runtime_error("Unrecovered original text scroll handler");
    const auto index=unsigned(m.read_byte(g+0x10d))*4;const auto dx=m.opcode_word(0x179d0+index),dy=m.opcode_word(0x179d2+index);m.write_word(actor+0x20,dx);m.write_word(actor+0x24,dy);m.write_word(actor+0x10,std::uint16_t(m.read_word(actor+0x18)+dx));m.write_word(actor+0x14,std::uint16_t(m.read_word(actor+0x1c)+dy));m.write_word(g+0x22,m.read_word(actor+0x10));m.write_word(g+0x24,m.read_word(actor+0x14));
}
template<class M> void stage_frame(std::uint32_t g,M&m){
    const auto counter=m.read_byte(g+0x124);m.write_word(g+0x134,(counter&1)?counter>>2:0);
    step_source_text_background(g,m);step_stage_backgrounds(g,m);
    if(!m.read_byte(g+0x12a)){
        const auto index=unsigned(m.read_word(g+0x100))*2u;
        m.write_word(g+0x32,m.opcode_word(0x178fc+index));m.write_word(g+0x3a,m.opcode_word(0x178feu+index));
        if(m.read_byte(g+0x125)){m.write_word(g+0x32,0x1e0e);m.write_word(g+0x3a,0x4261);}
    }
}
struct StagePictureSample{std::uint16_t tile,attribute;unsigned x,y,size,bank;std::uint32_t graphics_offset()const{return unsigned(tile)*(size==32?512u:128u)+y*(size==32?16u:8u)+(x/8)*4u;}};
// Exact CPS2 tilemap coordinates and raster origin; host viewport placement is
// separate. layer0=scroll2 (16px), layer1=scroll3 (32px).
template<class M> StagePictureSample stage_picture_sample(unsigned layer,unsigned x,unsigned y,std::uint32_t g,M&m){
    const unsigned size=layer?32:16,mask=layer?2047:1023;
    const auto sx=std::uint16_t(m.read_word(g+(layer?0x2a:0x26))-0x40),sy=std::uint16_t((layer?0x700:0x300)-m.read_word(g+(layer?0x2c:0x28)));
    const unsigned px=(x+64+sx)&mask,py=(y+16+sy)&mask,col=px/size,row=py/size;
    const unsigned index=layer?(row&7)+(col<<3)+((row&0x38)<<6):(row&15)+(col<<4)+((row&0x30)<<6);
    const auto address=(layer?0x908000u:0x904000u)+index*4;const auto tile=m.read_word(address),attr=m.read_word(address+2);
    return {std::uint16_t(layer?tile&0x3fff:tile),attr,(attr&0x20)?size-1-px%size:px%size,(attr&0x40)?size-1-py%size:py%size,size,(layer?0x60u:0x40u)+(attr&31)};
}
} // namespace puzzle_fighter
