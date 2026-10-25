// SPDX-License-Identifier: GPL-3.0-only
// USA C2D8..C31D turn step. Composition candidate; validation pending.
#pragma once
#include <cstdint>
#include "count_field.hpp"
#include "counter_countdown.hpp"
#include "counter_conversion.hpp"
namespace puzzle_fighter {
struct CounterPhaseResult {
    std::uint32_t score_increment{};
    bool counted{},converted{},advanced{};
    CountFieldResult field{};
    CounterCountdownResult countdown{};
    CounterConversionResult conversion{};
};
template<class Memory,class Effects>
CounterPhaseResult counter_phase(std::uint32_t player,std::uint32_t globals,Memory&m,Effects&effects) {
    CounterPhaseResult result;
    const auto bonus_high=m.read_word(player+0x376u);
    result.score_increment=(std::uint32_t(bonus_high)<<16)|m.read_word(player+0x378u);
    const auto score_high=m.read_word(player+0xaau);
    const auto score=((std::uint32_t(score_high)<<16)|m.read_word(player+0xacu))+result.score_increment;
    m.write_word(player+0xacu,static_cast<std::uint16_t>(score));
    m.write_word(player+0xaau,static_cast<std::uint16_t>(score>>16));
    // MOVE.L immediate, unlike ADD.L, writes the high word first.
    m.write_word(player+0x376u,0);m.write_word(player+0x378u,0);
    if(!m.read_byte(player+0x252u)) {
        if(!m.read_byte(player+0x246u)) {
            m.write_byte(player+0x194u,0);result.counted=true;
            result.field=count_field(player,globals,m);
            result.countdown=counter_countdown(player,m,effects);
        }
        result.converted=true;
        result.conversion=counter_conversion(player,m);
    }
    if(!m.read_byte(player+0x246u)) {
        m.write_byte(player+0x17eu,static_cast<std::uint8_t>(m.read_byte(player+0x17eu)+1u));
        m.write_byte(player+0x252u,1);result.advanced=true;
    }
    return result;
}
} // namespace puzzle_fighter
