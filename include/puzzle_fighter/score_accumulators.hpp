// SPDX-License-Identifier: GPL-3.0-only
// USA 960620: 00FEA8..00FF3B. Native gameplay-state contracts.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
// Memory supplies byte/word reads and writes, including external original ROM.
// These functions preserve gameplay access order. CPU scratch registers/CCR
// are outside their API. External table words are never embedded here.
template<class Memory>
std::uint16_t accumulate_colour_bonus(std::uint32_t player,Memory& m) {
    m.write_byte(player+0x393u,0);
    auto mask=m.read_byte(player+0x394u);
    for(unsigned bit=0;bit<4;++bit) {
        if(mask&1u)m.write_byte(player+0x393u,
            static_cast<std::uint8_t>(m.read_byte(player+0x393u)+1u));
        mask>>=1;
    }
    const auto colours=m.read_byte(player+0x393u);
    // Original DBRA executes colours times after the preliminary decrement.
    const auto bonus=static_cast<std::uint16_t>(colours>1?200u*colours:0u);
    m.write_word(player+0x1feu,bonus);
    return bonus;
}
template<class Memory>
std::uint16_t accumulate_chain_bonus(std::uint32_t player,Memory& m) {
    const auto chain=m.read_byte(player+0x19cu);
    const auto record=m.read_byte(player+0x32eu);
    const auto signed_byte=[](std::uint8_t x){return x<128?int(x):int(x)-256;};
    if(signed_byte(chain)>signed_byte(record))m.write_byte(player+0x32eu,chain);
    std::uint16_t bonus=0;
    // Table indexing uses the zero-extended chain byte, unlike the record test.
    if(chain>1) {
        const auto index=chain>11?10u:unsigned(chain)-1u;
        bonus=m.read_word(0x100c8u+2u*index);
    }
    m.write_word(player+0x1fcu,bonus);
    return bonus;
}
template<class Memory>
std::uint16_t accumulate_count_bonus(std::uint32_t player,Memory& m) {
    const auto count=m.read_word(player+0x200u);
    const auto reduced=static_cast<std::uint16_t>(count-2u);
    std::uint16_t bonus=0;
    // Original BPL tests N after WORD subtraction, including wrapped inputs.
    // Repeated ADDI.W has no intervening memory effects; multiplication retains
    // exactly its low-word wrap, followed by the original negative-result clamp.
    if(reduced && reduced<0x8000u) {
        bonus=static_cast<std::uint16_t>(10u*reduced);
        if(bonus&0x8000u)bonus=0;
    }
    m.write_word(player+0x200u,bonus);
    return bonus;
}
} // namespace puzzle_fighter
