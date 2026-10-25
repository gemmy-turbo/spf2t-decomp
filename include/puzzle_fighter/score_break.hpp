// SPDX-License-Identifier: GPL-3.0-only
// USA FAB0..FCC5 active ScoreBreak paths and original gameplay dependencies.
// Original instruction-derived composition; validation pending.
#pragma once
#include <initializer_list>
#include "remove_and_tally.hpp"
#include "score_accumulators.hpp"
#include "attack_amount.hpp"
#include "cancellation_scale.hpp"
#include "chain_adjustment.hpp"

namespace puzzle_fighter {
struct ScoreBreakResult {
    RemoveAndTallyResult removal{};
    std::uint16_t score_increment{};
    bool score_written{},attack_applied{},cancellation_applied{},scaled_cancellation{},chain_called{};
    AttackAmountResult attack{};
    ChainAdjustmentResult chain{};
};
template<class Memory,class Tables>
ScoreBreakResult score_break(std::uint32_t player,std::uint32_t globals,Memory&m,Tables&tables) {
    ScoreBreakResult result;
    result.removal=remove_and_tally(player,m);
    accumulate_count_bonus(player,m);
    if(result.removal.removed) {
        accumulate_chain_bonus(player,m);
        accumulate_colour_bonus(player,m);
        std::uint16_t bonuses=0,tallies=0;
        for(const auto offset:{0x1fcu,0x1feu,0x200u,0x202u})
            bonuses=static_cast<std::uint16_t>(bonuses+m.read_word(player+offset));
        for(const auto offset:{0x204u,0x206u,0x208u})
            tallies=static_cast<std::uint16_t>(tallies+m.read_word(player+offset));
        result.score_increment=static_cast<std::uint16_t>(tallies+bonuses);
        const auto high=m.read_word(player+0xaau);
        const auto score=((std::uint32_t(high)<<16)|m.read_word(player+0xacu))+result.score_increment;
        // Original ADD.L memory writeback uses low word before high word.
        m.write_word(player+0xacu,static_cast<std::uint16_t>(score));
        m.write_word(player+0xaau,static_cast<std::uint16_t>(score>>16));
        result.score_written=true;
        if(result.score_increment) {
            result.attack_applied=true;
            result.attack=attack_amount(player,globals,result.score_increment,m);
            m.write_word(player+0x200u,0);m.write_word(player+0x202u,0);m.write_word(player+0x1feu,0);
            m.write_byte(player+0x393u,0);m.write_byte(player+0x394u,0);m.write_byte(player+0x3a4u,0);
            const auto elapsed=m.read_word(player+0x340u);
            if(!(elapsed&0x8000u) && elapsed>=180u) {
                m.write_byte(player+0x1eeu,0);
            }else{
                result.cancellation_applied=true;
                const auto other_high=m.read_word(player+0x160u);
                const auto other=(std::uint32_t(other_high)<<16)|m.read_word(player+0x162u);
                const auto own=m.read_byte(player+0x19eu);
                m.write_byte(player+0x358u,own);
                const auto opponent=m.read_byte(other+0x19eu);
                if(opponent>=own) {
                    result.scaled_cancellation=true;
                    auto remaining=static_cast<std::uint16_t>(opponent-cancellation_scale(own));
                    if(remaining&0x8000u)remaining=0;
                    m.write_byte(other+0x19eu,static_cast<std::uint8_t>(remaining));
                    m.write_byte(other+0x1eeu,0);m.write_byte(player+0x1eeu,0);m.write_byte(player+0x19eu,0);
                }else{
                    m.write_byte(player+0x19eu,static_cast<std::uint8_t>(own-opponent));
                    m.write_byte(player+0x1eeu,0);m.write_byte(other+0x1eeu,0);m.write_byte(other+0x19eu,0);
                }
            }
            // FC70 branches over FC74..FCAF, including the display call.
            m.write_byte(player+0x349u,1);
        }
    }
    const auto chain=m.read_byte(player+0x19cu);
    if(chain) {
        result.chain_called=true;
        result.chain=chain_adjustment(player,globals,chain,m,tables);
    }
    return result;
}
} // namespace puzzle_fighter
