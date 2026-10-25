// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 123BE..12423. Instruction-derived candidate; see validation report.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
// Retains the original unbounded word index and ordinary DATA-ROM table view.
// The caller supplies 24-bit memory access; no CPU execution is performed here.
template<class Memory>
bool turn_picture(std::uint32_t player,Memory&m) {
    if(!m.read_word(player+0x24cu))return false;
    m.write_word(player+0x24cu,static_cast<std::uint16_t>(m.read_word(player+0x24cu)-1u));
    const bool left=m.read_byte(player+0x247u)!=0;
    const auto frames=m.read_word(player+0x24cu);
    auto orientation=std::uint16_t(m.read_byte(player+0x170u));
    if(left&&m.read_byte(player+0x247u)==2)orientation=4;
    const auto index=static_cast<std::uint16_t>(frames*2u+orientation*32u);
    const std::uint32_t displacement=(index&0x8000u)?0xffff0000u|index:index;
    const auto table=(left?0x12d00u:0x12da0u)+displacement;
    m.write_word(player+0x248u,m.read_word(table));
    m.write_word(player+0x24au,m.read_word(table+0x10u));
    return true;
}
} // namespace puzzle_fighter
