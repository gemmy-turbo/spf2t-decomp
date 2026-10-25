// SPDX-License-Identifier: GPL-3.0-only
// USA DCCA..DCD5; original wrapper validation pending.
#pragma once
#include "form_gems_e8bc.hpp"
namespace puzzle_fighter {
template<class Memory>
FormGemsE8bcResult form_gems_dcca(std::uint32_t player,Memory&memory) {
    if(memory.read_byte(player+0x31au))return {};
    return form_gems_e8bc(player,memory);
}
} // namespace puzzle_fighter
