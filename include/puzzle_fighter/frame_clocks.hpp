// SPDX-License-Identifier: GPL-3.0-only
// USA 11D6 (IRQ2 increment) and 9FEA..9FF1 (normal main-task prologue).
#pragma once
#include <cstdint>
namespace puzzle_fighter {
// Separate original events. These helpers do not reproduce the IRQ scheduler,
// input copy, phase dispatch, or the conditional IRQ helper at 1F26C.
template<class Memory>
void interrupt_clock_increment(std::uint32_t globals,Memory&m) {
    m.write_byte(globals+0x80u,static_cast<std::uint8_t>(m.read_byte(globals+0x80u)+1u));
}
template<class Memory>
void main_frame_begin(std::uint32_t globals,Memory&m) {
    m.write_byte(globals+0x84u,0xff);
    m.write_byte(globals+0x81u,static_cast<std::uint8_t>(m.read_byte(globals+0x81u)+1u));
}
} // namespace puzzle_fighter
