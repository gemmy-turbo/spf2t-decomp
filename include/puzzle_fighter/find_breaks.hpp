// SPDX-License-Identifier: GPL-3.0-only
// USA 00C28E..00C2AF, tail-calling 00D462. Composite validation pending.
#pragma once
#include "start_breaks.hpp"
#include "diamond_breaks.hpp"
#include "mark_breaks.hpp"
namespace puzzle_fighter {
struct FindBreaksResult {
    StartBreaksResult started;
    DiamondBreaksResult diamond;
    MarkBreaksResult marked;
};
// One original turn-state transition, not a complete turn or frame scheduler.
// The shared memory adapter retains writes between all three recovered routines.
template<class Memory,class Effects>
FindBreaksResult find_breaks(std::uint32_t player,std::uint32_t globals,
                            Memory&m,Effects&effects) {
    m.write_byte(player+0x17eu,static_cast<std::uint8_t>(m.read_byte(player+0x17eu)+1u));
    FindBreaksResult result;
    result.started=start_breaks(player,m,effects);
    result.diamond=diamond_breaks(player,m);
    if(m.read_byte(player+0x395u))m.write_byte(player+0x395u,0);
    result.marked=mark_breaks(player,globals,m,effects);
    return result;
}
} // namespace puzzle_fighter
