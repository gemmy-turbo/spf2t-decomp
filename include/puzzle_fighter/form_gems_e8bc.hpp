// SPDX-License-Identifier: GPL-3.0-only
// USA E8BC..ECA9. Instruction-derived candidate; validation pending.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct FormGemsE8bcResult {unsigned candidates{},formed{};};
// Live player scratch/pointer writes are part of this routine's gameplay state.
// In particular, the outer signed counter admits seven origins, not six.
template<class Memory>
FormGemsE8bcResult form_gems_e8bc(std::uint32_t p,Memory&m) {
    FormGemsE8bcResult result;
    const auto long_read=[&](std::uint32_t a){const auto hi=m.read_word(a);return (std::uint32_t(hi)<<16)|m.read_word(a+2u);};
    const auto long_move=[&](std::uint32_t a,std::uint32_t v){m.write_word(a,static_cast<std::uint16_t>(v>>16));m.write_word(a+2u,static_cast<std::uint16_t>(v));};
    const auto long_add=[&](std::uint32_t a,std::uint32_t n){const auto v=long_read(a)+n;m.write_word(a+2u,static_cast<std::uint16_t>(v));m.write_word(a,static_cast<std::uint16_t>(v>>16));};
    const auto word_add=[&](std::uint32_t a,std::uint16_t n){m.write_word(a,static_cast<std::uint16_t>(m.read_word(a)+n));};
    const auto signed_less=[](std::uint16_t a,std::uint16_t b){return (a^0x8000u)<(b^0x8000u);};
    const auto above=[&](std::uint32_t a){const auto b=long_read(p+0x398u);return (a^0x80000000u)>(b^0x80000000u);};
    m.write_byte(p+0x267u,0);
    long_move(p+0x212u,long_read(p+0x164u)+0xccu);
    long_move(p+0x216u,long_read(p+0x168u)+0xccu);
    m.write_word(p+0x238u,0);
    do {
        auto board=long_read(p+0x212u),picture=long_read(p+0x216u);
        for(unsigned scan=0;scan<11;++scan,board-=0x10u,picture-=0x10u) {
            std::uint32_t a2{},a3{},a4{};
            std::uint16_t colour{},v{},height{},width{},remaining{},span{};
            m.write_byte(p+0x232u,0);m.write_byte(p+0x230u,0);m.write_byte(p+0x233u,0);m.write_word(p+0x236u,0);
            v=m.read_word(board);
            if(!v||v==0xffu||(v&15u)==6u||(v&15u)==7u)continue;
            v=m.read_byte(board+0x400u);
            if(v) {
                if(m.read_byte(picture+1u)!=4u)continue;
                if(m.read_byte(p+0x230u)&&static_cast<std::uint8_t>(v)!=m.read_byte(p+0x230u))continue;
                word_add(p+0x236u,1);m.write_byte(p+0x233u,1);m.write_byte(p+0x230u,m.read_byte(board+0x400u));
            }
            colour=m.read_word(board)&15u;++result.candidates;
            a2=board;
            long_move(p+0x21au,board);long_move(p+0x222u,board);long_move(p+0x21eu,picture);long_move(p+0x226u,picture);
            height=10;
            do {
                a2-=0x10u;long_add(p+0x226u,0xfffffff0u);a3=long_read(p+0x226u);
                v=m.read_word(a3)&15u;
                if(v==4u) {
                    if(m.read_byte(p+0x230u)) {
                        v=m.read_byte(a2+0x400u);
                        if(static_cast<std::uint8_t>(v)!=m.read_byte(p+0x230u))break;
                    }
                    m.write_byte(p+0x233u,1);m.write_byte(p+0x230u,m.read_byte(a2+0x400u));
                }
                v=m.read_word(a2);
                if(v&0x80u) {
                    if(!m.read_byte(p+0x233u))break;
                    v=m.read_byte(a2+0x400u);if(static_cast<std::uint8_t>(v)!=m.read_byte(p+0x230u))break;
                }
                v=m.read_word(a2)&15u;
                if(!v||v==7u||v!=colour)break;
                if(!above(a2))break;
                a3=long_read(p+0x226u);word_add(p+0x236u,m.read_word(a3)&15u);
            }while(--height!=0xffffu);
            height=static_cast<std::uint16_t>(10u-height);
            if(!height)continue;
            m.write_word(p+0x1e6u,height);m.write_word(p+0x386u,static_cast<std::uint16_t>(height-1u));
            a3=long_read(p+0x21au);width=5;
            do {
                a3-=2u;long_add(p+0x21eu,0xfffffffeu);v=m.read_byte(a3+0x400u);
                if(v) {
                    a2=long_read(p+0x21eu);
                    if(m.read_byte(a2+1u)==4u) {
                        if(m.read_byte(p+0x230u)&&static_cast<std::uint8_t>(v)!=m.read_byte(p+0x230u))break;
                        m.write_byte(p+0x233u,1);m.write_byte(p+0x230u,m.read_byte(a3+0x400u));
                    }
                    if(!m.read_byte(p+0x233u))break;
                    if(static_cast<std::uint8_t>(v)!=m.read_byte(p+0x230u))break;
                }
                v=m.read_word(a3)&15u;if(v==7u||v!=colour)break;
                a2=long_read(p+0x21eu);word_add(p+0x236u,m.read_word(a2)&255u);
                a4=a3;long_move(p+0x22au,long_read(p+0x21eu));remaining=m.read_word(p+0x1e6u);
                do {
                    a4-=0x10u;long_add(p+0x22au,0xfffffff0u);v=m.read_byte(a3+0x400u);
                    if(v) {
                        if(!m.read_byte(p+0x233u))break;
                        if(static_cast<std::uint8_t>(v)!=m.read_byte(p+0x230u))break;
                    }
                    v=m.read_word(a4)&15u;if(v==7u||v!=colour)break;
                    if(!above(a4))break;
                    a2=long_read(p+0x22au);word_add(p+0x236u,m.read_word(a2)&15u);
                    a2=long_read(p+0x22au);
                    if(m.read_byte(a2+1u)==4u) {
                        bool matching=true;
                        if(m.read_byte(p+0x230u)) {
                            v=m.read_byte(a3+0x400u);matching=static_cast<std::uint8_t>(v)==m.read_byte(p+0x230u);
                        }
                        if(matching){m.write_byte(p+0x233u,1);m.write_byte(p+0x230u,m.read_byte(a4+0x400u));}
                    }
                }while(--remaining!=0xffffu);
                if(remaining==m.read_word(p+0x1e6u))break;
                span=static_cast<std::uint16_t>(m.read_word(p+0x1e6u)-remaining);
                if(signed_less(span,m.read_word(p+0x1e6u)))m.write_word(p+0x1e6u,span);
            }while(--width!=0xffffu);
            width=static_cast<std::uint16_t>(5u-width);height=m.read_word(p+0x1e6u);
            if(!width)continue;
            if(m.read_byte(p+0x233u)) {
                m.write_word(p+0x23cu,width);m.write_word(p+0x236u,0);long_move(p+0x21eu,picture);
                do {
                    a2=long_read(p+0x21eu);remaining=height;
                    do {word_add(p+0x236u,m.read_byte(a2+1u));a2-=0x10u;}while(--remaining!=0xffffu);
                    long_add(p+0x21eu,0xfffffffeu);
                }while(--width!=0xffffu);
                width=m.read_word(p+0x23cu);v=m.read_word(p+0x236u);
                if(v&&static_cast<std::uint8_t>(v)!=15u&&static_cast<std::uint8_t>(v)!=30u)continue;
            }
            long_move(p+0x21au,board);long_move(p+0x21eu,picture);
            m.write_byte(p+0x32cu,static_cast<std::uint8_t>(m.read_byte(p+0x32cu)+1u));
            v=m.read_byte(p+0x32cu);m.write_byte(p+0x32cu,static_cast<std::uint8_t>(v&255u));if(!v)m.write_byte(p+0x32cu,1);
            m.write_byte(p+0x194u,static_cast<std::uint8_t>(m.read_byte(p+0x194u)+1u));
            m.write_byte(p+0x194u,static_cast<std::uint8_t>(m.read_byte(p+0x194u)&7u));
            if(m.read_byte(p+0x194u)==7u||!m.read_byte(p+0x194u))m.write_byte(p+0x194u,1);
            m.write_byte(board+0x400u,m.read_byte(p+0x32cu));m.write_word(board,m.read_word(board)|0x80u);m.write_byte(picture+1u,4);
            m.write_byte(p+0x22eu,0);m.write_byte(p+0x22fu,0);
            do {
                a3=long_read(p+0x21au);a2=long_read(p+0x21eu);remaining=height;
                do {
                    if(!(m.read_word(a3)&0x80u))m.write_byte(p+0x267u,1);
                    m.write_word(a3,m.read_word(a3)|0x80u);m.write_word(a2,0);m.write_byte(a2,m.read_byte(p+0x32cu));
                    if(!width&&!m.read_byte(p+0x22fu)){m.write_byte(a2+1u,1);m.write_byte(p+0x22fu,1);}
                    a3-=0x10u;a2-=0x10u;
                }while(--remaining!=0xffffu);
                if(!m.read_byte(p+0x22eu)){m.write_byte(a2+0x11u,8);m.write_byte(p+0x22eu,1);}
                long_add(p+0x21au,0xfffffffeu);long_add(p+0x21eu,0xfffffffeu);
            }while(--width!=0xffffu);
            m.write_byte(a2+0x11u,2);m.write_byte(board+0x400u,m.read_byte(p+0x32cu));
            m.write_word(board,m.read_word(board)|0x80u);m.write_byte(picture+1u,4);m.write_byte(p+0x171u,1);++result.formed;
        }
        long_add(p+0x212u,0xfffffffeu);long_add(p+0x216u,0xfffffffeu);word_add(p+0x238u,1);
    }while(!signed_less(6,m.read_word(p+0x238u)));
    if(m.read_byte(p+0x267u))m.write_byte(p+0x266u,1);
    return result;
}
} // namespace puzzle_fighter
