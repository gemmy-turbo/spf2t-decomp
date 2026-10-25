// SPDX-License-Identifier: GPL-3.0-only
// USA2D834/2D83E/2D992..2DAAC scene clear/retain boundaries.
#pragma once
#include "task_pool.hpp"
#include "attack_display.hpp"
#include "character_controller.hpp"
#include "actor_tasks.hpp"
namespace puzzle_fighter {
template<class M> void clear_round_records(std::uint32_t g,M&m){
    for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400,a=g+0xb00+side*0x100;
        for(unsigned o:{4u,0xd0u,0xd4u})m.write_long(p+o,0);
        for(unsigned o=0x100;o<0x400;o+=4)m.write_long(p+o,0);
        for(unsigned o:{4u,0xcu,0x36u,0xd0u,0xd4u})m.write_long(a+o,0);
        m.write_byte(a+0xd8,0);
    }
}
template<class M> void clear_background_records(std::uint32_t g,M&m){m.write_byte(g+0xa9,0xff);for(unsigned o=0x200;o<0x300;o+=4)m.write_long(g+o,0);}
template<class M> void clear_scene_records(std::uint32_t g,M&m,bool fresh){
    if(fresh){
        for(unsigned side=0;side<2;++side){for(unsigned o=0;o<0x400;o+=4)m.write_long(g+0x300+side*0x400+o,0);for(unsigned o=0;o<0x100;o+=4)m.write_long(g+0xb00+side*0x100+o,0);}
        clear_background_records(g,m);for(unsigned o=0x100;o<0x180;o+=4)m.write_long(g+o,0);
        for(unsigned o=0x2900;o<0x2b00;o+=4)m.write_long(g+o,0);
        //2DA9A uses the advanced A6; preserve its actual negative displacement.
        m.write_byte(g+0x2b00-0x56f8,0x14);for(unsigned o:{0x8cu,0x8au,0x8bu})m.write_byte(g+o,0);
    }else{clear_background_records(g,m);for(unsigned o=0x2b00;o<0x3300;o+=4)m.write_long(g+o,0);}
    for(unsigned o=0x3300;o<0x4780;o+=4)m.write_long(g+o,0);
    clear_round_records(g,m);initialize_task_pool(g,m);initialize_attack_display_pool(g,m);initialize_character_auxiliary_pool(g,m);
    auto quiet=[](std::uint32_t,std::uint32_t,std::uint32_t){};ActorTasks<M,decltype(quiet)>{m,quiet,g}.reset_display();
}
// A262..A35A scalar state, before original BDAC/BD38/picture/player setup.
template<class M> void start_match_state(std::uint32_t g,M&m){
    m.write_word(g+8,m.read_word(g+8)+2);clear_scene_records(g,m,false);
    m.write_word(g+0x20,0x90c0);m.write_word(g+0x32,m.read_word(g+0x32)|14);
    for(unsigned o:{0xcu,0x10u,0x14u}){m.write_long(g+o,0);}
    m.write_word(g+0x102,0);
    for(unsigned o:{0x12cu,0x105u,0x107u,0x10du,0x14du,0xb8u,0x8au,0x10cu,0x10bu,0x10fu,0x118u,0x12au,0x125u,0x126u,0x124u,0x149u,0x127u,0x11cu,0x11du,0x11eu,0x151u,0x108u,0x109u,0x152u,0x156u,0x133u,0x301u,0x701u,0xb01u,0xc01u})m.write_byte(g+o,0);
    m.write_byte(g+0x10a,0x3b);m.write_byte(g+0xbd,1);m.write_byte(g+0x8f,1);m.write_byte(g+0x121,m.read_byte(g+0xa2));m.write_byte(g+0xb9,m.read_byte(g+0xac));m.write_byte(g+0x11f,m.read_byte(g+0x98));
    if(m.read_byte(g+0xa5)==2&&m.read_byte(g+0xe0))m.write_byte(g+0x11f,0);
    for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400;m.write_byte(p+0xa3,0);m.write_long(p+0xa4,0);m.write_long(p+0xaa,0);m.write_byte(p+0xaf,0);m.write_byte(p+0xb1,0);if(m.read_byte(g+0xac)!=3&&!m.read_byte(p+0x9e))m.write_long(p+0xaa,m.read_long(p+0x90));}
    if(m.read_byte(g+0xac)==3){m.write_byte(g+0x141,0xff);m.write_byte(g+0x121,m.read_byte(g+0xa3));}
}
// Source A6DE..A77A scalar part; calls2B292 and16C98 belong to the caller.
template<class M> void begin_round_state(std::uint32_t g,M&m){
    m.write_word(g+0x10,m.read_word(g+0x10)+2);m.write_byte(g+0x10e,1);m.write_byte(g+0x10f,1);m.write_long(g+0x14,0);
    for(unsigned o:{0x10cu,0x107u,0x12cu,0x125u,0x126u,0xbdu,0x145u,0x10du,0x12au,0x124u,0x149u,0x127u,0x11cu,0x11du,0x11eu,0x151u})m.write_byte(g+o,0);
    m.write_word(g+0x13a,0);m.write_word(g+0x13c,0);m.write_byte(g-0x6cb0,0);m.write_word(g-0x6cb2,180);
    m.write_word(g+0x32,m.read_word(g+0x32)|14);m.write_word(g+0x20,0x90c0);m.write_word(g+0x48,0x90c0);
    for(unsigned o:{0x301u,0x701u,0xb01u,0xc01u,0x133u,0x10du})m.write_byte(g+o,1);
}
} // namespace puzzle_fighter
