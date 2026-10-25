// SPDX-License-Identifier: GPL-3.0-only
// USA normal character controller13C60 and source animation event pipeline.
#pragma once
#include "global_rng.hpp"
#include <array>
#include <cstdint>
#include <stdexcept>
#include <initializer_list>
#include <string>
namespace puzzle_fighter {
// USA2D85E..2D88F, after the original first/third-pool initializers. The
// original then falls through display reset2D890, owned by the scene caller.
template<class M> void initialize_character_auxiliary_pool(std::uint32_t g,M&m){
    auto stack=g+0x5886u;auto a=g+0x1d00u;m.write_byte(g+0x581d,16);
    for(unsigned i=0;i<16;++i,a+=64){for(unsigned n=0;n<16;++n)m.write_long(a+n*4,0);m.write_byte(a+8,12);stack-=2;m.write_word(stack,std::uint16_t(a));}
    m.write_word(g+0x580c,std::uint16_t(stack));
}
// USA2DB08. Only the first16 bytes are cleared; remaining task fields persist.
template<class M> void free_character_second_pool(std::uint32_t a,std::uint32_t g,M&m){
    for(unsigned o:{0u,4u,8u,12u})m.write_long(a+o,0);
    m.write_byte(a+8,12);const auto raw=m.read_word(g+0x580c);
    const auto stack=std::uint32_t(raw&0x8000?int(raw)-65536:int(raw))-2;
    m.write_word(stack,std::uint16_t(a));m.write_word(g+0x580c,std::uint16_t(stack));m.write_byte(g+0x581d,m.read_byte(g+0x581d)+1);
}
//1446C ->2BEB8: chain_display supplies the already-formed character/variant/
// level index. The intermediate OPCODE BYTE table is mandatory; the index
// itself is not the command variation.
template<class M,class Sink> void character_chain_voice(std::uint32_t player,std::uint32_t g,std::uint16_t index,M&m,Sink&&sink){
    if(m.read_byte(g+0xbc))return;
    const auto delta=m.opcode_byte(0x14496u+std::uint32_t(index&0x8000u?int(index)-65536:int(index)));
    const auto base=m.opcode_word(0x2bee6u+2*m.read_byte(player+0x82));
    auto command=std::uint16_t((base&0xff00u)|std::uint8_t(base+delta));
    if(m.read_byte(player+0x81))command=std::uint16_t(command+0x200);
    sink(command,(m.read_byte(player+0x81)?24u:8u)<<8,0);
}
// First-pool feedback actors29EF0(kind43) and27CBC(kind33). H is the existing
// ActorTasks helper contract init/step/display/free/sound. These routines do
// not allocate or free second-pool records.
template<class M,class H> bool character_feedback_task(std::uint32_t a,std::uint32_t g,M&m,H&h){
    const auto signed_word=[](std::uint16_t x){return x&0x8000?int(x)-65536:int(x);};
    const auto parent=std::uint32_t(signed_word(m.read_word(a+0x32)));
    const auto phase=m.read_byte(a+4),kind=m.read_byte(a+2);
    auto inc=[&](unsigned o){m.write_byte(a+o,m.read_byte(a+o)+2);};
    auto copy_word=[&](unsigned o){m.write_word(a+o,m.read_word(parent+o));};
    auto walk=[&]{m.write_long(a+0x10,m.read_long(a+0x10)+m.read_long(a+0x20));m.write_long(a+0x20,m.read_long(a+0x20)+m.read_long(a+0x28));};
    if(kind==0x43){switch(phase){
        case 0:{
            inc(4);copy_word(0x10);m.write_word(a+0x24,m.read_word(parent+0x10));copy_word(0x14);copy_word(0x30);m.write_byte(a+0x3a,1);copy_word(0xc);m.write_byte(a+0xb,m.read_byte(parent+0xb));m.write_byte(a+9,4);m.write_byte(a+0xa,0);
            const bool flipped=m.read_byte(a+0xb)!=0;
            m.write_word(a+0x10,std::uint16_t(m.read_word(a+0x10)+(flipped?-0xe0:0xe0)));m.write_long(a+0x20,flipped?0x48000:0xfffb8000);
            m.write_word(a+0x24,std::uint16_t(m.read_word(a+0x24)+(flipped?-0x2c:0x2c)));m.write_word(a+0x14,m.read_word(a+0x14)+0x40);h.init(a,0xb,0x674ba);return true;
        }
        case 2:
            if(m.read_byte(g+0x12a)){inc(4);return true;}
            m.write_long(a+0x10,m.read_long(a+0x10)+m.read_long(a+0x20));
            if((!m.read_byte(a+0xb)&&signed_word(m.read_word(a+0x24))>signed_word(m.read_word(a+0x10)))||(m.read_byte(a+0xb)&&signed_word(m.read_word(a+0x24))<signed_word(m.read_word(a+0x10)))){m.write_word(a+0x10,m.read_word(a+0x24));m.write_byte(a+0xa,0xff);inc(4);}h.display(a);return true;
        case 4:inc(4);return true;
        case 6:h.free(a);return true;
        default:return false;
    }}
    if(kind!=0x33)return false;
    switch(phase){
    case 0:
        inc(4);copy_word(0x30);m.write_byte(a+0xb,m.read_byte(parent+0xb));copy_word(0xc);m.write_byte(a+0xd,m.read_byte(a+0xd)+1);m.write_byte(a+0xc,0xff);m.write_byte(a+9,6);copy_word(0x10);copy_word(0x14);
        for(auto o:{0x24u,0x28u,0x2cu})m.write_long(a+o,0);
        m.write_long(a+0x20,m.read_byte(a+0xb)?0x40000:0xfffc0000);h.init(a,9,0x7ab6c);return true;
    case 2:
        switch(m.read_byte(a+5)){
        case 0:
            walk();if((m.read_byte(a+0xb)&&signed_word(m.read_word(a+0x10))>0x98)||(!m.read_byte(a+0xb)&&signed_word(m.read_word(a+0x10))<0xe8)){
                inc(5);if(!m.read_byte(g+0xbc))h.sound(m.opcode_word(0x2c7b0u+2*(m.read_byte(parent+0x81)&1)),(m.read_byte(parent+0x81)?20u:12u)<<8,0);h.init(a,4,0x7ab6c);
            }break;
        case 2:if(m.read_byte(a+0x1a)&0x80){inc(5);m.write_byte(a+0xa,0xff);}break;
        case 4:if(m.read_byte(a+0x24)){inc(5);h.init(a,7,0x7ab6c);}break;
        case 6:if(m.read_byte(a+0x2c)){
            inc(5);h.init(a,9,0x7ab6c);if(!m.read_byte(g+0xbc))h.sound(m.opcode_word(0x2c75au+2*(m.read_byte(parent+0x81)&1)),(m.read_byte(a+0x81)?24u:8u)<<8,0);
            m.write_byte(a+0xb,m.read_byte(a+0xb)^1);m.write_byte(a+0xe,0x40);m.write_long(a+0x20,m.read_byte(a+0xb)?0x40000:0xfffc0000);
        }break;
        case 8:walk();m.write_byte(a+0xe,m.read_byte(a+0xe)-1);if(!m.read_byte(a+0xe))inc(4);break;
        default:return false;
        }
        h.step(a);h.display(a);return true;
    case 4:inc(4);return true;
    case 6:h.free(a);return true;
    default:return false;
    }
}
// Named original helper entries recovered below as native allocation/state
// operations. H supplies only the host sound sink; it never executes code.
enum class CharacterHelper:std::uint32_t {
    intro_2457a=0x2457a,idle_245f8=0x245f8,attack_setup=0x247a8,attack_tick=0x26e72,
    hurt=0x24bea,hurt_fire=0x2648c,hurt_electric=0x264c0,finish_260c4=0x260c4,
    finish_26822=0x26822,finish_26ab0=0x26ab0,win_299ca=0x299ca,
    intro_2811c=0x2811c,screen_dark=0xbbc8,screen_restore=0xbbe8
};
template<class M,class H> struct CharacterController {
    M&m;H&h;std::uint32_t g;
    std::uint32_t actor=0,work=0; // A6 and observable A4 helper context
    static int sw(std::uint16_t x){return x&0x8000?int(x)-65536:int(x);}
    static int sb(std::uint8_t x){return x&0x80?int(x)-256:int(x);}
    static std::uint32_t ptr(std::uint16_t x){return std::uint32_t(sw(x));}
    std::uint8_t b(unsigned o){return m.read_byte(actor+o);}std::uint16_t w(unsigned o){return m.read_word(actor+o);}
    void byte(unsigned o,unsigned v){m.write_byte(actor+o,std::uint8_t(v));}void word(unsigned o,unsigned v){m.write_word(actor+o,std::uint16_t(v));}
    void next(unsigned o){byte(o,b(o)+2);}
    std::uint32_t code_long(std::uint32_t a){return(std::uint32_t(m.opcode_word(a))<<16)|m.opcode_word(a+2);}
    std::uint16_t random(){const auto r=global_random(m.read_word(g+0xae));m.write_word(g+0xae,r.seed);return r.value;}
    void effect(CharacterHelper e,int d0=0,int d1=0){
        switch(e){
        case CharacterHelper::intro_2457a:for(unsigned n=2;n<8;++n)if(!spawn(0x0d00+n))return;return;
        case CharacterHelper::idle_245f8:if(spawn(0x0d00))spawn(0x0d01);return;
        case CharacterHelper::attack_setup:
            if(spawn(0x0f00)){m.write_byte(work+0x28,0);m.write_word(work+0x10,w(0x10));m.write_word(work+0x14,w(0x14));if(b(0xb))d0=-d0;m.write_word(work+0x10,std::uint16_t(m.read_word(work+0x10)+d0));m.write_word(work+0x14,std::uint16_t(m.read_word(work+0x14)+d1));}return;
        case CharacterHelper::hurt:spawn(0x1300);return;
        case CharacterHelper::hurt_fire:case CharacterHelper::hurt_electric:
            for(unsigned n=0;n<2;++n){if(!spawn(e==CharacterHelper::hurt_fire?0x2300:0x2301))return;m.write_byte(work+0xe,n);}return;
        case CharacterHelper::finish_260c4:for(unsigned n=0;n<4;++n)if(!spawn(0x2000))return;return;
        case CharacterHelper::finish_26822:
            for(auto n:{1u,0u,2u,3u}){if(!spawn(0x2600+n))return;m.write_byte(work+9,n==1?2:4);}return;
        case CharacterHelper::finish_26ab0:for(unsigned n=0;n<6;++n)if(!spawn(0x2900+n))return;return;
        case CharacterHelper::win_299ca:for(unsigned n=0;n<8;++n)if(!spawn(0x4000+n))return;return;
        case CharacterHelper::intro_2811c:case CharacterHelper::attack_tick:
            byte(0xf,b(0xf)-1);if(!b(0xf)){byte(0xf,4);spawn(e==CharacterHelper::intro_2811c?0x3500:0x2c00);}return;
        case CharacterHelper::screen_dark:
            m.write_byte(g+0x125,0xff);m.write_byte(g+0xb01,0);m.write_byte(g+0xc01,0);for(unsigned n=0;n<8;++n)m.write_long(0x90cfc0+n*4,0xffffffff);return;
        case CharacterHelper::screen_restore:
            if(m.read_byte(g+0x125)==0xff)m.write_byte(g+0x125,0);
            m.write_byte(g+0xb01,1);m.write_byte(g+0xc01,1);return;
        }
        throw std::runtime_error("Unrecovered original character helper");
    }
    void sound(std::uint16_t n){h.sound(n,(b(0x81)?24u:8u)<<8,0);}
    void voice(std::uint16_t n){sound(std::uint16_t(n+(b(0x81)?0x200:0)));}
    void side_sound(std::uint32_t table,bool gated=true){if(!gated||!m.read_byte(g+0xbc))sound(m.opcode_word(table+2*(b(0x81)&1)));}
    void win_voice(bool losing=false){
        if(m.read_byte(g+0xbc))return;
        const auto index=(unsigned(b(0x81)&1)<<3)+((random()&3)*2)+(unsigned(b(0x82))<<4);
        sound(m.opcode_word((losing?0x2cceeu:0x2cbfau)+index));
    }
    void palette(std::uint32_t source,unsigned blocks=4,std::uint32_t destination=0){
        if(!destination)destination=b(2)?0x90c200:0x90c180;
        for(unsigned i=0;i<blocks*8;++i)m.write_long(destination+4*i,m.read_long(source+4*i)|(m.read_long(destination+4*i)&0xf000f000));
    }
    void restore_palette(){const auto index=std::uint16_t((unsigned(b(0x82))*2u&0xff00u)|std::uint8_t(unsigned(b(0x82))*2u+b(0x9f)));palette(0xa0000u+std::uint32_t(sw(std::uint16_t(index<<7))));}
    void alternate_palette(bool electric){palette(code_long(0x147f2+b(0x82)*4)+(electric?0x60:0),3);}
    bool allocate(bool second=false){
        const auto count=g+(second?0x581d:0x581c),head=g+(second?0x580c:0x580a);
        if(!m.read_byte(count))return false;
        m.write_byte(count,m.read_byte(count)-1);const auto stack=ptr(m.read_word(head));work=ptr(m.read_word(stack));m.write_word(head,std::uint16_t(stack+2));return true;
    }
    bool spawn(std::uint16_t kind,bool second=false,bool byte_kind=false){
        if(!allocate(second))return false;
        m.write_byte(work,m.read_byte(work)+1);
        if(byte_kind)m.write_byte(work+2,std::uint8_t(kind));else m.write_word(work+2,kind);
        m.write_word(work+0x32,std::uint16_t(actor));return true;
    }
    void callback(std::uint32_t entry); // exact source sound descriptors below
    void frame(std::uint32_t at){
        m.write_long(actor+0x1c,at);m.write_long(actor+0x18,m.read_long(at));word(0xd4,m.read_word(at+8));
        const auto event=m.read_byte(at+0xb);if(!event)return;
        const auto table=code_long(0x14422+b(0x82)*4);callback(m.read_long(table+event*4));
    }
    void animation(unsigned index){const auto table=code_long(0x143f6+b(0x82)*4);frame(table+std::uint32_t(sw(m.read_word(table+std::uint16_t(index*2)))));}
    void step(){word(0x18,w(0x18)-1);if(w(0x18))return;auto at=m.read_long(actor+0x1c)+16;if(b(0x1a)&0x80)at=m.read_long(at);frame(at);}
    void position(bool initial=false){
        auto table=0x14902u;
        if(initial&&!m.read_word(g+0x102)&&!(b(0x82)==9&&(!b(0x9e)||m.read_byte(g+0x138))))table=0x148aa;
        const auto i=(unsigned(b(0x82))*2+b(2))*4;word(0x10,m.opcode_word(table+i));word(0x14,m.opcode_word(table+i+2));word(0xd6,m.opcode_word(table+i+2));
    }
    unsigned danger_index(){
        work=ptr(w(0x32));auto index=m.read_byte(work+0xd1);const auto opponent=ptr(w(0x34));
        if(m.read_byte(opponent+0xd1)==2&&!m.read_byte(work+0xd1)){index=3;if(b(0x82)==9&&b(0xb))index=0x10;}
        return index;
    }
    void round_transition(){
        if(b(5)||!m.read_byte(g+0x107))return;
        if(std::uint8_t(m.read_byte(g+0x10c)-1)==b(2)){
            if(!m.read_byte(work+0xd9)){m.write_long(actor+4,0x02040000);for(auto o:{0x127u,0x11cu,0x11du,0x11eu})m.write_byte(g+o,0);}
        }else m.write_long(actor+4,0x02060000);
    }
    void idle(){m.write_long(actor+4,0x02000000);restore_palette();byte(0xd9,0);const auto index=danger_index();byte(0xd1,index);animation(index);round_transition();}
    void taunt(){
        if(m.read_byte(g+0x10e)||b(0xda)||b(0x9e))return;
        const auto pressed=std::uint8_t(~m.read_byte(g+0x61))&m.read_byte(g+0x60);
        if(!(pressed&(1u<<(b(0x81)&1))))return;
        byte(0xda,0xff);
        if(!m.read_byte(g+0x107)&&!b(0xd9)){byte(0xd0,0);m.write_long(actor+4,0x020a0000);restore_palette();}
    }
    void idle_frame(){taunt();const auto index=danger_index();if(index!=b(0xd1)){byte(0xd1,index);animation(index);}round_transition();if(b(0x82)==0&&b(0x1b)){byte(0x1b,0);effect(CharacterHelper::idle_245f8);}step();}
    void initialize_extra(){
        switch(b(0x82)){
        case 1:case 3:spawn(0x0b00);break;
        case 4:if(spawn(0x0200,true))word(0x36,work);break;
        case 5:spawn(0x3b05);word(0x38,work);if(spawn(1,true,true)){word(0x36,work);if(spawn(3,true,true))word(0x38,work);}break;
        case 8:if(spawn(0,true,true)){word(0x38,work);m.write_byte(work+3,b(0x82));}break;
        case 9:palette(0xa0a00,1,b(0x81)?0x90c220:0x90c1a0);break;
        default:break;
        }
    }
    void initialize(){
        m.write_long(actor+4,0x02080000);word(0x30,0xa000);work=ptr(w(0x32));
        for(auto o:{0x82u,0x81u,0x9eu})byte(o,m.read_byte(work+o));
        byte(0xda,0);byte(0xdc,0);
        word(0xc,b(2)?0xff04:0);word(0x34,g+(b(2)?0xb00:0xc00));byte(0xb,b(2)^1);position(true);
        if(m.read_word(g+0x102))idle();else animation(b(0x82)==9&&(!b(0x9e)||m.read_byte(g+0x138))?0x12:9);
        initialize_extra();
    }
    void hurt(){
        switch(b(6)){
        case 0:next(6);position();restore_palette();if(b(0xd2)){effect(CharacterHelper::hurt);if(b(0xd2)==3){alternate_palette(false);effect(CharacterHelper::hurt_fire);animation(8);return;}if(b(0xd2)==4){alternate_palette(true);effect(CharacterHelper::hurt_electric);animation(8);return;}}animation(b(0xd2)+6);return;
        case 2:if(b(0x1a)){restore_palette();idle();}else step();return;
        default:throw std::runtime_error("Unsupported original hurt state");
        }
    }
    void win_timer(){if(b(0xe)){byte(0xe,b(0xe)-1);if(!b(0xe))m.write_byte(g+0x10b,m.read_byte(g+0x10b)|(1u<<(b(2)&7)));}}
    void win();void intro();void attack();
    void lose(){switch(b(6)){
        case 0:next(6);position();byte(0xe,0x18);if(b(0x82)==7)word(0x14,w(0x14)-2);if(b(0x82)==8)spawn(0x2b00);m.write_byte(g+0x10b,m.read_byte(g+0x10b)|(1u<<(b(2)&7)));animation(5);return;
        case 2:byte(0xe,b(0xe)-1);if(!b(0xe)){next(6);if(!b(0xdc)){byte(0xdc,0xff);win_voice(true);}}step();return;
        case 4:step();return;default:throw std::runtime_error("Unsupported original loss state");}}
    void chain(){if(!b(6)){next(6);animation(std::uint8_t(0x18+b(0xd8)));}else if(b(6)==2){if(b(0x1a))idle();else step();}else throw std::runtime_error("Unsupported original chain state");}
    void one(std::uint32_t a){actor=a;switch(b(4)){
        case 0:initialize();return;case 4:case 6:return;case 2:break;default:throw std::runtime_error("Unsupported original character state");}
        switch(b(5)){case 0:idle_frame();return;case 2:hurt();return;case 4:win();return;case 6:lose();return;case 8:intro();return;case 10:if(b(0xd0)&&m.read_byte(g+0x107)&&std::uint8_t(m.read_byte(g+0x10c)-1)==b(2)){m.write_byte(g+0x11c,0x3c);m.write_byte(g+0x11d,3);m.write_byte(g+0x11e,3);}attack();return;case 12:chain();return;default:throw std::runtime_error("Unsupported original character substate");}
    }
    //140B8 precedes13C60 in the normal caller. Opponent script+08 events,
    // not host attack counters, cause the defender's original hurt transition.
    void reactions(){for(unsigned side=0;side<2;++side){
        actor=g+0xb00+side*0x100;const auto state=w(4);if(state!=0x0200&&state!=0x0206&&state!=0x0202)continue;
        work=ptr(w(0x34));const auto hit=m.read_byte(work+0xd4);if(!hit)continue;
        const auto kind=std::uint8_t(hit-1);if(kind){const auto index=std::uint16_t(sb(kind)*4+2*(b(2)&1));h.sound(m.opcode_word(0x2cdd4u+std::uint32_t(sw(index))),0x1000,0);}
        m.write_byte(work+0xd4,0);byte(0xd2,m.read_byte(work+0xd5));
        if(b(5)!=2&&!m.read_byte(g+0xbc)){const auto table=m.read_byte(work+0xd0)?0x2cb06u:0x2ca12u;const auto index=(unsigned(b(0x81)&1)<<3)+2*(random()&3)+(unsigned(b(0x82))<<4);sound(m.opcode_word(table+index));}
        m.write_long(actor+4,0x02020000);
    }}
    void run(){
        unsigned held=0;if(m.read_byte(g+0x11c)){held=1;m.write_byte(g+0x11d,m.read_byte(g+0x11d)-1);if(!m.read_byte(g+0x11d)){m.write_byte(g+0x11c,m.read_byte(g+0x11c)-1);m.write_byte(g+0x11d,m.read_byte(g+0x11e));held=0;}}
        m.write_byte(g+0x127,held);if(!held){one(g+0xb00);one(g+0xc00);}
        const auto first=m.read_byte(m.read_long(g+0xb1c)+15),second=m.read_byte(m.read_long(g+0xc1c)+15);m.write_byte(g+0x119,first>=second?first:0);
    }
};

template<class M,class H> void CharacterController<M,H>::win(){
    const auto ch=b(0x82);
    if(!b(6)){
        next(6);byte(0xe,0xb4);position();
        if(ch==5){const auto r=random()&1;byte(0xa,r);animation(m.opcode_byte(0x220d0+r));}
        else if(ch==10){const auto r=random()&7;byte(0xa,m.opcode_byte(0x22cec+r));animation(m.opcode_byte(0x22cf4+r));}
        else animation(4);
        return;
    }
    if(b(6)!=2)throw std::runtime_error("Unsupported original win state");
    if(ch==5&&b(0xa)){
        win_timer();switch(b(7)){
        case 0:if(spawn(0x43,false,true)){next(7);word(0x2c,work);}return;
        case 2:if(b(0x1b)){work=ptr(w(0x2c));if(m.read_word(work)){if(!m.read_byte(work+0xa))return;next(7);byte(0x1b,0);}}step();return;
        case 4:if(b(0x1b)){byte(0x1b,0);win_voice();}step();return;
        default:throw std::runtime_error("Unsupported original win auxiliary state");}
    }
    if(ch==9){win_timer();if(b(0x1b)){byte(0x1b,0);effect(CharacterHelper::win_299ca);}step();return;}
    if(ch==10){
        if(b(0x1b)){byte(0x1b,0);if(!m.read_byte(g+0xbc)){
            if(b(0xa)<3){const auto table=std::array<std::uint32_t,3>{0x22d54,0x22d96,0x22dd8}[b(0xa)];voice(m.opcode_word(table+(random()&2)));}
            else if(b(0xa)<5)voice(b(0xa)==3?0x25a:0x25b);else throw std::runtime_error("Unsupported original character10 win variant");
        }}win_timer();step();return;
    }
    if(b(0x1b)){byte(0x1b,0);win_voice();}win_timer();step();
}
template<class M,class H> void CharacterController<M,H>::intro(){
    const auto ch=b(0x82);auto vertical=[&]{m.write_long(actor+0x14,m.read_long(actor+0x14)+m.read_long(actor+0x24));m.write_long(actor+0x24,m.read_long(actor+0x24)+m.read_long(actor+0x2c));};
    auto horizontal=[&]{m.write_long(actor+0x10,m.read_long(actor+0x10)+m.read_long(actor+0x20));m.write_long(actor+0x20,m.read_long(actor+0x20)+m.read_long(actor+0x28));};
    if(ch==0){if(!b(7)){next(7);effect(CharacterHelper::intro_2457a);return;}if(b(7)!=2)throw std::runtime_error("Unsupported character0 intro state");if(b(0x1a))idle();else step();return;}
    if(ch==3){if(!b(7)){next(7);m.write_long(actor+0x24,0x60000);m.write_long(actor+0x2c,0xffff8000);return;}if(b(7)!=2)throw std::runtime_error("Unsupported character3 intro state");if(b(0x1a)&0x80){word(0x14,w(0xd6));idle();return;}if(b(0x1b))vertical();step();return;}
    if(ch==4||ch==7){switch(b(6)){
        case 0:next(6);m.write_long(actor+0x20,b(0xb)?std::uint32_t(-int(ch==4?0x40000:0x60000)):(ch==4?0x40000:0x60000));m.write_long(actor+0x28,0);return;
        case 2:
            horizontal();if(ch==4&&b(0x1b)){byte(0x1b,0);spawn(9,false,true);}
            if((b(0xb)&&sw(w(0x10))<0xf0)||(!b(0xb)&&sw(w(0x10))>0x90)){next(6);m.write_long(actor+0x20,b(0xb)?0xfffb0000:0x50000);m.write_long(actor+0x28,b(0xb)?0x2000:0xffffe000);animation(0xf);return;}step();return;
        case 4:horizontal();if((b(0xb)&&sw(w(0x10))<0x98)||(!b(0xb)&&sw(w(0x10))>0xe8))next(6);step();return;
        case 6:if(b(0x1a))idle();else step();return;
        default:throw std::runtime_error("Unsupported original horizontal intro state");}
    }
    if(ch==10){switch(b(6)){
        case 0:next(6);m.write_long(actor+0x20,b(0xb)?0x40000:0xfffc0000);m.write_long(actor+0x28,0);return;
        case 2:horizontal();if((b(0xb)&&sw(w(0x10))>0x98)||(!b(0xb)&&sw(w(0x10))<0xe8)){next(6);animation(0xf);}else step();return;
        case 4:if(b(0x1a)&0x80)idle();else step();return;
        default:throw std::runtime_error("Unsupported character10 intro state");}
    }
    if(ch==9&&b(0x9e)&&!m.read_byte(g+0x138)){
        switch(b(6)){
        case 0:if(m.read_byte(g+0x581c)<1)return;next(6);byte(0xf,1);if(spawn(0x3300))word(0xe0,work);return;
        case 2:work=ptr(w(0xe0));if(m.read_byte(work+0xa)){next(6);side_sound(0x2c8f8,false);m.write_long(actor+0x20,b(0xb)?0x60000:0xfffa0000);}return;
        case 4:m.write_long(actor+0x10,m.read_long(actor+0x10)+m.read_long(actor+0x20));effect(CharacterHelper::intro_2811c);if((b(0xb)&&sw(w(0x10))>0x98)||(!b(0xb)&&sw(w(0x10))<0xe8)){next(6);work=ptr(w(0xe0));m.write_byte(work+0x24,0xff);byte(0xe,0x38);effect(CharacterHelper::screen_dark);}return;
        case 6:byte(0xe,b(0xe)-1);if(!b(0xe)){next(6);effect(CharacterHelper::screen_restore);animation(0);work=ptr(w(0xe0));m.write_byte(work+0x2c,0xff);}if(b(0xe)&3){effect(CharacterHelper::hurt);side_sound(0x2beb4,false);}return;
        case 8:break;default:throw std::runtime_error("Unsupported arcade Akuma intro state");}
    }
    if(b(0x1a)){if(ch==9&&b(0x9e)){m.write_byte(g+0x138,1);restore_palette();}idle();return;}
    if(ch==5&&b(0x1b)){byte(0x1b,0);if(spawn(0x0e00))spawn(0x0e01);}
    step();
}
template<class M,class H> void CharacterController<M,H>::attack(){
    const auto ch=b(0x82),strength=b(0xd0),state=b(7);
    if(strength>10||(strength&1))throw std::runtime_error("Unsupported original attack strength");
    auto finished=[&](bool negative=false){if(negative?(b(0x1a)&0x80)!=0:b(0x1a)!=0){idle();return true;}return false;};
    auto particles=[&]{byte(0xf,b(0xf)-1);if(!b(0xf)){byte(0xf,4);spawn(0x2c00);}};
    auto projectile=[&](unsigned kind){if(spawn(kind)){m.write_byte(work+0x28,0);}};
    auto vertical=[&]{m.write_long(actor+0x14,m.read_long(actor+0x14)+m.read_long(actor+0x24));m.write_long(actor+0x24,m.read_long(actor+0x24)+m.read_long(actor+0x2c));};
    auto flash=[&](std::uint32_t table){byte(0xe0,b(0xe0)-1);if(!(b(0xe0)&0x80))return;byte(0xe0,2);byte(0xe1,b(0xe1)+1);if(b(0xe1)==7)byte(0xe1,0);palette(table+b(0xe1)*32,1);};
    if(!strength){
        if(!state){next(7);unsigned index=10;if(ch==2||ch==9)index+=random()&1;if(ch==4)index=m.opcode_byte(0x21ea4+(random()&7));animation(index);return;}
        if(state!=2)throw std::runtime_error("Unsupported original taunt attack state");
        if(finished(ch!=1))return;
        if(ch!=9&&b(0x1b)){
            byte(0x1b,0);static constexpr std::array<unsigned,11> masks{14,6,14,6,14,6,14,14,14,0,14};
            static constexpr std::array<std::uint32_t,11> tables{0x2170c,0x21826,0x21952,0x21b22,0x21ee6,0x22200,0x2237a,0x22626,0x22856,0,0x22ec0};
            static constexpr std::array<unsigned,11> fallback{0x12d,0x274,0x27c,0x184,0x288,0x28c,0x297,0x29f,0x2a2,0,0x2ac};
            const auto index=random()&masks[ch];auto command=m.opcode_word(tables[ch]+index);if(m.read_byte(g+0xbc))command=fallback[ch];voice(command);
        }step();return;
    }
    const bool middle=strength<(ch==6?10:8);
    if(middle&&ch!=3&&ch!=7){
        if(!state){next(7);if(ch==6){word(0xe0,0);if(strength==8)flash(0xa9ba0);}animation((ch==10?10+strength/2:(ch==2||ch==9?12:11)+(strength-2)/2));return;}
        if(state!=2)throw std::runtime_error("Unsupported original medium attack state");
        if(ch==6&&strength==8)flash(0xa9ba0);
        const bool end=ch==9?(b(0x1a)&0x80)!=0:b(0x1a)!=0;
        if(end){if(ch==6||ch==9)restore_palette();idle();return;}
        if((ch==2||ch==4||ch==6||ch==9)&&b(0x1b)){
            byte(0x1b,0);
            if(ch==2)projectile(0x1200);
            if(ch==4)spawn(0x1600);
            if(ch==6&&spawn(0x0800))side_sound(0x2c94c,false);
            if(ch==9){palette(0xaa100u+(strength==2?0:0x20),1,b(2)?0x90c220:0x90c1a0);projectile(0x1207);}
        }step();return;
    }
    if(ch==0||ch==8||ch==10){if(!state){next(7);animation(14);}else if(state==2){if(!finished())step();}else throw std::runtime_error("Unsupported original strong attack state");return;}
    if(ch==1){if(!state){next(7);byte(0xf,1);effect(CharacterHelper::attack_setup,-28,19);animation(14);}else if(state==2){particles();if(!finished())step();}else throw std::runtime_error("Unsupported character1 strong state");return;}
    if(ch==2){switch(state){
        case 0:next(7);byte(0xf,1);animation(15);return;
        case 2:particles();if(b(0x1b)){next(7);byte(0x1b,0);effect(CharacterHelper::attack_setup,20,26);}step();return;
        case 4:particles();if(finished(true))return;if(b(0x1b)){byte(0x1b,0);projectile(0x1200);}step();return;
        default:throw std::runtime_error("Unsupported character2 strong state");}}
    if(ch==4||ch==5){
        const auto table=ch==4?0xa9800u:0xa99a0u;
        if(!state){next(7);word(0xe0,0);flash(table);animation(14);return;}
        if(state!=2)throw std::runtime_error("Unsupported palette attack state");
        flash(table);if(b(0x1a)){restore_palette();idle();return;}
        if(b(0x1b)){byte(0x1b,0);if(ch==4){m.write_byte(g+0x124,0x30);effect(CharacterHelper::finish_260c4);}else{
            //22286 deliberately lacks the allocator-failure guard.
            allocate();m.write_byte(work,m.read_byte(work)+1);m.write_word(work+2,0x1000);m.write_word(work+0x32,std::uint16_t(actor));
        }}step();return;
    }
    if(ch==6){switch(state){
        case 0:next(7);effect(CharacterHelper::finish_26822);byte(0xe,0x20);animation(15);return;
        case 2:byte(0xe,b(0xe)-1);if(!b(0xe)){next(7);byte(0xe,0x30);effect(CharacterHelper::finish_26ab0);animation(16);}else step();return;
        case 4:byte(0xe,b(0xe)-1);if(!b(0xe)){next(7);animation(17);}else step();return;
        case 6:if(!finished(true))step();return;default:throw std::runtime_error("Unsupported character6 strong state");}}
    if(ch==7){
        if(middle){switch(state){
            case 0:next(7);animation(11+(strength-2)/2);return;
            case 2:if(b(0x1b)){next(7);byte(0x1b,0);if(spawn(0x14,false,true)){m.write_byte(work+3,(strength-2)/2);byte(0xa,0);}}step();return;
            case 4:if(b(0x1b)){next(7);byte(0x1b,0);if(spawn(0x12,false,true)){m.write_byte(work+0x28,0);m.write_byte(work+3,(strength-2)/2+2);}}step();return;
            case 6:if(!finished())step();return;default:throw std::runtime_error("Unsupported character7 medium state");}
        }
        switch(state){
        case 0:next(7);byte(0xf,1);animation(14);return;
        case 2:particles();if(b(0x1b)){next(7);byte(0x1b,0);effect(CharacterHelper::attack_setup,13,34);if(spawn(0x1402))byte(0xa,0);}step();return;
        case 4:particles();if(b(0x1b)){next(7);byte(0x1b,0);projectile(0x1205);}step();return;
        case 6:particles();if(!finished())step();return;default:throw std::runtime_error("Unsupported character7 strong state");}
    }
    if(ch==9){switch(state){
        case 0:next(7);byte(0xf,1);animation(15);return;
        case 2:if(b(0x1b)){next(7);byte(0x1b,0);particles();spawn(0x11,false,true);}step();return;
        case 4:particles();if(b(0x1a)&0x80){restore_palette();idle();return;}if(b(0x1b)){byte(0x1b,0);palette(0xaa120,1,b(2)?0x90c220:0x90c1a0);projectile(0x1208);}step();return;
        default:throw std::runtime_error("Unsupported Akuma strong state");}}
    if(ch==3){
        if(strength==2){if(!state){next(7);animation(11);return;}if(state!=2)throw std::runtime_error("Unsupported character3 projectile state");if(b(0x1a)&0x80){restore_palette();idle();return;}if(b(0x1b)){palette(0xa9700,1,b(2)?0x90c220:0x90c1a0);byte(0x1b,0);projectile(0x1206);}step();return;}
        if(strength<8){switch(state){
            case 0:next(7);m.write_long(actor+0x24,0x50000);m.write_long(actor+0x2c,0xffffc000);animation(strength==4?12:13);return;
            case 2:if(!b(0x1b)){byte(0xd9,1);vertical();if(m.read_long(actor+0x24)&0x80000000u){next(7);animation(strength?15:14);return;}}step();return;
            case 4:vertical();if(sw(w(0xd6))>=sw(w(0x14))){word(0x14,w(0xd6));idle();}else step();return;
            default:throw std::runtime_error("Unsupported character3 jump state");}
        }
        switch(state){
        case 0:next(7);m.write_long(actor+0x24,0x40000);m.write_long(actor+0x2c,0xffffc000);byte(0xf,1);effect(CharacterHelper::attack_setup,-25,22);animation(16);return;
        case 2:particles();if(!b(0x1b)){byte(0xd9,1);vertical();if(m.read_long(actor+0x24)&0x80000000u){next(7);animation(17);return;}}step();return;
        case 4:particles();vertical();if(sw(w(0xd6))>=sw(w(0x14))){next(7);word(0x14,w(0xd6));m.write_long(actor+0x24,0x50000);m.write_long(actor+0x2c,0xffffc000);animation(18);}else step();return;
        case 6:particles();if(!b(0x1b)){vertical();if(m.read_long(actor+0x24)&0x80000000u){next(7);animation(19);return;}}step();return;
        case 8:particles();vertical();if(sw(w(0xd6))>=sw(w(0x14))){word(0x14,w(0xd6));idle();}else step();return;
        default:throw std::runtime_error("Unsupported character3 strong state");}
    }
    throw std::runtime_error("Unsupported original character attack");
}
// All normal animation event targets from original DATA callback tables.
template<class M,class H> void CharacterController<M,H>::callback(std::uint32_t entry){
    struct Entry {std::uint32_t entry,table;bool voice,work_pan;};
    static constexpr Entry entries[]={
        {0x2be64,0x2be8a,false,false},
        {0x2bf1c,0x2bf42,false,false},
        {0x2bf46,0x2bf74,true,false},
        {0x2bf78,0x2bf9e,false,false},
        {0x2bfa2,0x2bfc8,false,false},
        {0x2bfcc,0x2bffa,true,false},
        {0x2bffe,0x2c02c,true,false},
        {0x2c030,0x2c05e,true,false},
        {0x2c062,0x2c090,true,false},
        {0x2c094,0x2c0c2,true,false},
        {0x2c0c6,0x2c0f4,true,false},
        {0x2c0f8,0x2c126,true,false},
        {0x2c12a,0x2c158,true,false},
        {0x2c15c,0x2c18a,true,false},
        {0x2c18e,0x2c1bc,true,false},
        {0x2c1c0,0x2c1e6,false,false},
        {0x2c1ea,0x2c210,false,false},
        {0x2c214,0x2c23a,false,false},
        {0x2c23e,0x2c264,false,false},
        {0x2c268,0x2c296,true,false},
        {0x2c29a,0x2c2c8,true,false},
        {0x2c2cc,0x2c2f2,false,false},
        {0x2c2f6,0x2c31c,false,false},
        {0x2c320,0x2c34e,true,false},
        {0x2c352,0x2c378,false,false},
        {0x2c37c,0x2c3aa,true,false},
        {0x2c3ae,0x2c3dc,true,false},
        {0x2c3e0,0x2c40e,true,false},
        {0x2c412,0x2c440,true,false},
        {0x2c444,0x2c46a,false,false},
        {0x2c46e,0x2c494,false,false},
        {0x2c498,0x2c4c6,true,false},
        {0x2c4ca,0x2c4f8,true,false},
        {0x2c4fc,0x2c52a,true,false},
        {0x2c52e,0x2c55c,true,false},
        {0x2c560,0x2c586,false,false},
        {0x2c58a,0x2c5b0,false,false},
        {0x2c5b4,0x2c5da,false,false},
        {0x2c5de,0x2c60c,true,false},
        {0x2c610,0x2c636,false,false},
        {0x2c63a,0x2c660,false,false},
        {0x2c664,0x2c692,true,false},
        {0x2c696,0x2c6c4,true,false},
        {0x2c6c8,0x2c6f6,true,false},
        {0x2c6fa,0x2c728,true,false},
        {0x2c800,0x2c826,false,false},
        {0x2c854,0x2c87a,false,false},
        {0x2c87e,0x2c8a4,false,false},
        {0x2c8a8,0x2c8ce,false,false},
        {0x2c8fc,0x2c922,false,false},
        {0x2c9a4,0x2c9ca,false,true},
    };
    for(const auto&e:entries)if(e.entry==entry){
        if(e.voice&&m.read_byte(g+0xbc))return;
        const auto command=m.opcode_word(e.table+2*(b(0x81)&1));
        const auto pan=e.work_pan?(m.read_byte(work+0x81)?20u:12u):(b(0x81)?24u:8u);
        h.sound(command,pan<<8,0);return;
    }
    throw std::runtime_error("Unrecovered original character sound callback "+std::to_string(entry));
}
} // namespace puzzle_fighter
