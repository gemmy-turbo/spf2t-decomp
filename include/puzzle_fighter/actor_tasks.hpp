// SPDX-License-Identifier: GPL-3.0-only
// USA 2D258/2D304/2D31C/2D334/2D35C/2DAC4 and25B0E.
#pragma once
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
namespace puzzle_fighter {
template<class Memory,class Sound> struct ActorTasks {
    Memory&m;Sound emit;std::uint32_t globals;
    static std::uint32_t ptr(std::uint16_t w){return w&0x8000u?0xffff0000u|w:w;}
    static std::int32_t signed_word(std::uint16_t w){return w&0x8000u?int(w)-65536:int(w);}
    std::uint32_t code_long(std::uint32_t a){return (std::uint32_t(m.opcode_word(a))<<16)|m.opcode_word(a+2);}
    void sound(std::uint32_t n,std::uint32_t pan=0x1000,std::uint32_t extra=0){emit(n,pan,extra);}
    void init(std::uint32_t actor,unsigned index,std::uint32_t table){
        const auto doubled=std::uint16_t((index&255u)*2u);
        const auto frame=table+signed_word(m.read_word(table+signed_word(doubled)));
        m.write_long(actor+0x1c,frame);m.write_long(actor+0x18,m.read_long(frame));
    }
    void step(std::uint32_t actor){
        const auto left=std::uint16_t(m.read_word(actor+0x18)-1u);m.write_word(actor+0x18,left);
        if(left)return;
        auto frame=m.read_long(actor+0x1c)+16u;if(m.read_byte(actor+0x1a)&0x80)frame=m.read_long(frame);
        m.write_long(actor+0x1c,frame);m.write_long(actor+0x18,m.read_long(frame));
    }
    // Original display-list head/count reset,2D890..2D8E7.
    void reset_display(){
        for(unsigned i=0;i<6;++i){m.write_word(globals+0x5810+i*2,std::uint16_t(globals+0x58ceu+i*0x40));m.write_byte(globals+0x5820+i,0);}
        m.write_word(globals+0x5a0e,std::uint16_t(globals+0x5a14));m.write_word(globals+0x5a10,0);m.write_word(globals+0x5a12,0);
    }
    void display(std::uint32_t actor){
        m.write_byte(actor+1,1);
        const auto mode=m.read_byte(actor+8);
        if(mode==0||mode==4||mode==12)return;
        if(mode!=8)throw std::runtime_error("Unrecovered original display mode");
        if(m.read_byte(globals+0x125)&&!m.read_byte(actor+0x3a))return;
        const auto layer=m.read_byte(actor+9);
        if(layer>10||(layer&1))throw std::runtime_error("Invalid original display layer");
        const auto head=globals+0x5810+layer;
        const auto stack=ptr(m.read_word(head))-2u;
        m.write_word(stack,std::uint16_t(actor));m.write_word(head,std::uint16_t(stack));
        const auto count=globals+0x5820+layer/2;
        m.write_byte(count,std::uint8_t(m.read_byte(count)+1u));
    }
    void free(std::uint32_t actor){
        for(unsigned offset:{0u,4u,8u,12u,0x36u})m.write_long(actor+offset,0);
        m.write_byte(actor+0x3a,0);m.write_byte(actor+8,8);
        const auto stack=ptr(m.read_word(globals+0x580a))-2u;
        m.write_word(stack,std::uint16_t(actor));m.write_word(globals+0x580a,std::uint16_t(stack));
        m.write_byte(globals+0x581c,std::uint8_t(m.read_byte(globals+0x581c)+1u));
    }
    void palette_start(std::uint32_t actor,std::uint32_t frame){
        m.write_long(actor+0x1c,frame);m.write_long(actor+0x18,m.read_long(frame));
        auto blocks=m.read_long(frame+4);
        for(;;){
            auto destination=0x90c000u+signed_word(m.read_word(blocks));
            const auto table_index=m.read_word(blocks+2),descriptor=m.read_word(blocks+4);blocks+=6;
            auto source=code_long(0x2d3a6u+signed_word(table_index))+(descriptor&0xffe0u);
            for(unsigned i=0;i<16;++i,destination+=2,source+=2)m.write_word(destination,std::uint16_t((m.read_word(destination)&0xf000u)|(m.read_word(source)&0xfffu)));
            if(descriptor&1)return;
        }
    }
    void palette_step(std::uint32_t actor){
        const auto left=std::uint16_t(m.read_word(actor+0x18)-1u);m.write_word(actor+0x18,left);if(left)return;
        auto frame=m.read_long(actor+0x1c)+8;if(m.read_byte(actor+0x1a)&0x80)frame=m.read_long(frame);
        palette_start(actor,frame);
    }
    void large_finish(std::uint32_t actor){
        switch(m.read_byte(actor+4)){
            case 0:
                m.write_byte(actor+4,2);
                if(!m.read_byte(globals+0x125)){m.write_byte(actor+4,4);return;}
                // Original2BBC8 three queue commands, including control payloads.
                sound(0xffffff07,0,0);sound(0x0100ff08,0,0);sound(0xf0,0x71010,0x80);
                {const auto selector=m.read_word(globals+0xffff9332u);
                const auto frame=0x25b5eu+signed_word(m.opcode_word(0x25b5eu+signed_word(selector)));palette_start(actor,frame);}return;
            case 2:if(!m.read_byte(globals+0x125)){m.write_byte(actor+4,4);return;}palette_step(actor);return;
            case 4:free(actor);return;
            default:throw std::runtime_error("Invalid original large-finish state");
        }
    }
};
} // namespace puzzle_fighter
