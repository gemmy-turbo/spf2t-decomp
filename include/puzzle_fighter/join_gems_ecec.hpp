// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 ECEC..EEAF: preserve its literal scan and vertical inner loops.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct GemJoinEcecResult{std::uint32_t candidates=0,merges=0;};
namespace gem_join_ecec_detail {
constexpr std::uint32_t move_counter(std::uint32_t address,std::int32_t step,std::uint16_t counter){do{address+=static_cast<std::uint32_t>(step);counter=static_cast<std::uint16_t>(counter-1u);}while(counter!=0xffffu);return address;}
template<class Memory>
void commit(std::uint32_t player,std::uint32_t anchor,std::uint16_t extent,Memory&memory){
    memory.write_byte(player+0x32cu,static_cast<std::uint8_t>(memory.read_byte(player+0x32cu)+1u));
    const auto serial=memory.read_byte(player+0x32cu);memory.write_byte(player+0x32cu,serial);
    if(!serial)memory.write_byte(player+0x32cu,1);
    memory.write_word(player+0x34au,extent);
    auto columns=static_cast<std::uint16_t>(memory.read_word(player+0x34cu)-1u);
    auto column=anchor;
    do{auto cell=column;auto rows=memory.read_word(player+0x34au);do{
        memory.write_word(cell,static_cast<std::uint16_t>(memory.read_word(cell)&255u));
        memory.write_word(cell+0x400u,0);
        const auto id=memory.read_byte(player+0x32cu);memory.write_byte(cell+0x400u,id);
        cell-=16u;rows=static_cast<std::uint16_t>(rows-1u);
    }while(rows!=0xffffu);column+=2u;columns=static_cast<std::uint16_t>(columns-1u);}while(columns!=0xffffu);
    (void)memory.read_word(player+0x34au);(void)memory.read_word(player+0x34cu);
    memory.write_word(player+0x34cu,static_cast<std::uint16_t>(memory.read_word(player+0x34cu)-2u));
    memory.write_byte(anchor,0x30);memory.write_byte(anchor+0x401u,1);
    auto corner=move_counter(anchor,2,memory.read_word(player+0x34cu));
    memory.write_byte(corner,0x60);memory.write_byte(corner+0x401u,4);
    corner=move_counter(anchor,-16,memory.read_word(player+0x34au));
    memory.write_byte(corner,0x90);memory.write_byte(corner+0x401u,2);
    corner=move_counter(anchor,2,memory.read_word(player+0x34cu));
    corner=move_counter(corner,-16,memory.read_word(player+0x34au));
    memory.write_byte(corner,0xc0);memory.write_byte(corner+0x401u,8);
}
template<class Memory>
bool candidate(std::uint32_t player,std::uint32_t anchor,Memory&memory){
    const auto id=memory.read_byte(anchor+0x400u);memory.write_byte(player+0x38au,id);
    memory.write_byte(player+0x38bu,0);memory.write_byte(player+0x38fu,0);
    const auto colour=memory.read_word(anchor)&7u;
    auto top=anchor;std::uint16_t extent=0;
    do{top-=16u;extent=static_cast<std::uint16_t>(extent+1u);}while(memory.read_byte(top+0x401u)!=2);
    memory.write_word(player+0x34au,0);memory.write_word(player+0x34cu,0);
    auto column=anchor;std::uint8_t corner_sum=0;
    for(;;){auto remaining=extent;auto cell=column;do{
        const auto current_colour=memory.read_word(cell)&7u;
        bool accepted=current_colour!=7&&memory.read_byte(cell+0x400u)&&(memory.read_byte(cell+1u)&0x80u)&&current_colour==colour;
        bool check_sum=false;
        if(accepted){const auto current_id=memory.read_byte(cell+0x400u);
            if(current_id!=memory.read_byte(player+0x38au)){
                if(memory.read_byte(player+0x38fu)){
                    if(current_id!=memory.read_byte(player+0x3a5u))check_sum=true;
                }else if(memory.read_byte(cell+0x401u)!=1)accepted=false;
                else{memory.write_byte(player+0x38fu,1);const auto second=memory.read_byte(cell+0x400u);memory.write_byte(player+0x3a5u,second);}
            }
        }
        if(check_sum||!accepted){
            if(!check_sum&&remaining!=extent)return false;
            if(corner_sum!=45&&corner_sum!=30)return false;
            commit(player,anchor,extent,memory);return true;
        }
        corner_sum=static_cast<std::uint8_t>(corner_sum+memory.read_byte(cell+0x401u));
        memory.write_word(player+0x34au,static_cast<std::uint16_t>(memory.read_word(player+0x34au)+1u));
        memory.write_byte(player+0x38bu,static_cast<std::uint8_t>(memory.read_byte(player+0x38bu)+1u));
        cell-=16u;remaining=static_cast<std::uint16_t>(remaining-1u);
    }while(remaining!=0xffffu);
        column+=2u;memory.write_word(player+0x34au,0);
        memory.write_word(player+0x34cu,static_cast<std::uint16_t>(memory.read_word(player+0x34cu)+1u));
        memory.write_byte(player+0x38bu,0);
    }
}
}
// Six origins separated by -16, each followed by thirteen +2 steps. Preserve
// overlaps/extra addresses; the original is not normalized to a board grid.
// Malformed metadata has unbounded walks; oracle staging bounds only its tests.
template<class Memory>
GemJoinEcecResult join_gems_ecec(std::uint32_t player,Memory&memory){
    const auto high=memory.read_word(player+0x164u);const auto low=memory.read_word(player+0x166u);
    const auto field=(std::uint32_t(high)<<16)|low;GemJoinEcecResult result;
    auto origin=field+0xc2u;
    for(unsigned x=0;x<6;++x,origin-=16u){auto cell=origin;for(unsigned y=0;y<13;++y,cell+=2u){
        if(memory.read_byte(cell+0x401u)!=1)continue;
        ++result.candidates;result.merges+=gem_join_ecec_detail::candidate(player,cell,memory);
    }}return result;
}
}
