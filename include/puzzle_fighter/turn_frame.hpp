// SPDX-License-Identifier: GPL-3.0-only
// Original USA BF48..C105 ordered two-player turn caller and its poststep
// banner/score helpers. The stopped BEFA -> C7DE bonus path is a separate unit.
#pragma once
#include "global_rng.hpp"
#include "pair_generation_memory.hpp"
#include "settle_gems.hpp"
#include "tidy_field.hpp"
#include "counter_phase.hpp"
#include "form_gems_dcd6_e0ee.hpp"
#include "form_gems_dcca.hpp"
#include "board_picture_codes.hpp"
#include "find_breaks.hpp"
#include "big_gem_bonus.hpp"
#include "show_break.hpp"
#include "join_gems_f060.hpp"
#include "join_gems_eeb0.hpp"
#include "join_gems_ecec.hpp"
#include "counter_drop.hpp"
#include "loss_step.hpp"
#include "attack_display.hpp"
#include <array>
#include <stdexcept>
namespace puzzle_fighter {
namespace turn_frame_detail {
inline int sw(std::uint16_t x){return x&0x8000?int(x)-65536:int(x);}
inline int sb(std::uint8_t x){return x&0x80?int(x)-256:int(x);}
}
// USA1479E ->14756. The chain display effect is a real palette write;
// adapters must call this rather than treating chain_palette as audio only.
template<class M> void turn_chain_palette(std::uint32_t actor,M&m){
    const auto out=m.read_byte(actor+2)?0x90c200u:0x90c180u;
    auto index=std::uint16_t(m.read_byte(actor+0x82)*2);
    index=std::uint16_t((index&0xff00u)|std::uint8_t(index+m.read_byte(actor+0x9f)));
    const auto source=0xa0000u+std::uint32_t(turn_frame_detail::sw(std::uint16_t(index<<7)));
    for(unsigned n=0;n<32;++n)m.write_long(out+4*n,m.read_long(source+4*n)|(m.read_long(out+4*n)&0xf000f000));
}
// USA C8F8 and its four source-selected banners. These tables are DATA
// accesses through A1, unlike the opcode-space dispatch offsets at C940.
template<class M> void turn_banner(std::uint32_t p,M&m){
    const auto timer=m.read_byte(p+0x381);
    if(timer==1)clear_banner(p,m);
    if(timer==0x39)clear_banner(p,m);
    auto out=m.read_long(p+0x382);
    if(turn_frame_detail::sb(timer)<2)out-=8;
    else if(turn_frame_detail::sb(timer)<0x3a)out-=4;
    auto tile=[&](std::uint16_t code,unsigned advance){out+=advance;m.write_word(out,code);m.write_word(out+2,5);m.write_word(out+4,std::uint16_t(code+16));m.write_word(out+6,5);};
    switch(m.read_byte(p+0x38d)){
    case 0:{
        auto chain=m.read_byte(p+0x38e);if(chain==1)return;if(chain>9)chain=10;
        out+=0x100;const auto table=0xcc28u+4u*chain;
        m.write_word(out,m.read_word(table));m.write_word(out+2,5);m.write_word(out+4,m.read_word(table+2));m.write_word(out+6,5);
        out+=0x80;for(auto code:{0xc3,0xc8,0xc1,0xc9,0xce,0x81})tile(code,0x80);return;
    }
    case 1:{unsigned n=0;for(auto code:{0xe4,0xc5,0xc3,0xc8,0xc2,0xcf,0xce,0xe5,0xe3})tile(code,n++==4?0x100:0x80);return;}
    case 2:{unsigned n=0;for(auto code:{0xc1,0xcc,0xcc,0xc3,0xcc,0xc5,0xc1,0xe2,0x81})tile(code,n++==3?0x100:0x80);return;}
    case 3:{unsigned n=0;for(auto code:{0xa1,0xe3,0xe4,0xc1,0xe4,0xe4,0xc1,0xc3,0xcb})tile(code,n++==3?0x100:0x80);return;}
    default:throw std::runtime_error("Unsupported original turn banner index");
    }
}
// USA11208/1120C. Retain the source subtraction and signed-result test,
// including wrapping scores, and all seven pairs of original DATA tiles.
template<class M> void turn_score_tiles(std::uint32_t p,std::uint32_t g,M&m){
    const bool second=m.read_byte(p+0x180)==2;
    auto score=m.read_long(p+0xaa),out=second?0x900d64u:0x9007e4u;
    if(m.read_byte(g+0x83))return;
    for(unsigned digit=0;digit<7;++digit,out+=0x80){
        const auto divisor=m.read_long(0x1125c+4*digit);std::uint16_t count=0;
        for(;;){const auto next=score-divisor;if(next&0x80000000u)break;score=next;++count;if(count>1000)throw std::runtime_error("Unsupported original score domain");}
        const auto table=0x11278u+std::uint32_t(turn_frame_detail::sw(std::uint16_t(count<<2)));
        m.write_word(out,m.read_word(table));m.write_word(out+2,second?2:1);m.write_word(out+4,m.read_word(table+2));m.write_word(out+6,second?2:1);
    }
}
template<class M> void turn_poststep(std::uint32_t p,std::uint32_t g,M&m){
    if(m.read_byte(p+0x381)){
        m.write_byte(p+0x381,m.read_byte(p+0x381)-1);
        if(m.read_byte(p+0x381))turn_banner(p,m);else clear_banner(p,m);
    }
    turn_score_tiles(p,g,m);snapshot_attack_display(p,m);
    if(m.read_word(p+0x32a)){
        m.write_word(p+0x32a,m.read_word(p+0x32a)+1);
        if(turn_frame_detail::sw(m.read_word(p+0x32a))>3)m.write_word(p+0x32a,0);
    }
    if(m.read_byte(m.read_long(p+0x160)+0x17e)==20)return;
    m.write_word(p+0x342,m.read_word(p+0x342)+1);
    if(turn_frame_detail::sw(m.read_word(p+0x342))<60)return;
    m.write_word(p+0x342,0);m.write_word(p+0x340,m.read_word(p+0x340)+1);
    if(turn_frame_detail::sw(m.read_word(p+0x340))>539)m.write_word(p+0x340,539);
}
struct TurnPlayerObservation {
    std::uint8_t step{};bool started{};
    unsigned formations{},removed{};
    CounterDropResult drop{};bool dropped{};
};
struct TurnFrameResult {std::array<TurnPlayerObservation,2> players{};};
// Hooks are the already-recovered complete1166C steering and C31E end-turn
// callers: h.steering(side), h.end_turn(side). They share this exact Memory.
// Effects has the existing settled/break/counter sound contract. Results are
// host observers only; no observer participates in original control flow.
template<class M,class E,class H>
TurnFrameResult turn_frame(std::uint32_t g,M&m,E&effects,H&h){
    TurnFrameResult result;
    for(unsigned i=0;i<2;++i){
        const auto p=g+0x300u+i*0x400u;auto&o=result.players[i];
        if(!m.read_byte(p+0x9e)){m.write_word(p+0x140,m.read_word(p+0x9a));m.write_word(p+0x142,m.read_word(p+0x9c));}
        if(!m.read_byte(p+0x328)&&m.read_byte(p+0x17e)!=20){
            m.write_byte(p+0x328,1);advance_pair_memory(p,g,m);m.write_word(p+0x14,3);o.started=true;
        }
        m.write_byte(p+0x171,0);o.step=m.read_byte(p+0x17e);
        if(o.step>20)throw std::runtime_error("Unsupported original turn state");
        if(o.step)m.write_byte(p+0x266,1);
        auto advance=[&]{m.write_byte(p+0x17e,m.read_byte(p+0x17e)+1);};
        auto picture=[&]{board_picture_codes(p,m);tidy_field(p,m);};
        switch(o.step){
        case 0:{
            m.write_byte(g+0x149,0);
            for(auto off:{0x252u,0x3a6u,0x261u})m.write_byte(p+off,0);
            for(auto off:{0x254u,0x256u,0x258u,0x25au,0x306u,0x308u})m.write_word(p+off,0);
            for(auto off:{0x37cu,0x37au,0x396u})m.write_byte(p+off,0);
            const auto r=global_random(m.read_word(g+0xae));m.write_word(g+0xae,r.seed);m.write_byte(p+0x37b,r.value&7);
            if(!(m.read_word(g+0x80)&63))m.write_byte(p+0x266,1);
            h.steering(i);break;
        }
        case 1:case 3:case 10:m.write_byte(g+0x149,1);settle_gems(p,m,effects);tidy_field(p,m);break;
        case 2:counter_phase(p,g,m,effects);break;
        case 4:case 15:advance();o.formations+=form_gems_dcaa(p,m).formations;picture();m.write_byte(p+0x266,1);break;
        case 5:advance();break;
        case 6:advance();o.formations+=form_gems_dcb6(p,m).formations;picture();m.write_byte(p+0x266,1);break;
        case 7:advance();o.formations+=form_gems_dcca(p,m).formed;picture();break;
        case 8:find_breaks(p,g,m,effects);break;
        case 9:advance();if(m.read_word(p+0x25a))m.write_word(p+0x258,m.read_word(p+0x25a));big_gem_bonus(p,m);break;
        case 11:o.removed+=show_break(p,g,m,m).score.removal.removed;break;
        case 12:advance();o.formations+=join_gems_f060(p,m).merges;picture();break;
        case 13:advance();o.formations+=join_gems_eeb0(p,m).merges;picture();break;
        case 14:advance();o.formations+=join_gems_ecec(p,m).merges;picture();break;
        case 16:{advance();const auto op=m.read_long(p+0x160);m.write_byte(op+0x244,m.read_byte(op+0x19e));break;}
        case 17:advance();count_field(p,g,m);break;
        case 18:advance();if(m.read_byte(m.read_long(p+0x160)+0x17e)!=20){if(!m.read_byte(p+0x18d)){o.drop=counter_drop(p,g,m);o.dropped=true;}m.write_byte(p+0x18d,0);}break;
        case 19:h.end_turn(i);break;
        case 20:loss_step(p,g,m);break;
        }
        turn_poststep(p,g,m);
    }
    if(m.read_byte(g+0x47e)==20&&m.read_byte(g+0x87e)==20){m.write_byte(g+0x149,0);clear_banner(g+0x700,m);}
    if(!(m.read_word(g+0x80)&3))for(auto off:{0x2be0u,0x2fe0u,0x2de0u,0x31e0u})for(unsigned n=0;n<4;++n)m.write_long(g+off+n*4,0x00ff00ff);
    return result;
}
} // namespace puzzle_fighter
