// SPDX-License-Identifier: GPL-3.0-only
// USA 960620 gameplay-state recovery of 0x01182A..0x011915.
// Original data tables remain external. No CPU, ABI, flags or timing model.
#pragma once
#include <cstdint>

namespace puzzle_fighter {
struct FallSpeedState {
    std::uint8_t ten_phase = 0;          // player +1A1
    std::uint16_t pair_stage = 0;        // +1C8, early-generation phase
    std::uint16_t twenty_phase = 0;      // +184
    std::uint16_t speed_stage = 0;       // +186
    std::uint8_t handicap = 0;           // +A9
    std::uint8_t computer = 0;           // +9E
    std::uint16_t elapsed = 0;           // +340, signed comparisons
    std::uint8_t mode = 0;               // A5+AC; handicap mode is 3
    std::uint8_t setting = 0;            // A5+DF
    std::uint16_t slide_step = 0;        // +27C
    std::uint16_t movement_limit = 0;    // +388
    std::uint8_t delay = 0;              // +18A
    std::uint8_t fall_timer = 0;         // +15E
    std::uint16_t generated = 0;         // +1F4; preserved by this routine
};
struct FallSpeedBranches {
    bool ten_rollover = false, pair_stage_clamped = false;
    bool twenty_rollover = false, speed_stage_clamped = false;
    bool handicap_base = false, handicap_limit = false;
    bool after_120 = false, after_240 = false;
    bool default_level = false, short_period = false, late_handicap = false;
    std::uint32_t slide_address = 0, base_delay_address = 0, adjustment_address = 0;
};
struct FallSpeedResult {
    FallSpeedState state;
    FallSpeedBranches branches;
};
namespace fall_speed_detail {
constexpr std::int32_t signed_word(std::uint16_t value) {
    return value < 0x8000u ? value : static_cast<std::int32_t>(value) - 0x10000;
}
constexpr std::uint32_t indexed(std::uint32_t base, std::uint16_t offset) {
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(base) + signed_word(offset));
}
} // namespace fall_speed_detail

// tables.read_word(address) and tables.read_byte(address) supply the original
// big-endian DATA view, including adjacent data reached by abnormal indices.
template<class TableReader>
FallSpeedResult set_fall_speed(FallSpeedState state, TableReader &tables) {
    using fall_speed_detail::signed_word;
    FallSpeedBranches branches;
    state.ten_phase = static_cast<std::uint8_t>(state.ten_phase + 1u);
    if (state.ten_phase == 10) {
        branches.ten_rollover = true;
        state.ten_phase = 0;
        state.pair_stage = static_cast<std::uint16_t>(state.pair_stage + 1u);
        if (signed_word(state.pair_stage) > 99) {
            branches.pair_stage_clamped = true;
            state.pair_stage = 99;
        }
    }
    state.twenty_phase = static_cast<std::uint16_t>(state.twenty_phase + 1u);
    if (state.twenty_phase == 20) {
        branches.twenty_rollover = true;
        state.twenty_phase = 0;
        state.speed_stage = static_cast<std::uint16_t>(state.speed_stage + 1u);
        if (signed_word(state.speed_stage) >= 16) {
            branches.speed_stage_clamped = true;
            state.speed_stage = 15;
        }
    }

    const auto slide_offset = static_cast<std::uint16_t>(2u * state.speed_stage);
    branches.slide_address = fall_speed_detail::indexed(0x12BD8, slide_offset);
    state.slide_step = tables.read_word(branches.slide_address);

    branches.handicap_base = state.mode == 3 && state.computer == 0;
    branches.default_level = !branches.handicap_base;
    const auto base_level = branches.handicap_base ? state.handicap : std::uint8_t{2};
    // 0118A6 is ADD.B: a carry out of the low byte cannot change the stage's
    // high byte before the signed word index at 0118AC is used.
    const auto low_index = static_cast<std::uint8_t>(state.speed_stage + (base_level << 4));
    const auto base_offset = static_cast<std::uint16_t>((state.speed_stage & 0xff00u) | low_index);
    branches.base_delay_address = fall_speed_detail::indexed(0x12C20, base_offset);
    auto base_delay = static_cast<std::uint8_t>(tables.read_byte(branches.base_delay_address));

    branches.handicap_limit = state.mode == 3 && state.handicap == 4;
    state.movement_limit = branches.handicap_limit ? 0xfffe : 0xfff0;
    std::uint8_t adjustment_level;
    branches.after_120 = signed_word(state.elapsed) > 120;
    branches.short_period = !branches.after_120;
    if (branches.after_120) {
        adjustment_level = 3;
        branches.after_240 = signed_word(state.elapsed) > 240;
        if (branches.after_240) {
            adjustment_level = 4;
            state.movement_limit = 0xfffe;
            base_delay = 0;
        }
    } else {
        // Unlike the base-level selection, this has no computer-flag test.
        branches.late_handicap = state.mode == 3;
        adjustment_level = state.mode == 3 ? state.handicap : std::uint8_t{2};
    }
    const auto adjustment_offset = static_cast<std::uint8_t>((adjustment_level << 3) + state.setting);
    branches.adjustment_address = 0x12BF8u + adjustment_offset;
    state.delay = static_cast<std::uint8_t>(base_delay + tables.read_byte(branches.adjustment_address));
    state.fall_timer = state.delay;
    return {state, branches};
}
} // namespace puzzle_fighter
