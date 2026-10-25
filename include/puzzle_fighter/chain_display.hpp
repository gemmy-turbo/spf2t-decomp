// SPDX-License-Identifier: GPL-3.0-only
// USA 014314 and 01446C: gameplay-facing display contract; validation pending.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct ChainDisplayResult {
    std::uint32_t count;
    bool palette_requested=false,voice_requested=false;
};
// Rendering/audio remain explicit effects. The original palette-copy DBRA
// changes the count register to 0000FFFF; preserve that gameplay-facing result.
template<class Memory,class Effects>
ChainDisplayResult chain_display(std::uint32_t player,std::uint32_t globals,
                                std::uint32_t count,Memory&m,Effects&effects) {
    ChainDisplayResult result{count};
    if(m.read_byte(globals+0x107u))return result;
    const auto pointer=m.read_word(player+0x32u);
    const auto object=std::uint32_t(pointer)|(pointer&0x8000u?0xffff0000u:0u);
    if(m.read_byte(object+5u)==10 || m.read_byte(object+0xd9u))return result;
    const auto signed_byte=[](std::uint8_t v){return v<128?int(v):int(v)-256;};
    if(signed_byte(m.read_byte(player+0xd8u))>=2) {
        std::uint8_t level=1;
        for(int threshold=3;threshold<=6;++threshold) {
            if(signed_byte(m.read_byte(player+0xd8u))<threshold)break;
            ++level;
        }
        m.write_byte(object+0xd8u,level);
        m.write_word(object+4u,0x020cu);m.write_word(object+6u,0);
        effects.chain_palette(object);
        result.count=0xffffu;result.palette_requested=true;
    } else {
        m.write_byte(object+0xdbu,static_cast<std::uint8_t>(m.read_byte(object+0xdbu)+1u));
        m.write_byte(object+0xdbu,m.read_byte(object+0xdbu)&7u);
        (void)m.read_byte(object+0xd8u); // Original 68000 CLR memory read.
        m.write_byte(object+0xd8u,0);
        if(m.read_byte(object+0xd1u)==2)return result;
    }
    if(m.read_byte(globals+0xbcu))return result;
    const auto character=m.read_byte(object+0x82u);
    const auto variant=m.read_byte(object+0xdbu);
    const auto level=m.read_byte(object+0xd8u);
    const auto base=static_cast<std::uint16_t>((unsigned(character)<<6)+(unsigned(variant)<<3));
    const auto index=static_cast<std::uint16_t>((base&0xff00u)|static_cast<std::uint8_t>(base+level));
    // The backend must supply the original PC-relative voice-table semantics;
    // this request does not substitute host randomness or an invented sound ID.
    effects.chain_voice(player,index);result.voice_requested=true;
    return result;
}
} // namespace puzzle_fighter
