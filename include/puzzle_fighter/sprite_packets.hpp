// SPDX-License-Identifier: GPL-3.0-only
// USA1495A object-list composition. Coordinates and attributes remain source values.
#pragma once
#include "gem_picture_builder.hpp"
#include <stdexcept>
#include <vector>
namespace puzzle_fighter {
using SpritePacket=GemPictureObject;
enum class SpritePacketGroup {body,attachment,shadow,first_pool,board,pair,third_pool,raw,overlay,service};
struct SpritePacketOrigin {SpritePacketGroup group=SpritePacketGroup::body;std::uint32_t actor=0,parent=0;};
struct SpritePacketFrame {
    std::vector<SpritePacket> packets;
    std::vector<SpritePacketOrigin> origins; // parallel read-only host metadata
    bool updated=false;
    std::uint8_t bank=0;
    std::uint16_t remaining_budget=0;
};
namespace sprite_packet_detail {
inline int signed_word(std::uint16_t v){return v&0x8000?int(v)-65536:int(v);}
inline std::uint32_t pointer(std::uint16_t v){return std::uint32_t(signed_word(v));}
template<class M> struct Composer {
    M&m;std::uint32_t g;std::vector<SpritePacket>&out;std::uint16_t budget=0x380;
    std::vector<SpritePacketOrigin>*origins=nullptr;SpritePacketOrigin origin{};
    void push(SpritePacket p){out.push_back(p);if(origins)origins->push_back(origin);}
    std::uint16_t word(std::uint32_t&p){const auto v=m.read_word(p);p+=2;return v;}
    std::uint32_t longword(std::uint32_t&p){const auto v=m.read_long(p);p+=4;return v;}
    std::uint16_t palette(std::uint32_t a,std::uint16_t attr,std::uint16_t mask){
        const auto mode=m.read_byte(a+0xc);if(!mode)return attr;
        unsigned value=m.read_byte(a+0xd);if(mode&0x80)value+=attr;
        return (attr&mask)|(value&31);
    }
    void packet(std::uint32_t a,int x,int y,std::uint16_t tile,std::uint16_t attr,bool clip){
        x=signed_word(std::uint16_t(x+m.read_word(a+0x10)+0x40));
        y=signed_word(std::uint16_t(y+0x100-m.read_word(a+0x14)));
        if(clip){if(x<-576||x>448)x=448;if(y<-784||y>240)y=240;}
        push({std::uint16_t((x&0x3ff)|m.read_word(a+0x30)),std::uint16_t((y&0x3ff)|0x2000),tile,attr});
    }
    // shadow substitutes its ground Y; direct records use +1C as the shape.
    void shape(std::uint32_t a,std::uint32_t s,bool direct=false,bool shadow=false){
        const auto format=word(s);
        if(!direct&&!shadow&&format==10)return;
        if((direct&&format>4)||(shadow&&format>2)||format>8||(format&1))throw std::runtime_error("unsupported original sprite shape format");
        const auto cost=word(s);if(budget<cost)return;budget=std::uint16_t(budget-cost);
        const unsigned count=(!direct&&format==0)?unsigned(cost?cost:65536):unsigned(word(s))+1;
        std::uint16_t common=0;std::uint32_t coords=0;
        const bool uniform=direct?format==0:format==0||format==6;
        auto flip=std::uint8_t(m.read_byte(a+0xb)<<5);
        if(direct)flip=0;else if(shadow)flip&=0x20;else if(format!=8)flip&=0x60;
        if(uniform){common=word(s)|flip;if(!shadow)common=palette(a,common,!direct&&format==0?0x60:0xff60);}
        if(!direct&&(format==0||format==2)){const auto offset=word(s);coords=m.read_long(0x2db84u+std::uint32_t(signed_word(offset)));}
        else if((direct&&format!=4)||(!direct&&(format==6||format==8)))coords=longword(s);
        for(unsigned i=0;i<count;++i){
            std::uint16_t tile=0,attr=common;int x=0,y=0;
            if(format==4){const auto packed=longword(s),xy=longword(s);tile=packed>>16;attr=std::uint16_t(packed)^flip;x=signed_word(xy>>16);y=signed_word(xy);}
            else {x=signed_word(word(coords));y=signed_word(word(coords));if(uniform)tile=word(s);else{const auto packed=longword(s);tile=packed>>16;attr=std::uint16_t(packed)^flip;}}
            if(!direct){const bool reflected=uniform?bool(attr&0x20):bool(flip&0x20);if(reflected)x=-x-((format==0)?16:((attr&0xf00)>>4)+16);}
            if(!uniform&&!shadow)attr=palette(a,attr,format==8?0x60:0xff60);
            if(direct||(!shadow&&format>=4))tile=std::uint16_t(tile+0x5800);
            //15068 uses actor+D6 rather than+14. Keep source actor RAM read-only.
            if(shadow)y+=int(m.read_word(a+0x14))-int(m.read_word(a+0xd6));
            packet(a,x,y,tile,attr,!direct&&!shadow&&(format==6||format==8));
        }
    }
    bool visible(std::uint32_t a){return m.read_byte(a)&&m.read_byte(a+1);}
    void body(std::uint32_t a){origin.actor=a&0xffffff;shape(a,m.read_long(m.read_long(a+0x1c)+4));}
    void attached(std::uint32_t a){
        const auto saved=origin;origin={SpritePacketGroup::attachment,0,a&0xffffff};
        const auto before=m.read_word(a+0x38);if(before&&visible(pointer(before)))body(pointer(before));
        origin=saved;body(a);origin={SpritePacketGroup::attachment,0,a&0xffffff};
        const auto after=m.read_word(a+0x36);if(after&&visible(pointer(after)))body(pointer(after));origin=saved;
    }
    void layer(unsigned n){origin={SpritePacketGroup::first_pool,0,0};const auto count=m.read_byte(g+0x5820+n),base=g+0x58ceu+n*0x40;for(unsigned i=0;i<count;++i)attached(pointer(m.read_word(base-2-2*i)));}
    void shadows(){
        if(m.read_byte(g+0x125))return;
        for(auto a:{g+0xc00,g+0xb00}){if(!visible(a))continue;const auto index=m.read_byte(m.read_long(a+0x1c)+0xe);if(!index)continue;
            const auto animation=0x817b8u+std::uint32_t(signed_word(m.read_word(0x817b8+index*2)));origin={SpritePacketGroup::shadow,a,0};shape(a,m.read_long(animation+4),false,true);
        }
    }
    void direct(std::uint32_t a){origin.actor=a&0xffffff;shape(a,m.read_long(a+0x1c),true);}
    void reset(){
        for(unsigned i=0;i<6;++i){m.write_word(g+0x5810+i*2,std::uint16_t(g+0x58ceu+i*0x40));m.write_byte(g+0x5820+i,0);}
        m.write_word(g+0x5a0e,std::uint16_t(g+0x5a14));m.write_word(g+0x5a10,0);m.write_word(g+0x5a12,0);
        for(auto offset:{0x566u,0x48eu,0x966u,0x88eu})m.write_byte(g+offset,0);
    }
    void run(){
        layer(5);layer(4);layer(3);shadows();
        const bool reverse=m.read_byte(g+0x119)!=0;for(unsigned i=0;i<2;++i){const auto a=g+0xb00+((i^unsigned(reverse))*0x100);if(visible(a)){origin={SpritePacketGroup::body,a,0};attached(a);}}
        layer(2);auto sink=[&](SpritePacket p){push(p);};origin={SpritePacketGroup::board,0,0};budget=emit_board_gem_objects(g,m,sink,budget);origin={SpritePacketGroup::pair,0,0};budget=emit_pair_gem_objects(g,m,sink,budget);layer(1);layer(0);
        origin={SpritePacketGroup::third_pool,0,0};for(unsigned i=0;i<4;++i){const auto a=g+0x2100+i*0x200;if(visible(a)&&!m.read_byte(a+9))direct(a);}
        origin={SpritePacketGroup::raw,0,0};const auto cost=m.read_word(g+0x5a12);if(cost&&budget>=cost){budget=std::uint16_t(budget-cost);const unsigned count=m.read_word(g+0x5a10)?m.read_word(g+0x5a10):65536;for(unsigned i=0;i<count;++i){const auto p=g+0x5a14+i*8;push({m.read_word(p),m.read_word(p+2),m.read_word(p+4),m.read_word(p+6)});}}
        origin={SpritePacketGroup::overlay,g+0x2900,0};if(visible(g+0x2900))direct(g+0x2900);
        origin={SpritePacketGroup::service,0,0};if(m.read_byte(g+0xce)&&(m.read_byte(g+0xcf)&0x80)&&(m.read_byte(g+0x80)&0x20)&&budget>=6){budget-=6;for(unsigned i=0;i<6;++i)push({m.read_word(0x15642+i*4),0x20de,m.read_word(0x15644+i*4),0x14});}
    }
};
}
template<class M> SpritePacketFrame compose_sprite_packets(std::uint32_t g,M&m){
    SpritePacketFrame frame;frame.bank=m.read_byte(g+0x94);sprite_packet_detail::Composer<M> c{m,g,frame.packets};c.origins=&frame.origins;
    if(!m.read_byte(g+0x95)){frame.updated=true;c.run();frame.remaining_budget=c.budget;m.write_word(g+0x11a,std::uint16_t(0x380-c.budget));m.write_byte(g+0x94,frame.bank^1);m.write_byte(g+0x95,0xff);}
    c.reset();return frame;
}
// Optional source hardware adapter: writer(address,word). A held frame writes
// nothing. The terminator is four bytes, not a complete eight-byte packet.
template<class Writer> void write_sprite_packets(const SpritePacketFrame&frame,Writer&&writer){
    if(!frame.updated)return;
    for(unsigned i=0;i<16;++i)writer(0x708000+i*2,std::uint16_t(0));
    std::uint32_t p=0x708020;for(const auto&o:frame.packets){writer(p,o.x);writer(p+2,o.y);writer(p+4,o.tile);writer(p+6,o.attributes);p+=8;}
    writer(p,std::uint16_t(0));writer(p+2,std::uint16_t(0x8000));
}
} // namespace puzzle_fighter
