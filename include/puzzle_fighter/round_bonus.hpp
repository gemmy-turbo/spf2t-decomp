// SPDX-License-Identifier: GPL-3.0-only
// USA C7DE and BEFA stopped-turn branch; kind30 at27476..279D0.
#pragma once
#include "attack_display.hpp"
#include <cstdint>
#include <stdexcept>
namespace puzzle_fighter {
template<class M> void round_bonus(std::uint32_t p,std::uint32_t g,M&m){
    if(m.read_byte(g+0x149)||!m.read_byte(g+0x107)||m.read_byte(p+0x348))return;
    m.write_byte(p+0x348,1);const auto remaining=std::uint16_t(180-m.read_word(p+0x340));
    const std::uint32_t time=(remaining&0x8000)?0:unsigned(remaining)*100u,chain=unsigned(m.read_byte(p+0x32e))*500u,big=unsigned(m.read_byte(p+0x32f))*100u,stage=unsigned(m.read_word(g+0x112))*1000u+2000u;
    m.write_long(p+0x330,time);m.write_long(p+0x334,chain);m.write_long(p+0x338,big);m.write_long(p+0x33c,stage);
    const auto total=time+chain+big+stage;m.write_long(p+0x344,m.read_long(p+0x344)+total);
    if(m.read_byte(g+0x10c)&(1u<<(m.read_byte(p+0x81)&7)))m.write_long(p+0xaa,m.read_long(p+0xaa)+total);
}
template<class M> void stopped_turns(std::uint32_t g,M&m){
    round_bonus(g+0x300,g,m);round_bonus(g+0x700,g,m);
    // Preserve the original misplaced address arithmetic, not intended fields.
    const auto base=g+0x700;
    m.write_byte(base-0x79d8,0);m.write_byte(base-0x75d8,0);
    for(auto displacement:{0x7cecu,0x78ecu,0x7baeu,0x77aeu,0x7bacu,0x77acu})m.write_word(base-displacement,0);
}
template<class M,class H> struct BonusTask {
    M&m;H&h;std::uint32_t g,a;
    static std::uint32_t rol(std::uint32_t v,unsigned n){return (v<<n)|(v>>(32-n));}
    static std::uint16_t rolw(std::uint16_t v,unsigned n){return std::uint16_t((unsigned(v)<<n)|(v>>(16-n)));}
    static std::uint32_t ptr(std::uint16_t v){return attack_display_detail::ptr(v);}
    std::uint32_t script(unsigned index){const auto table=0x84560u;return table+attack_display_detail::sw(m.read_word(table+(index&255)*2));}
    void rectangle(std::uint32_t source,std::uint32_t destination){
        const auto height=m.read_word(source+2),width=m.read_word(source+4);source+=6;
        for(unsigned x=0;x<=width;++x,destination+=0x80)for(unsigned y=0;y<=height;++y,source+=4)m.write_long(destination+y*4,m.read_long(source));
    }
    void label(unsigned index,std::uint32_t destination){const auto table=script(m.read_word(g+0x90)?8:7);rectangle(m.read_long(table+index*16+4),destination);}
    void panel_start(unsigned index){const auto frame=script(index);m.write_long(a+0x50,frame);m.write_long(a+0x54,m.read_long(frame));rectangle(m.read_long(frame+4),m.read_long(a+0x5c));}
    void panel_step(){const auto left=std::uint16_t(m.read_word(a+0x54)-1);m.write_word(a+0x54,left);if(left)return;
        auto frame=m.read_long(a+0x50)+16;if(m.read_byte(a+0x56)&0x80)frame=m.read_long(frame);
        m.write_long(a+0x50,frame);m.write_long(a+0x54,m.read_long(frame));rectangle(m.read_long(frame+4),m.read_long(a+0x5c));
    }
    std::uint32_t bcd(std::uint32_t v){attack_display_detail::bcd(v,g,m);return m.read_long(g+0xffff9358u);}
    void digit(std::uint32_t out,unsigned digit){const auto tile=m.opcode_word(0x27962u+(digit&15)*2);m.write_word(out,tile);m.write_word(out+2,0x13);m.write_word(out+4,std::uint16_t(tile+16));m.write_word(out+6,0x13);}
    void number(std::uint32_t&out,std::uint32_t value,unsigned mode){
        bool begun=false;
        if(mode==0){for(unsigned n=0;n<7;++n){value=rol(value,4);const auto d=value&15;if(d||begun){begun=true;digit(out,d);}out+=0x80;}value=rol(value,4);}
        else if(mode==1){value=rol(value,16);for(unsigned n=0;n<4;++n){const auto d=value&15;if(d||begun){begun=true;digit(out,d);}value=rol(value,4);out+=0x80;}}
        else if(mode==2){auto word=std::uint16_t(value);for(unsigned n=0;n<3;++n){word=rolw(word,4);const auto d=word&15;if(d||begun){begun=true;digit(out,d);out+=0x80;}}value=rolw(word,4);}
        else if(mode==3){auto word=rolw(std::uint16_t(value),8);for(unsigned n=0;n<2;++n){const auto d=word&15;if(d||begun){begun=true;digit(out,d);}word=rolw(word,4);out+=0x80;}value=word;}
        else if(mode==4){const auto d=(value>>4)&15;if(d)digit(out,d);out+=0x80;}
        digit(out,value&15);out+=0x80;
    }
    void units(std::uint32_t out,bool seconds){const unsigned first=seconds?0x4ca:0x4da;for(unsigned i=0;i<3;++i)m.write_long(out+4+i*0x80,((i==2?0x4ccu:first+i)<<16)|0x13);}
    std::uint32_t place(unsigned low){return (m.read_byte(a+3)?0x903000u:0x902000u)+low;}
    void advance(){m.write_byte(a+6,m.read_byte(a+6)+2);h.sound(0xe8,0x1000,0);}
    bool expired(){const auto v=std::uint8_t(m.read_byte(a+0xe)-1);m.write_byte(a+0xe,v);return v&0x80;}
    void tick(){
        const auto phase=m.read_byte(a+4);
        if(phase==4||phase==6){h.free(a);return;}
        if(phase==0){m.write_byte(a+4,2);m.write_word(a+0xc,0);m.write_byte(a+0xb,0);m.write_byte(a+9,0);m.write_word(a+0x30,0xe000);m.write_word(a+0x14,0xd0);
            const auto frame=script(3);m.write_long(a+0x50,frame);m.write_long(a+0x54,m.read_long(frame));h.init(a,0x2d,0x7da5e);return;}
        if(phase!=2)throw std::runtime_error("Invalid original bonus task phase");
        switch(m.read_byte(a+5)){
            case 0:if(m.read_byte(g+0x151)==1){m.write_byte(a+5,2);const auto winner=ptr(m.read_word(g+0x13a));m.write_byte(a+3,m.read_byte(winner+0x81));m.write_word(a+0x10,m.read_byte(a+3)?0x110:0x10);m.write_long(a+0x5c,place(0x90));rectangle(m.read_long(m.read_long(a+0x50)+4),m.read_long(a+0x5c));}return;
            case 2:if(m.read_byte(a+0x57))m.write_byte(a+5,4);panel_step();return;
            case 6:if(m.read_word(a+0x56)&0x8000){m.write_byte(a+4,4);m.write_byte(g+0x151,0);}panel_step();return;
            case 4:break;
            default:throw std::runtime_error("Invalid original bonus substep");
        }
        if(m.read_byte(g+0x151)==2){m.write_byte(a+5,6);panel_start(4);}
        const auto winner=ptr(m.read_word(g+0x13a));std::uint32_t out=0;
        switch(m.read_byte(a+6)){
        case 0:advance();m.write_byte(a+0xe,8);{const auto v=bcd(m.read_long(winner+0x33c));out=place(0x134);label(0,place(0x12c));out+=0x180;number(out,v,1);units(out,false);}break;
        case 2:if(expired()){advance();m.write_byte(a+0xe,8);const auto v=bcd(m.read_word(winner+0x340));out=place(0x148);label(2,place(0x140));out+=0x200;number(out,v,3);units(out,true);}break;
        case 4:if(expired()){advance();m.write_byte(a+0xe,60);const auto v=bcd(m.read_long(winner+0x330));out=place(0x15c);label(3,place(0x154));out+=0x180;number(out,v,1);units(out,false);}break;
        case 6:if(expired()){advance();m.write_byte(a+0xe,8);rectangle(m.read_long(m.read_long(a+0x50)+4),m.read_long(a+0x5c));}break;
        case 8:if(expired()){advance();m.write_byte(a+0xe,8);const auto count=m.read_byte(winner+0x32e);const auto v=bcd(count);out=place(0x134);label(4,place(0x12c));const auto original_out=out;number(out,v,4);m.write_long(out+4,0x04cd0013);out+=0x80;number(out,0x500,3);m.write_long(out+4,0x04ce0013);out+=0x80;auto half=std::uint16_t(count>>1);if(count&1)half+=0x5000;number(out,rolw(half,12),2);out=original_out+0x484;units(out,false);}break;
        case 10:if(expired()){advance();m.write_byte(a+0xe,8);const auto v=bcd(m.read_byte(winner+0x32f));out=place(0x148);label(5,place(0x140));const auto original_out=out;number(out,v,4);m.write_long(out+4,0x04cd0013);out+=0x80;number(out,0x100,3);m.write_long(out+4,0x04ce0013);out+=0x80;number(out,rolw(std::uint16_t(v),8),2);out=original_out+0x484;units(out,false);}break;
        case 12:if(expired()){advance();const auto v=bcd(m.read_long(winner+0x344));out=place(0x15c);label(6,place(0x154));out+=0x80;number(out,v,0);}break;
        default:break;
        }
        h.display(a);
    }
};
} // namespace puzzle_fighter
