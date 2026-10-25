// SPDX-License-Identifier: GPL-3.0-only
// USA CCC6..CEC1 reachable body; original validation pending.
#pragma once
#include "first_attack_banner.hpp"
#include "attack_warning.hpp"
#include "counter_gem_pattern.hpp"
#include <cstdint>
namespace puzzle_fighter {
struct CounterDropResult {
    bool nonempty{},first_attack{},full_row{};
    unsigned patterns{},placed{},occupied{},partial_steps{};
    AttackWarningResult warning{};
};
namespace counter_drop_detail {
constexpr std::int32_t signed_word(std::uint16_t v){return v<0x8000u?std::int32_t(v):std::int32_t(v)-0x10000;}
constexpr std::int32_t signed_byte(std::uint8_t v){return v<0x80u?std::int32_t(v):std::int32_t(v)-0x100;}
}
template<class Memory>
CounterDropResult counter_drop(std::uint32_t player,std::uint32_t globals,Memory&m) {
    using counter_drop_detail::signed_word;
    CounterDropResult result;
    auto pointer=[&](std::uint32_t a){const auto hi=m.read_word(a);return (std::uint32_t(hi)<<16)|m.read_word(a+2u);};
    auto opponent=pointer(player+0x160u);
    if(!m.read_byte(opponent+0x19eu))return result;
    result.nonempty=true;
    if(!m.read_byte(player+0x391u)&&!m.read_byte(globals+0xd8u)) {
        const auto score=pointer(opponent+0xaau)+1000u;
        // Original ADDI.L writes the low word before the high word.
        m.write_word(opponent+0xacu,static_cast<std::uint16_t>(score));
        m.write_word(opponent+0xaau,static_cast<std::uint16_t>(score>>16));
        m.write_byte(globals+0x691u,1);m.write_byte(globals+0xa91u,1);
        first_attack_banner(player,m);result.first_attack=true;
    }
    result.warning=attack_warning(player,globals,m);
    opponent=pointer(player+0x160u);
    m.write_byte(player+0x322u,1);m.write_byte(player+0x318u,1);
    // These list reads execute even though FF3C replaces their loaded values.
    const auto colour_offset=m.read_word(player+0x256u);
    (void)m.read_word(opponent+0x280u+static_cast<std::uint32_t>(signed_word(colour_offset)));
    const auto count_offset=m.read_word(player+0x256u);
    (void)m.read_word(opponent+0x2c0u+static_cast<std::uint32_t>(signed_word(count_offset)));
    m.write_byte(player+0x261u,static_cast<std::uint8_t>(m.read_byte(player+0x261u)+1u));
    if(counter_drop_detail::signed_byte(m.read_byte(player+0x261u))>=3)m.write_byte(player+0x261u,0);
    m.write_word(player+0x308u,0);
    auto pattern=[&](){++result.patterns;return counter_gem_pattern(player,m,&opponent);};
    (void)pattern();
    // CD54 branches over unreachable CD58..CDAE. Do not recreate its timeout path.
    for(;;) {
        auto cell=pointer(player+0x164u)-14u;
        if(!m.read_byte(player+0x37au)&&m.read_byte(opponent+0x19eu)>5) {
            for(unsigned column=0;column<6;++column) {
                m.write_word(player+0x308u,static_cast<std::uint16_t>(column*2u));
                const auto gem=pattern();m.write_word(cell+column*2u,gem);++result.placed;
            }
            m.write_byte(opponent+0x19eu,static_cast<std::uint8_t>(m.read_byte(opponent+0x19eu)-6u));
            m.write_word(player+0x306u,static_cast<std::uint16_t>(m.read_word(player+0x306u)+1u));
            m.write_byte(player+0x37du,static_cast<std::uint8_t>(m.read_byte(player+0x37du)+6u));
            result.full_row=true;
            break; // CEA8 skips the partial-row cleanup.
        }
        const auto order=std::uint16_t(m.read_byte(player+0x37bu))*8u;
        const auto progress=m.read_byte(player+0x37au);
        const auto index=static_cast<std::uint16_t>((order&0xff00u)|((order+progress)&0xffu));
        // MOVE.B changes only D0's low byte; the shifted index high byte survives.
        const auto offset=static_cast<std::uint16_t>((index&0xff00u)|m.read_byte(0xff98u+static_cast<std::uint32_t>(signed_word(index))));
        cell+=static_cast<std::uint32_t>(signed_word(offset));
        if(!m.read_byte(cell)) {
            m.write_word(player+0x308u,offset);
            const auto gem=pattern();m.write_word(cell,gem);++result.placed;
        } else ++result.occupied;
        ++result.partial_steps;
        m.write_byte(player+0x37du,static_cast<std::uint8_t>(m.read_byte(player+0x37du)+1u));
        m.write_byte(opponent+0x19eu,static_cast<std::uint8_t>(m.read_byte(opponent+0x19eu)-1u));
        m.write_byte(player+0x37au,static_cast<std::uint8_t>(m.read_byte(player+0x37au)+1u));
        if(m.read_byte(player+0x37au)==6) {
            m.write_word(player+0x306u,static_cast<std::uint16_t>(m.read_word(player+0x306u)+1u));
            m.write_byte(player+0x37au,0);
        } else if(m.read_byte(opponent+0x19eu))continue;
        if(m.read_byte(player+0x37au)==6) {
            m.write_word(player+0x306u,static_cast<std::uint16_t>(m.read_word(player+0x306u)+1u));
            m.write_byte(player+0x37au,0);m.write_byte(player+0x1beu,0);
        } else {
            m.write_byte(player+0x1beu,0);m.write_word(player+0x20au,0);
        }
        break;
    }
    if(!m.read_byte(opponent+0x19eu))m.write_byte(player+0x27fu,0);
    return result;
}
} // namespace puzzle_fighter
