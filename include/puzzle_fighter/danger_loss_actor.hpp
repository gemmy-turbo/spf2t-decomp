// SPDX-License-Identifier: GPL-3.0-only
// USA 25AB0..25AFD and allocator 2DAAE..2DAC3.
// Bounded original comparisons: tests/oracle_gameplay_010.cpp.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct DangerLossActorResult {
    bool latched{},allocated{},started{};
    std::uint32_t actor{};
    std::uint16_t free_cursor{};
};
namespace danger_actor_detail {
constexpr std::uint32_t word_pointer(std::uint16_t v){return v<0x8000u?std::uint32_t(v):0xffff0000u|v;}
}
// actor_argument is the original low D1 byte; the end-turn caller supplies FF.
// Tables supplies opcode_byte for the original direct PC-indexed table operand.
template<class Memory,class Tables>
DangerLossActorResult danger_loss_actor(std::uint32_t player,std::uint32_t globals,
                                        std::uint8_t actor_argument,Memory&m,Tables&tables) {
    DangerLossActorResult result;
    if(m.read_byte(globals+0x83u)||m.read_byte(globals+0x126u))return result;
    (void)m.read_byte(globals+0x126u); // Original ST.B reads its destination before writing.
    m.write_byte(globals+0x126u,0xff);result.latched=true;
    if(!m.read_byte(globals+0x581cu))return result;
    m.write_byte(globals+0x581cu,static_cast<std::uint8_t>(m.read_byte(globals+0x581cu)-1u));
    auto cursor=danger_actor_detail::word_pointer(m.read_word(globals+0x580au));
    result.actor=danger_actor_detail::word_pointer(m.read_word(cursor));
    cursor+=2u;result.free_cursor=static_cast<std::uint16_t>(cursor);
    m.write_word(globals+0x580au,result.free_cursor);result.allocated=true;
    // Caller BEQ consumes the final MOVE.W's Z, including pointer-word wrap.
    if(!result.free_cursor)return result;
    m.write_byte(result.actor,static_cast<std::uint8_t>(m.read_byte(result.actor)+1u));
    m.write_byte(result.actor+2u,0x1e);m.write_byte(result.actor+3u,actor_argument);
    const auto source=danger_actor_detail::word_pointer(m.read_word(player+0x34u));
    m.write_byte(source+0xb2u,static_cast<std::uint8_t>(m.read_byte(source+0xb2u)+1u));
    const auto character=m.read_byte(source+0x82u);
    m.write_word(globals+0xffff9332u,tables.opcode_byte(0x25afeu+character));
    m.write_byte(globals+0x125u,0x90);result.started=true;
    return result;
}
} // namespace puzzle_fighter
