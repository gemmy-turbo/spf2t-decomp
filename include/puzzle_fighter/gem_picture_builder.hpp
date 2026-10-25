// SPDX-License-Identifier: GPL-3.0-only
// Original USA151F0..15641 animation-record builder. ROM animations are DATA;
// the two small type tables at1542A/1545E are PC-relative OPCODE bytes.
#pragma once
#include <array>
#include <cstdint>
namespace puzzle_fighter {
namespace gem_picture_detail {
inline int sw(std::uint16_t w){return w&0x8000?int(w)-65536:int(w);}
inline std::uint16_t rol(std::uint16_t w,unsigned n){return std::uint16_t((unsigned(w)<<n)|(w>>(16-n)));}
template<class M> struct Builder {
    M&m;std::uint32_t g;
    void dimensions(std::uint32_t p,std::uint32_t out,std::uint32_t meta){
        if(!m.read_byte(p+0x266))return;
        for(unsigned row=0;row<12;++row){
            for(unsigned col=0;col<5;++col,out+=0x20,meta+=2){
                if(m.read_byte(meta+1)!=2)continue;
                const auto group=m.read_byte(meta);unsigned width=0,height=0;
                for(unsigned n=0;n<5-col&&m.read_byte(meta+2+2*n)==group;++n)++width;
                for(unsigned n=0;n<12-row&&m.read_byte(meta+0x10+0x10*n)==group;++n)++height;
                for(unsigned y=0;y<=height;++y)for(unsigned x=0;x<=width;++x){
                    const auto a=out+y*0xc0+x*0x20;
                    m.write_word(a+0x16,m.read_word(a+0x14));m.write_byte(a+0x14,width);m.write_byte(a+0x15,height);
                    m.write_byte(a+0x18,width-x);m.write_byte(a+0x19,height-y);
                }
            }
            out+=0x20;meta+=6;
        }
    }
    void palette(){
        if(m.read_byte(g+0x81)&3)return;
        const auto phase=std::uint16_t((m.read_word(g-0x7348)+0x20)&0xe0);m.write_word(g-0x7348,phase);
        const auto from=0xab8c0u+phase,to=0x90c0c0u;
        const auto brightness=m.read_long(to)&0xf000f000u;
        for(unsigned i=0;i<8;++i)m.write_long(to+4*i,m.read_long(from+4*i)|brightness);
    }
    // cell is the board/pair address BEFORE the source postincrement; out is
    // one32-byte animation record. next=true selects queued gem dispatch.
    void cell(std::uint32_t p,std::uint32_t cell,std::uint32_t out,std::uint32_t flag,bool pair=false,bool next=false){
        std::uint16_t gem=m.read_word(cell),colour=gem,selector=0;
        auto blank=[&]{m.write_long(out,0);m.write_long(out+0x14,0);};
        auto load=[&](std::uint32_t animation){
            m.write_long(out+4,animation);m.write_long(out+8,m.read_long(animation));
            m.write_long(out+0xc,m.read_long(animation+4));m.write_long(out+0x10,m.read_long(animation+8));
            if(m.read_byte(flag))colour=5;
            m.write_word(out+0x12,m.read_word(out+0x12)|colour);
        };
        auto initialize=[&]{
            m.write_word(out+2,selector);
            const auto relative=m.read_word(0x40000u+std::uint32_t(sw(selector)));
            load(0x40000u+std::uint32_t(sw(relative)));
        };
        auto advance=[&]{
            auto animation=m.read_long(out+4);
            if(!m.read_byte(flag)){
                m.write_word(out+8,m.read_word(out+8)-1);
                if(m.read_word(out+8))return;
                animation+=12;
                if(m.read_byte(out+0xa)&0x80)animation=m.read_long(animation);
            }
            load(animation);
        };
        if(!pair){
            if(!gem){blank();return;}
            if(gem&0x20){
                if(!(gem&0x80)){
                    const auto raw=m.opcode_byte(0x1542a+(gem&15));colour=std::uint16_t(raw&0x80?0xff00|raw:raw);
                    const auto phase=std::uint16_t((m.read_word(g-0x734a)+2)&0x1e);m.write_word(g-0x734a,phase);
                    selector=phase+0x40;m.write_word(out,selector);initialize();return;
                }
                colour=gem&3;const auto type=m.opcode_byte(0x1545eu+(rol(gem,4)&15));
                if(!type){blank();return;}
                if(!(type&0x80))selector=type+0xb6;
                else if(type&0x20){
                    if(m.read_byte(out+0x18)!=4||m.read_byte(out+0x15)<5||(m.read_byte(out+0x19)&1)){blank();return;}
                    selector=std::uint16_t(type+0x16);
                }else if(type&0x40){
                    if(m.read_byte(out+0x15)<5||!(m.read_byte(out+0x19)&1)){blank();return;}
                    selector=std::uint16_t(type-10);
                }else{
                    if(m.read_byte(out+0x18)!=4){blank();return;}selector=type+0x36;
                }
                m.write_long(out+0x14,0);m.write_word(out,selector);initialize();return;
            }
            if(gem&0x10){colour=gem&15;colour=colour==7?5:colour&3;advance();return;}
            if(gem&0x80){
                selector=(rol(gem,5)&0x1e)+0x60;m.write_word(out,selector);colour=gem&3;
                if(m.read_word(out+0x14)!=m.read_word(out+0x16)||m.read_byte(p+0x178))initialize();else advance();return;
            }
            m.write_long(out+0x14,0);
        }
        const auto kind=gem&15;selector=kind*2;m.write_word(out,selector);
        switch(kind){
        case 0:case 8:case 13:case 14:case 15:blank();return;
        case 1:case 2:case 3:case 4:
            colour=gem&3;
            if(!pair){
                bool neighbour=false;
                for(auto offset:{2u,0xfffffffeu,0xfffffff0u,0x10u})if((m.read_word(cell+offset)&15)==gem){neighbour=true;break;}
                if(neighbour){selector+=0x9e;m.write_word(out,selector);if(selector==m.read_word(out+2)){advance();return;}}
            }
            initialize();return;
        case 5:colour=6;if(selector==m.read_word(out+2))advance();else initialize();return;
        case 6:colour=rol(gem,4)&3;initialize();return;
        case 7:{
            auto rotated=rol(gem,4);colour=rotated&3;selector=rol(rotated,5)&0x1e;
            const bool zero=selector==0;selector+=0x20;m.write_word(out,selector);
            if(!zero&&!m.read_byte(p+0x18e)&&selector==m.read_word(out+2))advance();else initialize();return;
        }
        case 9:case 10:case 11:case 12:colour=gem&3;if(selector==m.read_word(out+2))advance();else initialize();return;
        }
        (void)next; // Queue and current tables have identical recovered targets.
    }
};
}
// Exactly164 records: P1 current2/NEXT2, P2 current2/NEXT2, then78 cells
// per player. Rows use +360..+36C flags; all four pair slots use +36F.
template<class M> void build_gem_pictures(std::uint32_t g,M&m){
    gem_picture_detail::Builder<M> b{m,g};
    b.dimensions(g+0x300,g+0x3400,g+0x2f12);b.dimensions(g+0x700,g+0x3dc0,g+0x3112);b.palette();
    auto out=g+0x3300;
    for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400;
        // A2 still points at P2 after155A0; this matters to counter-like
        // pair fixtures because15394 tests A2+18E before1521E reloads A2.
        for(unsigned gem=0;gem<4;++gem,out+=0x20)b.cell(g+0x700,p+0x152+gem*2,out,p+0x36f,true,gem>=2);
    }
    for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400,field=g+0x2b12+side*0x200;
        for(unsigned row=0;row<13;++row)for(unsigned col=0;col<6;++col,out+=0x20)b.cell(p,field+row*16+col*2,out,p+0x360+row);
    }
}
struct GemPictureObject {std::uint16_t x,y,tile,attributes;};
// Source15178 and149DA object emitters. Consumers get original packed CPS2
// coordinates, tiles and attributes; source priority/block bits are retained.
// Call board first, pair second as1495A does. This is a standalone gem-only
// budget; a complete scene must supply remaining hardware budget at each call.
template<class M,class Sink> std::uint16_t emit_board_gem_objects(std::uint32_t g,M&m,Sink&&sink,std::uint16_t budget){
    if(!m.read_byte(g+0x133))return budget;
    for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400;
        if(!m.read_byte(p))continue;
        auto record=g+0x3400+side*0x9c0;std::uint16_t y=m.read_word(p+0x32a)+16;
        for(unsigned row=0;row<13;++row,y+=16){std::uint16_t x=0xa050+side*0x100;
            for(unsigned col=0;col<6;++col,record+=32,x+=16){
                if(!m.read_word(record))continue;
                if(budget&0x8000)return budget;
                budget=std::uint16_t(budget-(m.read_word(record+0xa)&15));
                sink(GemPictureObject{std::uint16_t(m.read_word(record+0xc)+x),std::uint16_t(((m.read_word(record+0xe)+y)&0x3ff)|0x2000),m.read_word(record+0x10),m.read_word(record+0x12)});
            }
        }
    }
    return budget;
}
template<class M,class Sink> std::uint16_t emit_pair_gem_objects(std::uint32_t g,M&m,Sink&&sink,std::uint16_t budget){
    auto record=g+0x3300;
    for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400;
        if(!m.read_byte(p)||!m.read_byte(p+1))continue; // source does NOT advance A4 here
        std::uint16_t x=std::uint16_t((0x50+side*0x100+(m.read_word(p+0x10)<<4))&0x3ff);
        std::uint16_t y=std::uint16_t((((m.read_word(p+0x14)-1)<<4)+m.read_word(p+0x242))&0x3ff);
        std::uint16_t selector=0;
        auto one=[&](std::uint16_t ox,std::uint16_t oy){
            //14B66 leaves D0 unchanged on exhausted budget.
            if(!(budget&0x8000)){selector=m.read_word(record);if(selector){--budget;sink(GemPictureObject{ox,oy,m.read_word(record+0x10),m.read_word(record+0x12)});}}
            record+=32;
        };
        one(x|0xa000,y|0x2000);
        if((m.read_byte(g+0xb5)&1)&&!(budget&0x8000)&&selector){--budget;sink(GemPictureObject{std::uint16_t(x|0xa000),std::uint16_t(y|0x2000),m.opcode_word(0x14b46+selector),0});}
        const auto rotation=unsigned(m.read_byte(p+0x170))*4;
        const auto sx=std::uint16_t((x+m.opcode_word(0x14b36+rotation)+m.read_word(p+0x248))&0x3ff);
        const auto sy=std::uint16_t((y+m.opcode_word(0x14b38+rotation)+m.read_word(p+0x24a))&0x3ff);
        one(sx|0xa000,sy|0x2000);
        if(m.read_byte(p)==2)continue; // current pair only; source retains A4
        one(0xe0ca+side*0x5c,0x2032);one(0xe0ca+side*0x5c,0x2022);
    }
    return budget;
}
} // namespace puzzle_fighter
