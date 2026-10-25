// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 FD64..FEA7, active path FDA4. Original data tables stay external.
// Gameplay memory/effect contract; no CPU, register, flag or timing model.
#pragma once
#include <cstdint>

namespace puzzle_fighter {
struct AttackAmountResult {
    std::uint16_t amount;
    bool pending_capped;
};
namespace attack_amount_detail {
constexpr std::int32_t signed_word(std::uint16_t value) {
    return value<0x8000u?value:std::int32_t(value)-0x10000;
}
constexpr std::uint32_t indexed(std::uint32_t base,std::uint16_t offset) {
    return base+static_cast<std::uint32_t>(signed_word(offset));
}
// Original SUBI.W / BMI loops test the wrapped result's N bit. In particular,
// a negative input near 8000 can wrap positive on the first subtraction.
constexpr std::uint16_t subtract_until_negative(std::uint16_t value,unsigned step) {
    std::uint16_t count=0;
    for(;;) {
        value=static_cast<std::uint16_t>(value-step);
        if(value&0x8000u)return count;
        count=static_cast<std::uint16_t>(count+1u);
    }
}
constexpr std::uint16_t arithmetic_half(std::uint16_t value) {
    return static_cast<std::uint16_t>((value>>1)|(value&0x8000u));
}
} // namespace attack_amount_detail

// Memory supplies ordered byte/word accesses to the original big-endian DATA
// view, and masks addresses only at its 24-bit bus boundary. There is no native
// table-walk cap or sanitized array index; test readers may bound their domain.
template<class Memory>
AttackAmountResult attack_amount(std::uint32_t player,std::uint32_t globals,
                                std::uint16_t input_amount,Memory &memory) {
    using namespace attack_amount_detail;
    auto elapsed=static_cast<std::uint16_t>(memory.read_word(player+0x340u)+15u);
    std::uint32_t table_offset=0,address=0x10110u;
    for(;;) {
        const auto threshold=memory.read_word(address);
        if(signed_word(elapsed)<=signed_word(threshold))break;
        table_offset+=2u;address+=2u;
    }
    auto multiplier=memory.read_word(indexed(0x10130u,static_cast<std::uint16_t>(table_offset)));
    auto quotient=subtract_until_negative(input_amount,10);
    std::uint16_t extra=0;
    // FDE0 tests zero before each addition; unlike the later DBRA loops, this
    // performs exactly the unsigned word multiplier's number of additions.
    while(multiplier) {
        extra=static_cast<std::uint16_t>(extra+quotient);
        multiplier=static_cast<std::uint16_t>(multiplier-1u);
    }
    auto amount=static_cast<std::uint16_t>(input_amount+extra);
    const auto setting=memory.read_byte(globals+0xa1u)&7u;
    auto scale=memory.read_word(0x10150u+2u*setting);
    quotient=subtract_until_negative(amount,10);
    amount=0;
    scale=static_cast<std::uint16_t>(scale+9u);
    do {
        amount=static_cast<std::uint16_t>(amount+quotient);
        scale=static_cast<std::uint16_t>(scale-1u);
    } while(scale!=0xffffu);
    amount=subtract_until_negative(amount,100);
    if(memory.read_byte(player+0x1fau))amount=arithmetic_half(amount);
    if(!memory.read_byte(player+0x9eu)) {
        bool reduced=memory.read_byte(player+0x82u)==8;
        if(!reduced)reduced=memory.read_byte(player+0x82u)==9;
        if(reduced) {
            const auto quarter=arithmetic_half(arithmetic_half(amount));
            amount=static_cast<std::uint16_t>(arithmetic_half(amount)+quarter);
        }
    }
    std::uint16_t own=2,opponent=2;
    if(memory.read_byte(globals+0xacu)==3) {
        own=memory.read_byte(player+0xa9u);
        const auto high=memory.read_word(player+0x160u);
        const auto low=memory.read_word(player+0x162u);
        const auto other=(std::uint32_t(high)<<16)|low;
        opponent=memory.read_byte(other+0xa9u);
    }
    auto difference=static_cast<std::uint16_t>(opponent-own);
    if(difference && !(difference&0x8000u)) {
        quotient=subtract_until_negative(amount,10);
        extra=0;
        do {
            extra=static_cast<std::uint16_t>(extra+quotient);
            difference=static_cast<std::uint16_t>(difference-1u);
        } while(difference!=0xffffu);
        amount=static_cast<std::uint16_t>(amount+extra);
    }
    const auto pending=memory.read_byte(player+0x19eu);
    const auto comparison=static_cast<std::uint16_t>((amount&0xff00u)|static_cast<std::uint8_t>(amount+pending));
    const bool capped=signed_word(comparison)>250;
    if(capped) {
        memory.write_byte(player+0x1eeu,0);
        memory.write_byte(player+0x19eu,250);
    } else {
        // Original ADD.B re-reads its destination after the comparison.
        const auto previous=memory.read_byte(player+0x19eu);
        memory.write_byte(player+0x19eu,static_cast<std::uint8_t>(previous+amount));
    }
    return {amount,capped};
}
} // namespace puzzle_fighter
