// SPDX-License-Identifier: GPL-3.0-only
// Ordered memory adapter for original USA 11A1A..11BBD and player RNG 122EC.
// Pure gameplay formulas are separately validated in pair_generation.hpp.
#pragma once
#include "pair_generation.hpp"
#include <cstdint>
#include <initializer_list>
namespace puzzle_fighter {
struct PairGenerationMemoryResult {
    PairGenerationBranches branches{};
    std::uint16_t final_seed{};
};
template<class Memory>
PairGenerationMemoryResult make_next_pair_memory(std::uint32_t player,std::uint32_t globals,Memory&m) {
    PairGenerationMemoryResult result;
    const auto draw=[&](){
        const auto next=player_random(m.read_word(player+0x144u)).seed;
        m.write_word(player+0x144u,next);result.final_seed=next;return next;
    };
    result.branches.early=pair_detail::signed_word(m.read_word(player+0x1c8u))<=1;
    if(result.branches.early) {
        const auto first=draw();const auto order=m.read_byte(player+0x380u);
        const auto index=static_cast<std::uint16_t>((first&31u)*2u+std::uint16_t(order)*64u);
        m.write_word(player+0x156u,m.read_word(pair_detail::indexed(0x12578u,index)));
        const auto second=draw();
        m.write_word(player+0x158u,m.read_word(0x12778u+(second&31u)*2u));
        auto first_colour=m.read_word(player+0x156u),second_colour=m.read_word(player+0x158u);
        if(pair_detail::signed_word(m.read_word(player+0x156u))>7)first_colour=static_cast<std::uint16_t>(first_colour-8u);
        if(pair_detail::signed_word(m.read_word(player+0x158u))>7)second_colour=static_cast<std::uint16_t>(second_colour-8u);
        if(first_colour==second_colour) {
            result.branches.same_colour_suppressed=true;
            m.write_word(player+0x156u,first_colour);m.write_word(player+0x158u,first_colour);
        }
    } else {
        const auto first=draw();const auto first_order=m.read_byte(player+0x380u);
        const auto first_index=static_cast<std::uint16_t>((first&63u)*2u+std::uint16_t(first_order)*128u);
        m.write_word(player+0x156u,m.read_word(pair_detail::indexed(0x127b8u,first_index)));
        if(m.read_byte(globals+0x83u)&&pair_detail::signed_word(m.read_word(player+0x156u))<7) {
            result.branches.demonstration_crash=true;
            m.write_word(player+0x156u,static_cast<std::uint16_t>(m.read_word(player+0x156u)+8u));
        }
        const auto second=draw();const auto second_order=m.read_byte(player+0x380u);
        const auto second_index=static_cast<std::uint16_t>((second&31u)*2u+std::uint16_t(second_order)*128u);
        m.write_word(player+0x158u,m.read_word(pair_detail::indexed(0x127b8u,second_index)));
    }
    m.write_word(player+0x1f4u,static_cast<std::uint16_t>(m.read_word(player+0x1f4u)+1u));
    const auto count=m.read_word(player+0x1f4u);
    if(count==m.read_word(player+0x1f6u)) {
        result.branches.diamond=true;m.write_word(player+0x158u,5);
        m.write_word(player+0x1f8u,static_cast<std::uint16_t>(m.read_word(player+0x1f8u)+1u));
        const auto cursor=m.read_word(player+0x1f8u);
        m.write_word(player+0x1f6u,m.read_word(pair_detail::indexed(0x124b8u,static_cast<std::uint16_t>(cursor*2u))));
    }
    for(const auto offset:{0x156u,0x158u}) {
        const auto colour=m.read_word(player+offset)&7u;
        const auto address=player+0x350u+colour;
        m.write_byte(address,static_cast<std::uint8_t>(m.read_byte(address)+1u));
    }
    for(unsigned i=0;i<4;++i) {
        if(pair_detail::signed_byte(m.read_byte(player+0x350u+i))>12) {
            m.write_byte(player+0x350u+i,0);m.write_word(player+0x156u,static_cast<std::uint16_t>(9u+i));
            result.branches.forced_crash_mask|=static_cast<std::uint8_t>(1u<<i);
        }
    }
    const auto second=m.read_word(player+0x158u);
    if(second==m.read_word(player+0x156u)&&pair_detail::signed_word(m.read_word(player+0x156u))>8) {
        result.branches.duplicate_crash=true;
        m.write_word(player+0x156u,static_cast<std::uint16_t>(m.read_word(player+0x156u)-8u));
    }
    return result;
}
template<class Memory>
PairGenerationMemoryResult advance_pair_memory(std::uint32_t player,std::uint32_t globals,Memory&m) {
    m.write_word(player+0x152u,m.read_word(player+0x156u));
    m.write_word(player+0x154u,m.read_word(player+0x158u));
    return make_next_pair_memory(player,globals,m);
}
} // namespace puzzle_fighter
