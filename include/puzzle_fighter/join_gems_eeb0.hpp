// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 EEB0..F05F: exact second join pass, not generic rectangle merging.
// Memory owns the original field/+400 metadata/player storage and 24-bit bus.
#pragma once
#include <cstdint>

namespace puzzle_fighter {
struct GemJoinEeb0Result {std::uint32_t candidates=0,merges=0;};
namespace gem_join_eeb0_detail {
constexpr std::uint32_t move_counter(std::uint32_t address,std::int32_t step,std::uint16_t counter) {
    do {
        address+=static_cast<std::uint32_t>(step);
        counter=static_cast<std::uint16_t>(counter-1u);
    } while(counter!=0xffffu);
    return address;
}

template<class Memory>
void commit(std::uint32_t player,std::uint32_t anchor,std::uint16_t extent,Memory &memory) {
    memory.write_byte(player+0x32cu,static_cast<std::uint8_t>(memory.read_byte(player+0x32cu)+1u));
    const auto serial=static_cast<std::uint8_t>(memory.read_byte(player+0x32cu)&255u);
    memory.write_byte(player+0x32cu,serial); // Original identity ANDI.B still writes.
    if(!serial)memory.write_byte(player+0x32cu,1);
    memory.write_word(player+0x34au,extent);
    auto rows=static_cast<std::uint16_t>(memory.read_word(player+0x34cu)-1u);
    if(rows&0x8000u)rows=0; // Original SUBQ.W/BPL tests the wrapped N bit.
    auto row=anchor;
    do {
        auto cell=row;
        auto columns=memory.read_word(player+0x34au);
        do {
            memory.write_word(cell,static_cast<std::uint16_t>(memory.read_word(cell)&255u));
            memory.write_word(cell+0x400u,0);
            const auto id=memory.read_byte(player+0x32cu);
            memory.write_byte(cell+0x400u,id);
            cell-=2u;columns=static_cast<std::uint16_t>(columns-1u);
        } while(columns!=0xffffu);
        row-=16u;rows=static_cast<std::uint16_t>(rows-1u);
    } while(rows!=0xffffu);
    // EFE2/EFE6 read both scratch words before the RMW at EFEA, even though
    // those register values do not drive the following gameplay operations.
    (void)memory.read_word(player+0x34au);
    (void)memory.read_word(player+0x34cu);
    memory.write_word(player+0x34cu,static_cast<std::uint16_t>(memory.read_word(player+0x34cu)-2u));
    auto corner=anchor;
    memory.write_byte(corner,0x30);memory.write_byte(corner+0x401u,4);
    corner=move_counter(corner,-16,memory.read_word(player+0x34cu));
    memory.write_byte(corner,0x60);memory.write_byte(corner+0x401u,8);
    const auto width=memory.read_word(player+0x34au);
    corner=move_counter(anchor,-2,width);
    memory.write_byte(corner,0x90);memory.write_byte(corner+0x401u,1);
    corner=move_counter(anchor,-16,memory.read_word(player+0x34cu));
    corner=move_counter(corner,-2,memory.read_word(player+0x34au));
    memory.write_byte(corner,0xc0);memory.write_byte(corner+0x401u,2);
}

template<class Memory>
bool candidate(std::uint32_t player,std::uint32_t anchor,Memory &memory) {
    const auto initial_id=memory.read_byte(anchor+0x400u);
    memory.write_byte(player+0x38au,initial_id);
    memory.write_byte(player+0x38bu,0);memory.write_byte(player+0x38fu,0);
    const auto colour=memory.read_word(anchor)&7u;
    auto left=anchor;std::uint16_t extent=0;
    do {left-=2u;extent=static_cast<std::uint16_t>(extent+1u);} while(memory.read_byte(left+0x401u)!=1);
    memory.write_word(player+0x34au,0);memory.write_word(player+0x34cu,0);
    auto row=anchor;std::uint8_t corner_sum=0;
    for(;;) {
        auto remaining=extent;auto cell=row;
        do {
            const auto current_colour=memory.read_word(cell)&7u;
            bool accepted=false;
            if(current_colour!=7 && (memory.read_byte(cell+1u)&0x80u) && current_colour==colour) {
                const auto current_id=memory.read_byte(cell+0x400u);
                accepted=true;
                if(current_id!=memory.read_byte(player+0x38au)) {
                    if(!memory.read_byte(player+0x38bu) && !memory.read_byte(player+0x38fu)) {
                        if(memory.read_byte(cell+0x401u)!=1)accepted=false;
                        else memory.write_byte(player+0x38fu,1);
                    }
                }
            }
            if(!accepted) {
                if(remaining!=extent || corner_sum!=30)return false;
                commit(player,anchor,extent,memory);return true;
            }
            corner_sum=static_cast<std::uint8_t>(corner_sum+memory.read_byte(cell+0x401u));
            memory.write_word(player+0x34au,static_cast<std::uint16_t>(memory.read_word(player+0x34au)+1u));
            memory.write_byte(player+0x38bu,static_cast<std::uint8_t>(memory.read_byte(player+0x38bu)+1u));
            cell-=2u;remaining=static_cast<std::uint16_t>(remaining-1u);
        } while(remaining!=0xffffu);
        row-=16u;
        memory.write_word(player+0x34au,0);
        memory.write_word(player+0x34cu,static_cast<std::uint16_t>(memory.read_word(player+0x34cu)+1u));
        memory.write_byte(player+0x38bu,0);
    }
}
} // namespace gem_join_eeb0_detail

// Preserve live RAM accesses and failed-candidate writes. The original has no
// scan bounds for malformed metadata; bounded oracle readers define their own
// finite test domain instead of clipping this native path.
template<class Memory>
GemJoinEeb0Result join_gems_eeb0(std::uint32_t player,Memory &memory) {
    const auto high=memory.read_word(player+0x164u);
    const auto low=memory.read_word(player+0x166u);
    const auto field=(std::uint32_t(high)<<16)|low;
    GemJoinEeb0Result result;
    auto column=field+0xccu;
    for(unsigned x=0;x<6;++x,column-=2u) {
        auto cell=column;
        for(unsigned y=0;y<13;++y,cell-=16u) {
            if(memory.read_byte(cell+0x401u)!=4)continue;
            ++result.candidates;
            result.merges+=gem_join_eeb0_detail::candidate(player,cell,memory);
        }
    }
    return result;
}
} // namespace puzzle_fighter
