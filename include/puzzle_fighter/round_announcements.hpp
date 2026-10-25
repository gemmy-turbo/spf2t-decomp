// SPDX-License-Identifier: GPL-3.0-only
// Original USA 167A8..16834, 23336..23798, A908..A994.
#pragma once
#include <array>
#include <cstdint>
namespace puzzle_fighter {
namespace announcement_detail {
inline std::uint32_t address(std::uint16_t v){return std::uint32_t(std::int32_t(std::int16_t(v)));}
template<class M> void increment_byte(M&m,std::uint32_t a){m.write_byte(a,std::uint8_t(m.read_byte(a)+2));}
template<class M> bool decrement_byte(M&m,std::uint32_t a){const auto v=std::uint8_t(m.read_byte(a)-1);m.write_byte(a,v);return (v&0x80)!=0;}
template<class M> bool decrement_word(M&m,std::uint32_t a){const auto v=std::uint16_t(m.read_word(a)-1);m.write_word(a,v);return (v&0x8000)!=0;}
template<class M> void move_vertical(std::uint32_t a,M&m){m.write_long(a+0x14,m.read_long(a+0x14)+m.read_long(a+0x24));m.write_long(a+0x24,m.read_long(a+0x24)+m.read_long(a+0x2c));}
template<class H> bool announcement_sound(unsigned n,unsigned winner,H&h){
    constexpr unsigned commands[]={0x90,0x91,0x92,0x93,0x94,0x95,0x97,0x98,0x96};
    if(n>=9)return false;
    h.sound(commands[n]+(n==7?(winner&1):0),0x1000,0);return true;
}
}
// Exact atomic five-slot setup. Slot 4 is kind0x30, a separate task; caller
// must dispatch that kind separately. No blanket clearing: source relies on
// pool initialization/free. A zero array means the original capacity guard.
template<class M> std::array<std::uint32_t,5> allocate_round_announcements(std::uint32_t g,M&m){
    using namespace announcement_detail;std::array<std::uint32_t,5> out{};
    if(m.read_byte(g+0x581c)<5)return out;
    for(unsigned i=0;i<5;++i){
        m.write_byte(g+0x581c,std::uint8_t(m.read_byte(g+0x581c)-1));
        const auto stack=address(m.read_word(g+0x580a)),a=address(m.read_word(stack));
        m.write_word(g+0x580a,std::uint16_t(stack+2));out[i]=a;
        m.write_byte(a,std::uint8_t(m.read_byte(a)+1));
        constexpr unsigned kinds[]={0x300,0x301,0x305,0x306,0x3000};m.write_word(a+2,kinds[i]);
        if(i==0)m.write_byte(a+0xa,m.read_byte(g+0x121)==1?8:std::uint8_t(m.read_word(g+0x102)));
        if(i==1)m.write_byte(a+0xa,7);
        if(i==2||i==3){m.write_byte(a+0xa,0x19);m.write_word(a+0x10,i==2?0x10:0x110);m.write_word(a+0x32,i==2?0x8300:0x8700);}
    }return out;
}
// Helpers: init(actor,index,0x7DA5E), step(actor), display(actor), free(actor),
// sound(command,pan,extra). False explicitly denotes unsupported menu variant3
// or corrupt/out-of-range task state; it must not be silently ignored.
template<class M,class H> bool round_announcement_task(std::uint32_t a,std::uint32_t g,M&m,H&h){
    using namespace announcement_detail;
    const auto phase=m.read_byte(a+4),variant=m.read_byte(a+3),sub=m.read_byte(a+5);
    if(phase==4||phase==6){h.free(a);return true;}
    if(phase==0){increment_byte(m,a+4);m.write_byte(a+9,0);m.write_word(a+0x30,0xe000);if(variant==3)m.write_byte(a+7,0xff);h.init(a,m.read_byte(a+0xa),0x7da5e);return true;}
    if(phase!=2||variant>8||variant==3)return false;
    auto advance=[&]{increment_byte(m,a+5);};
    auto finish=[&]{increment_byte(m,a+4);};
    auto render=[&]{h.step(a);h.display(a);};
    auto cancel=[&]{m.write_byte(a+4,4);};
    if(variant==0){
        if(m.read_byte(g+0x8a)){cancel();return true;}
        switch(sub){
        case 0:if(m.read_byte(g+0x12c)==1){advance();m.write_byte(a+0xe,0xb);m.write_word(a+0x10,0x180);m.write_word(a+0x14,0x80);}return true;
        case 2:m.write_word(a+0x10,m.read_word(a+0x10)-0x10);if(decrement_byte(m,a+0xe)){advance();m.write_byte(a+0xe,0x2f);m.write_word(a+0x10,0xc0);if(!announcement_sound(m.read_byte(a+0xa),0,h))return false;}break;
        case 4:if(decrement_byte(m,a+0xe)){advance();m.write_byte(a+0xe,0xb);}break;
        case 6:m.write_word(a+0x10,m.read_word(a+0x10)-0x10);if(decrement_byte(m,a+0xe)){advance();m.write_byte(a+0xe,0xb);m.write_word(a+0x10,0x180);}break;
        case 8:m.write_word(a+0x10,m.read_word(a+0x10)-0x10);if(decrement_byte(m,a+0xe)){advance();m.write_byte(a+0xe,0x2f);m.write_word(a+0x10,0xc0);announcement_sound(6,0,h);}break;
        case 10:if(decrement_byte(m,a+0xe))advance();break;
        case 12:m.write_word(a+0x10,m.read_word(a+0x10)-0x10);if(m.read_word(a+0x1a)&0x8000){finish();m.write_byte(g+0x12c,0);}break;
        default:return false;
        }render();return true;
    }
    if(variant==1){
        if(m.read_byte(g+0x8a)){cancel();return true;}
        if(sub==0){if(m.read_byte(g+0x12c)==2){advance();m.write_word(a+0x10,0xc0);m.write_word(a+0x14,0x80);announcement_sound(7,m.read_byte(g+0x145),h);}return true;}
        if(sub!=2)return false;
        if(m.read_word(a+0x1a)&0x8000)finish();
        render();return true;
    }
    if(variant==2){if(m.read_byte(g+0x104))finish();h.display(a);return true;}
    if(variant==4){h.display(a);return true;}
    if(variant==5||variant==6){
        switch(sub){
        case 0:if(m.read_byte(g+0x12c)==3){advance();m.write_word(a+0x14,0x100);m.write_long(a+0x24,0);m.write_long(a+0x2c,0xffffc000);const auto owner=address(m.read_word(a+0x32));if(!(m.read_byte(g+0x10c)&(1u<<(m.read_byte(owner+0x81)&7))))h.init(a,0x1a,0x7da5e);}return true;
        case 2:move_vertical(a,m);if(m.read_byte(a+0x1b)){advance();const auto neg=0u-m.read_long(a+0x24);m.write_long(a+0x24,(neg>>1)|(neg&0x80000000));m.write_byte(a+0x1b,0);}break;
        case 4:move_vertical(a,m);if(m.read_byte(a+0x1b)){advance();if(variant==5)m.write_byte(g+0x12c,4);}break;
        case 6:if(!m.read_byte(a+0x1b))advance();break;
        case 8:move_vertical(a,m);if(m.read_word(a+0x1a)&0x8000){finish();m.write_byte(g+0x12c,0);}break;
        default:return false;
        }render();return true;
    }
    if(variant==7){
        if(sub==0){advance();m.write_byte(a+0xf,0x1f);const auto n=m.read_byte(a+0xe);m.write_word(a+0x10,m.opcode_word(0x2366a+n));m.write_word(a+0x14,m.opcode_word(0x2366eu+n));m.write_word(a+0x20,m.opcode_word(0x2367a+n));m.write_word(a+0x24,m.opcode_word(0x2367eu+n));}
        else if(sub==2){m.write_word(a+0x10,m.read_word(a+0x10)+m.read_word(a+0x20));m.write_word(a+0x14,m.read_word(a+0x14)+m.read_word(a+0x24));if(decrement_byte(m,a+0xf)){if(m.read_byte(a+0xe))cancel();else advance();}}
        else if(sub!=4)return false;
        h.display(a);return true;
    }
    if(m.read_byte(g+0x8a)){cancel();return true;}
    switch(sub){
    case 0:advance();m.write_word(a+0xe,0x1f);m.write_word(a+0x10,0xc0);m.write_word(a+0x14,0x120);m.write_long(a+0x24,0xfff40000);m.write_long(a+0x2c,0x6000);break;
    case 2:move_vertical(a,m);if(decrement_word(m,a+0xe))advance();break;
    case 4:if(m.read_byte(g+0x12c)==1){advance();m.write_word(a+0xe,0x80);}break;
    case 6:if(decrement_word(m,a+0xe)){advance();m.write_word(a+0xe,0x1f);m.write_long(a+0x2c,0xffffa000);}break;
    case 8:move_vertical(a,m);if(decrement_word(m,a+0xe))cancel();break;
    default:return false;
    }h.display(a);return true;
}
// A908..A994 only. Substep6 belongs to arcade post-round progression and is
// deliberately reported to the caller, not guessed here. Sound preserves the
// full queue command and payload generated by original2B26A.
template<class M,class H> bool round_won_announcement_phase(std::uint32_t g,M&m,H&h){
    using namespace announcement_detail;
    const auto sub=m.read_word(g+0x10);if(sub>4||sub&1)return false;
    if(!m.read_byte(g+0x149))m.write_byte(g+0x10e,1);
    auto advance=[&]{m.write_word(g+0x10,m.read_word(g+0x10)+2);};
    if(sub==0){if(m.read_byte(g+0x125)){m.write_byte(g+0x125,m.read_byte(g+0x125)-1);return true;}advance();m.write_byte(g+0x12c,2);m.write_byte(g+0x145,m.read_byte(address(m.read_word(g+0x13a))+0x81));if(m.read_byte(g+0x126))h.sound(0x0200ff06,0xff000000,0);return true;}
    if(sub==2){if(m.read_byte(g+0x149))return true;advance();m.write_byte(g+0x12c,3);
        for(const auto p:{g+0x300,g+0x700})if(!m.read_byte(p+0x9e)){const auto v=m.read_byte(p+0x32e);if(v>=m.read_byte(p+0xaf))m.write_byte(p+0xaf,v);const auto w=m.read_byte(p+0x32f);if(w>=m.read_byte(p+0xb1))m.write_byte(p+0xb1,w);}
        return true;}
    if(m.read_byte(g+0x12c)==4){advance();h.sound(m.read_byte(address(m.read_word(g+0x13a))+0x9e)?0x9b:0x9a,0x1000,0);}return true;
}
} // namespace puzzle_fighter
