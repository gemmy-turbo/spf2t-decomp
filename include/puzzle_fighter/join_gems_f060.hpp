// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 F060..F21D: original third join pass and live RAM semantics.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct GemJoinF060Result {std::uint32_t candidates=0,merges=0;};
namespace gem_join_f060_detail {
constexpr std::uint32_t move_counter(std::uint32_t address,std::int32_t step,std::uint16_t counter){do{address+=static_cast<std::uint32_t>(step);counter=static_cast<std::uint16_t>(counter-1u);}while(counter!=0xffffu);return address;}
template<class Memory>
void commit(std::uint32_t player,std::uint32_t anchor,std::uint16_t extent,Memory&memory){
    memory.write_byte(player+0x32cu,static_cast<std::uint8_t>(memory.read_byte(player+0x32cu)+1u));
    const auto serial=memory.read_byte(player+0x32cu);memory.write_byte(player+0x32cu,serial);
    if(!serial)memory.write_byte(player+0x32cu,1);
    memory.write_word(player+0x34au,extent);
    auto rows=static_cast<std::uint16_t>(memory.read_word(player+0x34cu)-1u);
    auto row=anchor;
    do{auto cell=row;auto columns=memory.read_word(player+0x34au);do{
        memory.write_word(cell,static_cast<std::uint16_t>(memory.read_word(cell)&255u));
        memory.write_word(cell+0x400u,0);
        const auto id=memory.read_byte(player+0x32cu);memory.write_byte(cell+0x400u,id);
        cell+=2u;columns=static_cast<std::uint16_t>(columns-1u);
    }while(columns!=0xffffu);row-=16u;rows=static_cast<std::uint16_t>(rows-1u);}while(rows!=0xffffu);
    (void)memory.read_word(player+0x34au);(void)memory.read_word(player+0x34cu);
    memory.write_word(player+0x34cu,static_cast<std::uint16_t>(memory.read_word(player+0x34cu)-2u));
    memory.write_byte(anchor,0x30);memory.write_byte(anchor+0x401u,1);
    auto corner=move_counter(anchor,-16,memory.read_word(player+0x34cu));
    memory.write_byte(corner,0x60);memory.write_byte(corner+0x401u,2);
    auto width=static_cast<std::uint16_t>(memory.read_word(player+0x34au)-1u);
    corner=move_counter(anchor,2,width);memory.write_byte(corner,0x90);memory.write_byte(corner+0x401u,4);
    corner=move_counter(anchor,-16,memory.read_word(player+0x34cu));
    width=static_cast<std::uint16_t>(memory.read_word(player+0x34au)-1u);
    corner=move_counter(corner,2,width);memory.write_byte(corner,0xc0);memory.write_byte(corner+0x401u,8);
}
template<class Memory>
bool candidate(std::uint32_t player,std::uint32_t anchor,Memory&memory){
    const auto id=memory.read_byte(anchor+0x400u);memory.write_byte(player+0x38au,id);
    memory.write_byte(player+0x38bu,0);memory.write_byte(player+0x38fu,0);memory.write_byte(player+0x3a5u,0);
    const auto colour=memory.read_word(anchor)&7u;
    auto right=anchor;std::uint16_t extent=0;
    do{right+=2u;extent=static_cast<std::uint16_t>(extent+1u);}while(memory.read_byte(right+0x401u)!=4);
    memory.write_word(player+0x34au,0);memory.write_word(player+0x34cu,0);
    auto row=anchor;std::uint8_t corner_sum=0;
    for(;;){auto remaining=extent;auto cell=row;do{
        const auto current_colour=memory.read_word(cell)&7u;
        bool accepted=current_colour!=7&&(memory.read_byte(cell+1u)&0x80u)&&current_colour==colour;
        bool check_sum=false;
        if(accepted){const auto current_id=memory.read_byte(cell+0x400u);
            if(current_id!=memory.read_byte(player+0x38au)){
                if(memory.read_byte(player+0x38fu)){
                    // F0FE goes directly to F14E even in the middle of a row.
                    if(current_id!=memory.read_byte(player+0x3a5u))check_sum=true;
                }else if(memory.read_byte(cell+0x401u)!=1)accepted=false;
                else{memory.write_byte(player+0x38fu,1);const auto second=memory.read_byte(cell+0x400u);memory.write_byte(player+0x3a5u,second);}
            }
        }
        if(check_sum||!accepted){
            if(!check_sum&&remaining!=extent)return false;
            if(corner_sum!=30)return false;
            commit(player,anchor,extent,memory);return true;
        }
        corner_sum=static_cast<std::uint8_t>(corner_sum+memory.read_byte(cell+0x401u));
        memory.write_word(player+0x34au,static_cast<std::uint16_t>(memory.read_word(player+0x34au)+1u));
        memory.write_byte(player+0x38bu,static_cast<std::uint8_t>(memory.read_byte(player+0x38bu)+1u));
        cell+=2u;remaining=static_cast<std::uint16_t>(remaining-1u);
    }while(remaining!=0xffffu);
        row-=16u;memory.write_word(player+0x34au,0);memory.write_byte(player+0x38bu,0);
        memory.write_word(player+0x34cu,static_cast<std::uint16_t>(memory.read_word(player+0x34cu)+1u));
    }
}
}
// The original has unbounded metadata search/scan paths. Test readers restrict
// their staged domain; this gameplay routine preserves the original path.
template<class Memory>
GemJoinF060Result join_gems_f060(std::uint32_t player,Memory&memory){
    const auto high=memory.read_word(player+0x164u);const auto low=memory.read_word(player+0x166u);
    const auto field=(std::uint32_t(high)<<16)|low;GemJoinF060Result result;
    auto column=field+0xc2u;
    for(unsigned x=0;x<6;++x,column+=2u){auto cell=column;for(unsigned y=0;y<13;++y,cell-=16u){
        if(memory.read_byte(cell+0x401u)!=1)continue;
        ++result.candidates;result.merges+=gem_join_f060_detail::candidate(player,cell,memory);
    }}return result;
}
}
