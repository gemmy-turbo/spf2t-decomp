// SPDX-License-Identifier: GPL-3.0-only
// USA DAFA..DCA9. Original instruction-derived candidate; validation pending.
#pragma once
#include <cstdint>
#include <array>
namespace puzzle_fighter {
// The input group is the original low D3 byte. The special shape branches
// deliberately reread current metadata instead of reusing this argument.
template<class Memory>
std::uint8_t shape_code(std::uint32_t cell,std::uint8_t group,Memory&m) {
    std::uint8_t adjacent=0;
    constexpr std::array<std::uint32_t,4> offsets{0xfffffff0u,2u,0x10u,0xfffffffeu};
    for(unsigned direction=0;direction<4;++direction) {
        const auto neighbor=cell+offsets[direction];
        const auto kind=m.read_word(neighbor)&0x1fu;
        if((kind&0x10u) || kind==7 || kind==6)continue;
        if(m.read_byte(neighbor+0x400u)==group)adjacent|=static_cast<std::uint8_t>(0x10u<<direction);
    }
    auto code=adjacent;
    if(adjacent==0xb0u || adjacent==0xe0u) {
        const auto current_group=m.read_byte(cell+0x400u);
        const auto match=adjacent==0xb0u?0x30u:0x60u;
        bool left=false;
        if((m.read_byte(cell-8u)&0xf0u)==match)left=true;
        else if((m.read_byte(cell-6u)&0xf0u)==match)left=true;
        else if((m.read_byte(cell-4u)&0xf0u)==match) {
            if(m.read_byte(cell+0x404u)!=current_group)left=true;
            else if(m.read_byte(cell+0x406u)==current_group)code=adjacent==0xb0u?0x40u:0x10u;
        }else if(m.read_byte(cell+0x404u)==current_group)code=adjacent==0xb0u?0x40u:0x10u;
        if(left)code=adjacent==0xb0u?0x50u:0x20u;
    }
    m.write_byte(cell,code);
    return code;
}
} // namespace puzzle_fighter
