// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 gameplay-state recovery of 0x011A26 and its local helpers.
// This models state changes, not CPU registers, flags, timing or stack writes.
#pragma once
#include "player_rng.hpp"
#include <array>
#include <cstdint>

namespace puzzle_fighter {
struct PairGenerationState {
    std::uint16_t seed = 0;
    std::uint16_t speed_stage = 0; // +1C8: not the generated-pair count
    std::uint8_t order = 0;
    std::array<std::uint16_t, 2> current{};
    std::array<std::uint16_t, 2> next{};
    std::uint16_t count = 0, diamond_at = 0, diamond_cursor = 0;
    // Raw +350..357 window: generation indexes all eight bytes. Later bytes
    // are also used by AI/loss animation; they are not eight exclusive counters.
    std::array<std::uint8_t, 8> colours{};
};
struct PairGenerationBranches {
    bool early = false, same_colour_suppressed = false;
    bool demonstration_crash = false, diamond = false, duplicate_crash = false;
    std::uint8_t forced_crash_mask = 0;
};
struct PairGenerationResult {
    PairGenerationState state;
    PairGenerationBranches branches;
};

namespace pair_detail {
constexpr std::int32_t signed_word(std::uint16_t value) {
    return value < 0x8000u ? value : static_cast<std::int32_t>(value) - 0x10000;
}
constexpr int signed_byte(std::uint8_t value) {
    return value < 0x80u ? value : static_cast<int>(value) - 0x100;
}
// Original indexed addressing sign-extends the low word, including wrapped
// diamond schedule indices. The caller supplies the original data-space word.
constexpr std::uint32_t indexed(std::uint32_t base, std::uint16_t offset) {
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(base) + signed_word(offset));
}
} // namespace pair_detail

// read_word(address) must provide big-endian words from the original DATA view.
// Tables remain external assets; no proprietary arrays are embedded here.
template<class ReadWord>
PairGenerationResult make_next_pair(PairGenerationState state,
                                    std::uint8_t showing_how, ReadWord read_word) {
    using pair_detail::signed_word;
    PairGenerationBranches branches;
    const auto draw = [&state]() {
        state.seed = player_random(state.seed).seed;
        return state.seed;
    };
    branches.early = signed_word(state.speed_stage) <= 1;
    if (branches.early) {
        const auto first_index = static_cast<std::uint16_t>(state.order * 64u + 2u * (draw() & 31u));
        state.next[0] = read_word(pair_detail::indexed(0x12578, first_index));
        const auto second_index = static_cast<std::uint16_t>(2u * (draw() & 31u));
        state.next[1] = read_word(pair_detail::indexed(0x12778, second_index));
        auto first_colour = state.next[0];
        auto second_colour = state.next[1];
        if (signed_word(first_colour) > 7) first_colour = static_cast<std::uint16_t>(first_colour - 8u);
        if (signed_word(second_colour) > 7) second_colour = static_cast<std::uint16_t>(second_colour - 8u);
        if (first_colour == second_colour) {
            branches.same_colour_suppressed = true;
            state.next = {first_colour, first_colour};
        }
    } else {
        const auto first_index = static_cast<std::uint16_t>(state.order * 128u + 2u * (draw() & 63u));
        state.next[0] = read_word(pair_detail::indexed(0x127B8, first_index));
        if (showing_how && signed_word(state.next[0]) < 7) {
            state.next[0] = static_cast<std::uint16_t>(state.next[0] + 8u);
            branches.demonstration_crash = true;
        }
        const auto second_index = static_cast<std::uint16_t>(state.order * 128u + 2u * (draw() & 31u));
        state.next[1] = read_word(pair_detail::indexed(0x127B8, second_index));
    }

    state.count = static_cast<std::uint16_t>(state.count + 1u);
    if (state.count == state.diamond_at) {
        branches.diamond = true;
        state.next[1] = 5;
        state.diamond_cursor = static_cast<std::uint16_t>(state.diamond_cursor + 1u);
        const auto offset = static_cast<std::uint16_t>(2u * state.diamond_cursor);
        state.diamond_at = read_word(pair_detail::indexed(0x124B8, offset));
    }
    // The original counts indices 1..4 for ordinary colours, then checks
    // indices 0..3. Preserve that offset quirk and signed byte comparisons.
    for (const auto gem : state.next) {
        auto &counter = state.colours[gem & 7u];
        counter = static_cast<std::uint8_t>(counter + 1u);
    }
    for (unsigned index = 0; index < 4; ++index) {
        if (pair_detail::signed_byte(state.colours[index]) > 12) {
            state.colours[index] = 0;
            state.next[0] = static_cast<std::uint16_t>(9u + index);
            branches.forced_crash_mask |= static_cast<std::uint8_t>(1u << index);
        }
    }
    if (state.next[0] == state.next[1] && signed_word(state.next[0]) > 8) {
        branches.duplicate_crash = true;
        state.next[0] = static_cast<std::uint16_t>(state.next[0] - 8u);
    }
    return {state, branches};
}

// Original entry at 0x011A1A promotes the queued pair before falling through
// to 0x011A26. Keep that transition explicit for a persistent native stream.
template<class ReadWord>
PairGenerationResult advance_pair(PairGenerationState state,
                                 std::uint8_t showing_how, ReadWord read_word) {
    state.current = state.next;
    return make_next_pair(state, showing_how, read_word);
}
} // namespace puzzle_fighter
