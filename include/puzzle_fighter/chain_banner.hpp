// SPDX-License-Identifier: GPL-3.0-only
// USA C89E..C8B7 and C948 tile clear. Candidate pending composed validation.
#pragma once
#include "clear_banner.hpp"
namespace puzzle_fighter {
template<class Memory>
void chain_banner(std::uint32_t player,Memory&m) {
    m.write_byte(player+0x38eu,m.read_byte(player+0x19cu));
    m.write_byte(player+0x38du,0);m.write_byte(player+0x381u,0x3c);
    clear_banner(player,m);
}
} // namespace puzzle_fighter
