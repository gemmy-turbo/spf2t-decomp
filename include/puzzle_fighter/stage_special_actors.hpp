// SPDX-License-Identifier: GPL-3.0-only
// USA stage actors15=24CC2,24=264F4,31=279D2. Original embedded motion,
// shared RNG, palette/frame selection and exact task lifetimes.
#pragma once
#include "stage_actors.hpp"
#include "global_rng.hpp"
#include <cstdint>
namespace puzzle_fighter {
template<class M,class H> bool stage_special_task(std::uint32_t a,std::uint32_t g,M&m,H&h){
    StageActors<M,H> shared{m,h,g,a};
    const auto kind=m.read_byte(a+2),state=m.read_byte(a+4);
    auto next=[&](unsigned offset){m.write_byte(a+offset,m.read_byte(a+offset)+2);};
    auto refresh=[&](unsigned palette){if(m.read_byte(a+0xa)!=m.read_byte(g+0x10d)){m.write_byte(a+0xa,m.read_byte(g+0x10d));shared.palette(palette);}};
    if(kind==0x15){
        switch(state){
        case 0:
            next(4);m.write_word(a+0xc,0);m.write_byte(a+0xe,0x40);m.write_long(a+0x24,0x4000);m.write_byte(a+0xa,m.read_byte(g+0x10d));shared.palette(11);h.init(a,0,0x7c95c);return true;
        case 2:{
            const auto phase=m.read_byte(a+5);if(phase!=0&&phase!=2)return false;
            const auto velocity=m.read_long(a+0x24);m.write_long(a+0x14,phase?m.read_long(a+0x14)-velocity:m.read_long(a+0x14)+velocity);
            m.write_byte(a+0xe,m.read_byte(a+0xe)-1);
            if(m.read_byte(a+0xe)&0x80){m.write_byte(a+5,phase?0:2);m.write_byte(a+0xe,0x40);}
            refresh(11);h.display(a);return true;
        }
        case 4:case 6:h.free(a);return true;
        default:return false;
        }
    }
    if(kind==0x24){
        auto init=[&]{shared.picture_init(m.read_byte(a+3),0x7c94a);};
        auto refresh_picture=[&]{if(m.read_byte(a+0xa)!=m.read_byte(g+0x10d)){m.write_byte(a+0xa,m.read_byte(g+0x10d));init();}};
        switch(state){
        case 0:next(4);m.write_word(a+0xc,0);m.write_byte(a+3,1);m.write_byte(a+0xa,m.read_byte(g+0x10d));init();return true;
        case 2:
            if(m.read_byte(g+0x105)){
                const auto word=m.read_word(g+0x13a);
                const auto p=std::uint32_t(word&0x8000?int(word)-65536:int(word));
                if(m.read_byte(p+0x82)==4){next(4);m.write_byte(a+3,2);init();}
            }
            refresh_picture();shared.picture_step();h.display(a);return true;
        case 4:refresh_picture();shared.picture_step();h.display(a);return true;
        case 6:h.free(a);return true;
        default:return false;
        }
    }
    if(kind!=0x31)return false;
    auto random=[&]{const auto r=global_random(m.read_word(g+0xae));m.write_word(g+0xae,r.seed);return r.value;};
    auto trajectory=[&](std::uint32_t p,unsigned index){
        const auto table=0x27afau+(index&7)*16;
        for(unsigned n=0;n<4;++n)m.write_long(p+n*4,(std::uint32_t(m.opcode_word(table+n*4))<<16)|m.opcode_word(table+n*4+2));
        // Source writes only integer halves, preserving fractional words.
        m.write_word(p+0x10,m.read_word(a+0x20));m.write_word(p+0x14,0xb0);
    };
    switch(state){
    case 0:
        next(4);m.write_word(a+0xc,0);m.write_long(a+0x20,m.read_long(a+0x10));m.write_byte(a+0xa,m.read_byte(g+0x10d));shared.palette(6);shared.picture_init(2,0x7c960);return true;
    case 2:
        switch(m.read_byte(a+5)){
        case 0:
            if(m.read_byte(g+0x107)){next(5);m.write_byte(a+0xe,15);const auto index=random()&7;trajectory(a+0x50,index);trajectory(a+0x68,(index+1)&7);}break;
        case 2:
            if(m.read_byte(g+0x12a)){next(4);break;}
            {
                // BTST reads the HIGH byte of global wordB4, not lowB5.
                const auto p=a+((m.read_byte(g+0xb4)&1)?0x68u:0x50u);
                const auto vx=m.read_long(p);m.write_long(p+0x10,m.read_long(p+0x10)+(m.read_byte(a+0xb)?0u-vx:vx));
                m.write_long(p,m.read_long(p)+m.read_long(p+4));
                m.write_long(p+0x14,m.read_long(p+0x14)+m.read_long(p+8));
                m.write_long(p+8,m.read_long(p+8)+m.read_long(p+0xc));
                m.write_word(a+0x10,m.read_word(p+0x10));m.write_word(a+0x14,m.read_word(p+0x14));
                if(m.read_word(a+0x14)<=0x60)trajectory(p,random()&7);
                m.write_byte(a+0xe,m.read_byte(a+0xe)-1);if(m.read_byte(a+0xe)&0x80){m.write_byte(a+0xe,15);m.write_byte(a+0xb,m.read_byte(a+0xb)^1);}
                shared.picture_step();h.display(a);
            }break;
        default:return false;
        }
        refresh(6);return true;
    case 4:case 6:h.free(a);return true;
    default:return false;
    }
}
} // namespace puzzle_fighter
