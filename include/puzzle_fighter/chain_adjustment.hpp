// SPDX-License-Identifier: GPL-3.0-only
// USA 01575E entry, tail-branching to 015794..0157B3; validation pending.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct ChainAdjustmentResult {
    bool applied{},negative_clamp{},upper_clamp{};
    std::uint16_t increment{},value{};
};
// Tables supplies opcode_word(address): this is an original direct PC-indexed
// table lookup, not an ordinary data-space access or a host-selected formula.
template<class Memory,class Tables>
ChainAdjustmentResult chain_adjustment(std::uint32_t player,std::uint32_t globals,
                                      std::uint16_t chain,Memory&m,Tables&tables) {
    ChainAdjustmentResult result;
    if(m.read_byte(globals+0xacu)==3 || m.read_byte(player+0x9eu))return result;
    const auto index=chain>=8?7u:unsigned(chain&0xffu);
    result.applied=true;result.increment=tables.opcode_word(0x15784u+index*2u);
    const auto counter=globals+0xffff8cc6u;
    m.write_word(counter,static_cast<std::uint16_t>(m.read_word(counter)+result.increment));
    result.value=m.read_word(counter);
    if(result.value&0x8000u) {
        (void)m.read_word(counter); // Original CLR.W bus read before writeback.
        m.write_word(counter,0);result.value=0;result.negative_clamp=true;
    }else if(result.value>=0x100u) {
        m.write_word(counter,0xff);result.value=0xff;result.upper_clamp=true;
    }
    return result;
}
} // namespace puzzle_fighter
