// SPDX-License-Identifier: GPL-3.0-only
// USA C120/C780 + C13E; third-pool2D930/2DB2E; HUD task2A9F8.
#pragma once
#include <cstdint>
#include <initializer_list>
namespace puzzle_fighter {
namespace attack_display_detail {
inline std::int32_t sw(std::uint16_t w){return w&0x8000u?int(w)-65536:int(w);}
inline std::uint32_t ptr(std::uint16_t w){return std::uint32_t(sw(w));}
template<class M> std::uint32_t code_long(M&m,std::uint32_t p){return (std::uint32_t(m.opcode_word(p))<<16)|m.opcode_word(p+2);}
// Exact decimal result/scratch of2D116 for the unsigned input. The original
// routine accumulates powers-of-two in eight packed-BCD bytes at globals-6CAC.
template<class M> void bcd(std::uint32_t n,std::uint32_t g,M&m){
    std::uint64_t packed=0;for(unsigned shift=0;n;n/=10,shift+=4)packed|=std::uint64_t(n%10)<<shift;
    m.write_long(g+0xffff9354u,std::uint32_t(packed>>32));m.write_long(g+0xffff9358u,std::uint32_t(packed));
}
}
// Call after EACH player's source state dispatch (BFE6 -> C106), before
// third-pool task execution. This snapshots that player's pending attack onto
// its opponent's display record. It does not alter pending attack itself.
template<class M> void snapshot_attack_display(std::uint32_t p,M&m){
    const auto pending=m.read_byte(p+0x19e);const unsigned clipped=pending>99?99:pending;
    m.write_word(m.read_long(p+0x160)+0x27a,clipped);
    m.write_byte(p+0x16e,clipped==0?0:clipped<=10?1:clipped<=30?2:3);
}
// Third pool is four512-byte actors, separate from first pool and its free
// list. Initialize once per source scene, not once per HUD frame.
template<class M> void initialize_attack_display_pool(std::uint32_t g,M&m){
    auto stack=g+0x588e;auto a=g+0x2100;m.write_byte(g+0x581e,4);
    for(unsigned i=0;i<4;++i,a+=0x200){for(unsigned j=0;j<128;++j)m.write_long(a+j*4,0);m.write_byte(a+8,0x10);stack-=2;m.write_word(stack,std::uint16_t(a));}
    m.write_word(g+0x580e,std::uint16_t(stack));
}
// Exact allocation-only prefix16C0C..16C1C; subsequent score rendering is a
// separate source unit. Zero means original free-count guard rejected it.
template<class M> std::uint32_t allocate_attack_display(std::uint32_t g,M&m){
    using namespace attack_display_detail;if(!m.read_byte(g+0x581e))return 0;
    m.write_byte(g+0x581e,m.read_byte(g+0x581e)-1);const auto stack=ptr(m.read_word(g+0x580e));const auto a=ptr(m.read_word(stack));m.write_word(g+0x580e,std::uint16_t(stack+2));m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,0x100);return a;
}
template<class M> void free_attack_display(std::uint32_t a,std::uint32_t g,M&m){
    using namespace attack_display_detail;for(unsigned offset:{0u,4u,8u,12u})m.write_long(a+offset,0);m.write_byte(a+8,0x10);const auto stack=ptr(m.read_word(g+0x580e))-2;m.write_word(stack,std::uint16_t(a));m.write_word(g+0x580e,std::uint16_t(stack));m.write_byte(g+0x581e,m.read_byte(g+0x581e)+1);
}
// Original third-pool kind1. Produces a live RAM type2 picture descriptor at
// actor+80, pointed to by actor+1C. Twelve tile/attribute longs start actor+8A;
// the coordinate-layout DATA pointer lives actor+86. There is no2D258 enqueue:
// original third-pool visibility is actor+1=1, actor+8=0x10.
template<class M,class H> bool attack_display_task(std::uint32_t a,std::uint32_t g,M&m,H&h){
    using namespace attack_display_detail;const auto phase=m.read_byte(a+4);
    if(phase==4||phase==6){free_attack_display(a,g,m);return true;}if(phase!=0&&phase!=2)return false;
    auto icons_and_digits=[&](std::uint32_t p,std::uint32_t out){
        const auto op=ptr(m.read_word(p+0x34));unsigned index=m.read_byte(op+0x16e)*4u;if(m.read_byte(g+0xb5)&8)index+=16;
        m.write_long(out,code_long(m,0x2aaf4+index));
        const auto amount=m.read_word(p+0x27a);
        if(!amount){m.write_long(out+4,0x00200000);m.write_long(out+8,0x00200000);return;}
        bcd(amount,g,m);const auto packed=m.read_word(g+0xffff935au);
        m.write_word(out+4,std::uint16_t((packed>>4)+0x150));m.write_word(out+6,1);m.write_word(out+8,(packed&15)+0x150);m.write_word(out+10,1);
    };
    auto marker=[&](std::uint32_t base,std::uint16_t index){const auto at=base+sw(std::uint16_t(index<<2));m.write_long(at,code_long(m,0x2ac28+m.read_byte(a+5)*2u));};
    auto both_markers=[&]{marker(a+0x96,m.read_byte(a+0xf));marker(a+0xae,m.read_byte(a+0xf));};
    auto winner_marker=[&]{const auto winner=m.read_word(g+0x13a);if(!winner)return;const auto p=ptr(winner);marker(a+(m.read_byte(p+0x81)?0xae:0x96),std::uint16_t(m.read_byte(p+0xa3)-1));};
    if(phase==0){
        m.write_byte(a+4,m.read_byte(a+4)+2);m.write_byte(a+1,1);m.write_byte(a+9,0);m.write_word(a+0x10,0);m.write_word(a+0x14,0);m.write_word(a+0xc,0);m.write_byte(a+0xf,0);m.write_byte(a+0xe,0x20);
        if(m.read_word(g+0x102))m.write_byte(a+5,0xe);
        if(m.read_byte(g+0x83))m.write_byte(a+5,0x1a);
        m.write_word(a+0x30,0xe000);m.write_long(a+0x1c,a+0x80);const auto table=0x2ac98u;m.write_long(a+0x86,table+sw(m.read_word(table+m.read_byte(g+0x121)*2u)));m.write_word(a+0x80,2);m.write_word(a+0x82,0x10);m.write_word(a+0x84,0xb);
        for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400,out=a+0x8a+side*0x18;icons_and_digits(p,out);for(unsigned i=0;i<3;++i){const auto index=std::uint8_t(m.read_byte(a+5)*2u+m.read_byte(p+0xa4+i));m.write_long(out+12+i*4,code_long(m,0x2ac28+index));}}
    }
    icons_and_digits(g+0x300,a+0x8a);icons_and_digits(g+0x700,a+0xa2);
    const auto sub=m.read_byte(a+5);
    auto expired=[&]{const auto v=std::uint8_t(m.read_byte(a+0xe)-1);m.write_byte(a+0xe,v);return (v&0x80)!=0;};
    auto advance=[&]{m.write_byte(a+5,m.read_byte(a+5)+2);};
    switch(sub){
    case 0:case 2:case 4:case 6:case 8:case 10:if(expired()){advance();m.write_byte(a+0xe,1);both_markers();}return true;
    case 12:if(expired()){advance();both_markers();const auto next=std::uint8_t(m.read_byte(a+0xf)+1);m.write_byte(a+0xf,next);if(next!=m.read_byte(g+0x121)){m.write_byte(a+5,0);m.write_byte(a+0xe,1);}}return true;
    case 14:if(m.read_byte(g+0x12c)==3){advance();m.write_byte(a+0xe,3);h.sound(0xe6,0x1000,0);winner_marker();}return true;
    case 16:case 18:case 20:case 22:if(expired()){advance();m.write_byte(a+0xe,3);winner_marker();}return true;
    case 24:winner_marker();return true;
    case 26:return true;
    default:return false;
    }
}
} // namespace puzzle_fighter
