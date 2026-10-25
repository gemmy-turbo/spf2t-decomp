// SPDX-License-Identifier: GPL-3.0-only
// USA FF3C..FF97. Original execution validated; see COUNTER_GEM_PATTERN_STATUS.md.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
namespace counter_pattern_detail {
constexpr std::int32_t signed_word(std::uint16_t value) {
    return value<0x8000u?std::int32_t(value):std::int32_t(value)-0x10000;
}
}
// Return the original low D2 word. ROM lookups use the ordinary program view.
// Negative row/column values retain original word shifts and signed ADDA.W.
template<class Memory>
std::uint16_t counter_gem_pattern(std::uint32_t player,Memory&m,std::uint32_t* loaded_opponent=nullptr) {
    using counter_pattern_detail::signed_word;
    const auto opponent_high=m.read_word(player+0x160u);
    const auto opponent=(std::uint32_t(opponent_high)<<16)|m.read_word(player+0x162u);
    // The caller retains this original A2 reload, including upper pointer tags.
    if(loaded_opponent)*loaded_opponent=opponent;
    auto row=m.read_word(player+0x306u);
    if(signed_word(row)>=12)row=11;
    const auto row_offset=static_cast<std::uint16_t>(row<<4);
    auto pattern=0x10188u+static_cast<std::uint32_t>(signed_word(row_offset));
    const auto character=m.read_byte(opponent+0x82u);
    pattern+=std::uint32_t(character)*0xc0u;
    const auto column=m.read_word(player+0x308u);
    pattern+=static_cast<std::uint32_t>(signed_word(column));
    const auto colour=m.read_word(pattern);
    const auto colour_offset=static_cast<std::uint16_t>(colour*2u);
    auto gem=m.read_word(0x10178u+static_cast<std::uint32_t>(signed_word(colour_offset)));
    if(m.read_byte(player+0x27fu))return static_cast<std::uint16_t>(gem+0x307u);
    const auto current_character=m.read_byte(opponent+0x82u);
    const auto countdown=m.read_word(0x10160u+std::uint32_t(current_character)*2u);
    return static_cast<std::uint16_t>(gem+countdown);
}
} // namespace puzzle_fighter
