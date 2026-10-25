// SPDX-License-Identifier: GPL-3.0-only
// USA C60A..C633. Ordered accesses preserve the decrement and subsequent reread.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct LossStepResult {bool winner_written=false;std::uint8_t row=0;};
template<class Memory>
LossStepResult loss_step(std::uint32_t player,std::uint32_t globals,Memory&m) {
    if(m.read_byte(player+0x354u)) {
        const auto next=static_cast<std::uint8_t>(m.read_byte(player+0x354u)-1u);
        m.write_byte(player+0x354u,next);
        if(!(next&0x80u)) {
            const auto row=m.read_byte(player+0x354u);
            m.write_byte(player+0x360u+row,1);
            return {false,row};
        }
    }
    const auto high=m.read_word(player+0x160u);
    const auto opponent=(std::uint32_t(high)<<16)|m.read_word(player+0x162u);
    const auto winner=m.read_byte(opponent+0x180u);
    m.write_byte(globals+0x10cu,winner);
    return {true,0};
}
} // namespace puzzle_fighter
