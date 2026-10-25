// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy (adapted source guide).
// Modified 2026-10-25: USA native memory/callback bindings.
// Adapted from Tommy McLeroy's computer.cpp/difficulty.cpp (2026), pinned
// cps2recomp 145314f81e18d05040fcbe9e87c4df50a57e9a28, with USA static checks.
// Native gameplay functions only: no CPU, instruction loop or fallback.
// Source origin and adaptation details: UPSTREAM.md.
#pragma once
#include "global_rng.hpp"
#include <cstdint>
#include <stdexcept>
#include <initializer_list>
namespace puzzle_fighter::prototype {
template<class Memory, class Effects> class ClassicCpu {
    Memory &m_; Effects &effects_; uint32_t globals_, player_;
    static constexpr uint32_t kFieldBottom=0x164, kBottomRow=0xc0, kDanger=0xd1,
        kPadBefore=0x142,kPad=0x140,kPairRow=0x14,kCharacter=0x82,kFrameCount=0x80,kPlayerNumber=0x81;
    static constexpr int kFieldColumns=6,kFieldRows=14;
    static constexpr uint32_t kRowSize=16;
    static constexpr uint16_t kColour=7,kKind=15,kWall=255,kEmpty=0,kCounter=7,kCrash=8,kCellBreaking=16;
    uint8_t code_byte(uint32_t a) { auto w=m_.opcode_word(a&~1u);return uint8_t(a&1u?w:w>>8); }
    uint8_t draw() {auto r=global_random(m_.read_word(globals_+0xae));m_.write_word(globals_+0xae,r.seed);return r.value;}
    static constexpr uint32_t kRandom = 0x02DB6A;
    static constexpr uint32_t kLevel = 0xA8;            // byte: how well this computer player plays (0 to 31: difficulty.cpp)
    static constexpr uint32_t kCourse = 0xFF0C8C; // byte: the player's choice at the start: 0 EASY, 1 NORMAL, 2 HARD
    static constexpr uint32_t kStage = 0xFF8112;   // word: which opponent of the ladder
    // Its pace: it waits kThinkFrames before it steers a pair. The wait is from a table by its level, the
    // course and how far the round has gone (kPaceStage: a step every kPairsAStep pairs, fifteen at most), with
    // one of sixteen small differences added by chance
    static constexpr uint32_t kThinkFrames = 0x31C;     // byte
    static constexpr uint32_t kPaceStage = 0x31D, kPacePairs = 0x31E; // bytes
    static constexpr uint8_t kPairsAStep = 10, kPaceStageMost = 15;
    static constexpr uint32_t kThinkTable = 0x012E20, kThinkChanges = 0x012498;
    // Slips: two counts of frames drawn by chance for each pair from tables by its level. Once the pair is
    // where it is to go, the computer pushes it down for the first count and then leaves it to fall for the
    // second (ComputerSteers); the second, less 10 or 20 on NORMAL and HARD, is also taken off the wait
    static constexpr uint32_t kBeforeLetGo = 0x37E, kLetGo = 0x37F; // bytes
    static constexpr uint32_t kBeforeLetGoTable = 0x013220, kLetGoTable = 0x013420;
    static constexpr uint32_t kPairsWaiting = 0x1C8;    // word: (more than one, and no slips are drawn)

    // --- steering ---
    // Where the pair is to go is worked out (PlanPair) as counts of presses: so many to
    // the right, to the left, and of each turn button. This plays them on the computer's controls.
    static constexpr uint32_t kPressRight = 0x15A, kPressLeft = 0x15B, kPressTurnA = 0x15C, kPressTurnB = 0x31B; // bytes
    static constexpr uint32_t kStartWait = 0x300, kGap = 0x301;   // bytes: frames before the first press, and to the next
    static constexpr uint32_t kGapTable = 0x012C70;               // bytes: the gap, by course and stage
    // At the first stage the gap grows as the round goes on: kGapStep goes up one in every four presses, to
    // nine, and kGapMore is what it adds (table kGapMoreTable); later stages add the most whenever the
    // computer is in danger
    static constexpr uint32_t kGapPresses = 0x30E, kGapStep = 0x310, kGapMore = 0x312; // words
    static constexpr uint32_t kGapMoreTable = 0x012CA0;
    static constexpr uint16_t kGapStepMost = 9;
    static constexpr uint32_t kPushSound = 0x02BA74;
    static constexpr uint16_t kStickRightBit = 0x0001, kStickLeftBit = 0x0002, kStickDownBit = 0x0004, kStickUpBit = 0x0008, kTurnA = 0x0100, kTurnB = 0x0200;
    static constexpr uint32_t kNoWaiting = 0xFF8083;          // byte

    uint8_t PressesLeft(uint32_t player) {
        return static_cast<uint8_t>(m_.read_byte(player + kPressRight) + m_.read_byte(player + kPressLeft) + m_.read_byte(player + kPressTurnA) +
                                    m_.read_byte(player + kPressTurnB));
    }

    // 012082. The frames to the next press.
    void SetGap() {
        const uint32_t player = player_;
        const uint16_t byCourse = m_.read_byte(kCourse) * 16;
        m_.write_byte(player + kGap, m_.read_byte(kGapTable + byCourse + m_.read_word(kStage)));
        bool step = false;
        if (m_.read_word(kStage) != 0) {
            if (m_.read_byte(player + kDanger)) {
                m_.write_word(player + kGapStep, kGapStepMost);
                m_.write_byte(player + kGap, 1);
                step = true;
            }
        } else {
            m_.write_word(player + kGapPresses, static_cast<uint16_t>(m_.read_word(player + kGapPresses) + 1));
            if (m_.read_word(player + kGapPresses) == 4) {
                m_.write_word(player + kGapStep, static_cast<uint16_t>(m_.read_word(player + kGapStep) + 1));
                if (m_.read_word(player + kGapStep) == kGapStepMost + 1) {
                    m_.write_word(player + kGapStep, kGapStepMost);
                }
                step = true;
            }
        }
        if (step) {
            m_.write_word(player + kGapMore, m_.read_word(kGapMoreTable + static_cast<uint16_t>(m_.read_word(player + kGapStep) * 2 + byCourse)));
            m_.write_word(player + kGapPresses, 0);
        }
        m_.write_byte(player + kGap, static_cast<uint8_t>(m_.read_byte(player + kGap) + m_.read_word(player + kGapMore)));
    }

    // 011F34. A frame of the computer's controls while its pair falls. With presses still to make: one of
    // each kind that is left, every so many frames (not until the pair is well into the field). With none
    // left: after its wait (kThinkFrames) it pushes the pair down, every other frame; but for a number of
    // frames drawn by chance (DrawSlips) it first pushes on (kBeforeLetGo) and then lets the pair fall by
    // itself (kLetGo).
    void ComputerSteers() {
        const uint32_t player = player_;
        m_.write_word(player + kPadBefore, m_.read_word(player + kPadBefore) & kStickDownBit);
        m_.write_word(player + kPad, m_.read_word(player + kPad) & kStickDownBit);
        if (PressesLeft(player) != 0) {
            if (m_.read_byte(player + kStartWait)) {
                m_.write_byte(player + kStartWait, static_cast<uint8_t>(m_.read_byte(player + kStartWait) - 1));
                return;
            }
            m_.write_byte(player + kGap, static_cast<uint8_t>(m_.read_byte(player + kGap) - 1));
            if (m_.read_byte(player + kGap) != 0) {
                m_.write_word(player + kPadBefore, 0);
                m_.write_word(player + kPad, 0);
                return;
            }
            SetGap();
            if (static_cast<int16_t>(m_.read_word(player + kPairRow)) <= 2) {
                return;
            }
            if (m_.read_byte(player + kPressTurnA)) {
                m_.write_word(player + kPad, kTurnA);
                m_.write_byte(player + kPressTurnA, static_cast<uint8_t>(m_.read_byte(player + kPressTurnA) - 1));
            }
            if (m_.read_byte(player + kPressTurnB)) {
                m_.write_word(player + kPad, kTurnB);
                m_.write_byte(player + kPressTurnB, static_cast<uint8_t>(m_.read_byte(player + kPressTurnB) - 1));
            }
            if (m_.read_byte(player + kPressRight)) {
                m_.write_word(player + kPad, m_.read_word(player + kPad) | kStickRightBit);
                m_.write_byte(player + kPressRight, static_cast<uint8_t>(m_.read_byte(player + kPressRight) - 1));
            }
            if (m_.read_byte(player + kPressLeft)) {
                m_.write_word(player + kPad, m_.read_word(player + kPad) | kStickLeftBit);
                m_.write_byte(player + kPressLeft, static_cast<uint8_t>(m_.read_byte(player + kPressLeft) - 1));
            }
            // (the last press made, and no wait: the sound of pushing down)
            if (PressesLeft(player) == 0 && !m_.read_byte(player + kThinkFrames)) {
                effects_.soft_drop_sound(player);
            }
            return;
        }

        if (!m_.read_byte(kNoWaiting)) {
            if (m_.read_byte(kCourse) != 2 && m_.read_word(kStage) == 0) {
                return; // (at the first stage it never pushes down, except on HARD)
            }
            if (m_.read_byte(player + kThinkFrames)) {
                m_.write_byte(player + kThinkFrames, static_cast<uint8_t>(m_.read_byte(player + kThinkFrames) - 1));
                return;
            }
        }
        if (m_.read_byte(player + kBeforeLetGo)) {
            m_.write_byte(player + kBeforeLetGo, static_cast<uint8_t>(m_.read_byte(player + kBeforeLetGo) - 1));
        } else if (m_.read_byte(player + kLetGo)) {
            m_.write_word(player + kPad, 0);
            m_.write_byte(player + kLetGo, static_cast<uint8_t>(m_.read_byte(player + kLetGo) - 1));
            if (m_.read_byte(player + kLetGo) == 0) {
                effects_.soft_drop_sound(player);
            }
            return;
        }
        if ((m_.read_word(globals_ + kFrameCount) & 1) == 0) {
            m_.write_word(player + kPad, kStickDownBit);
        }
    }

    // --- planning ---
    // Before each of its pairs the computer works out the presses that take the pair where it wants it.
    // There are three ways of doing it, and each character has one of them.
    enum class Plan { Seeker, Matcher, Stacker };
    static constexpr Plan kPlans[12] = {Plan::Seeker, Plan::Matcher, Plan::Seeker, Plan::Matcher, Plan::Matcher, Plan::Seeker,
                                 Plan::Stacker, Plan::Matcher, Plan::Seeker, Plan::Seeker, Plan::Matcher, Plan::Matcher};
    static constexpr uint32_t kPairNext = 0x156;          // two words: the pair that comes next (pair.cpp)
    static constexpr uint32_t kFound = 0x265;             // byte: a place has been found
    static constexpr uint32_t kTurns = 0x262;             // byte: presses of the turn button the place needs
    static constexpr uint32_t kPlace = 0x268;             // word: the place, a number into kPresses
    static constexpr uint32_t kScore = 0x210;             // byte: how good the column being looked at is
    static constexpr uint32_t kBestColumn = 0x26C, kBestScore = 0x26D, kFirstColumn = 0x26E, kFirstScore = 0x26F; // bytes
    static constexpr uint32_t kTried = 0x270;             // two words: the pair as it is being tried (which gem under)
    static constexpr uint32_t kPressesRight = 0x263, kPressesLeft = 0x264; // bytes
    // The presses for a place: to the right and to the left (bytes). Places 0 to 5 are the columns, counted
    // from the right (2 is the column pairs come in at); 8 to 15 and 16 to 23 are what is taken by chance
    static constexpr uint32_t kPresses = 0x013C30;
    static constexpr uint32_t kStackerKept = 0x356, kStackerTurn = 0x355; // word, byte
    static constexpr int32_t kBelow = 16;
    static constexpr uint16_t kDiamond = 5;
    // (the time the game's code takes over a look at every column, a search along them, and the rest of a plan)
    static constexpr uint32_t kLookTime = 5000, kFindTime = 3700, kPlanTime = 330;

    uint32_t BottomLeft(uint32_t player) {
        return m_.read_long(player + kFieldBottom) + kBottomRow + 2;
    }
    void SetPresses(uint32_t player, uint8_t right, uint8_t left, uint8_t turns) {
        m_.write_byte(player + kPressRight, right);
        m_.write_byte(player + kPressLeft, left);
        m_.write_byte(player + kPressTurnA, turns);
    }

    // 013AA8. With gems up to the fifth row from the top of the middle, the pair goes to a side that is not
    // as high: right if the third column is, else left if the fifth is.
    void KeepFromTheTop(uint32_t player) {
        const uint32_t top = m_.read_long(player + kFieldBottom);
        if (m_.read_word(top + 0x48) == 0) {
            return;
        }
        if (m_.read_word(top + 0x36) != 0) {
            SetPresses(player, 5, 0, 0);
        } else if (m_.read_word(top + 0x3A) != 0) {
            SetPresses(player, 0, 5, 0);
        }
    }
    // 013AFC. ... and does not go towards a side whose column next to the middle is high
    void KeepFromHighSide(uint32_t player) {
        const uint32_t top = m_.read_long(player + kFieldBottom);
        if (m_.read_byte(player + kPressRight)) {
            if (m_.read_word(top + 0x3A) != 0) {
                m_.write_byte(player + kPressRight, 0);
                m_.write_byte(player + kPressLeft, 3);
            }
        } else if (m_.read_byte(player + kPressLeft) && m_.read_word(top + 0x36) != 0) {
            m_.write_byte(player + kPressRight, 3);
            m_.write_byte(player + kPressLeft, 0);
        }
    }

    // 013742. Scores every column for the pair standing upright in it, `lower` under `upper`: on what the
    // two gems under the first empty cell are. 1: the second one down is of the upper gem's colour (or the
    // column is full); 2: the one just under is of the lower gem's colour; 3 to 6: both, and more by whether
    // either is a crash gem's meeting. A column that is neither keeps the score of the column before it.
    // The best column (the first of the best) is noted.
    void ScoreColumns(uint32_t player, uint16_t upper, uint16_t lower) {
        m_.write_byte(player + kBestColumn, 0);
        m_.write_byte(player + kBestScore, 0);
        m_.write_byte(player + kScore, 0);
        for (int n = 5; n >= 0; n--) {
            const uint32_t bottom = BottomLeft(player) + 2u * static_cast<uint32_t>(5 - n);
            int row = 0;
            while (row < 10 && m_.read_word(bottom - static_cast<uint32_t>(row) * kBelow) != 0) {
                row++;
            }
            if (row == 10) {
                m_.write_byte(player + kScore, 1);
            } else {
                const uint32_t cell = bottom - static_cast<uint32_t>(row) * kBelow;
                const uint16_t under = m_.read_word(cell + kBelow), underThat = m_.read_word(cell + 2 * kBelow);
                const bool far = (underThat & kColour) == (upper & kColour), near = (under & kColour) == (lower & kColour);
                if (far && near) {
                    const bool farBig = (underThat & kKind) + (upper & kKind) > 8, nearBig = (under & kKind) + (lower & kKind) > 8;
                    m_.write_byte(player + kScore, farBig ? (nearBig ? 6 : 5) : (nearBig ? 4 : 3));
                } else if (far) {
                    m_.write_byte(player + kScore, 1);
                } else if (near) {
                    m_.write_byte(player + kScore, 2);
                }
            }
            if (static_cast<int8_t>(m_.read_byte(player + kScore)) > static_cast<int8_t>(m_.read_byte(player + kBestScore))) {
                m_.write_byte(player + kBestScore, m_.read_byte(player + kScore));
                m_.write_byte(player + kBestColumn, static_cast<uint8_t>(n));
                m_.write_byte(player + kFound, 1);
            }
        }
    }

    // 013826. A place by chance
    void PlaceByChance(uint32_t player) {
        m_.write_byte(player + kFound, 1);
        m_.write_byte(player + kTurns, 1);
        const auto random = draw();
        m_.write_word(player + kPlace, static_cast<uint16_t>((random & 7) + 0x10));
    }

    // 01367E. The matcher's first look: the pair upright in each column, either way up, for gems of its
    // colours to land on. What it finds is thrown away for some columns (the entry column and the one at
    // the right-hand end; all the right half when the computer is in danger) and when the danger is great.
    // With nothing found it takes a place by chance, unless the pair has a diamond or pairs are waiting;
    // and one time in sixteen it takes a place by chance whatever it found (at the first stage only, as the
    // game means it: see below).
    void MatcherLooks() {
        const uint32_t player = player_;
        m_.write_byte(player + kFound, 0);
        m_.write_byte(player + 0x26A, 0);
        m_.write_byte(player + kTurns, 0);
        const uint16_t first = m_.read_word(player + kPairNext), second = m_.read_word(player + kPairNext + 2);
        m_.write_word(player + kTried, first);
        m_.write_word(player + kTried + 2, second);
        ScoreColumns(player, first, second);
        m_.write_byte(player + kFirstColumn, m_.read_byte(player + kBestColumn));
        m_.write_byte(player + kFirstScore, m_.read_byte(player + kBestScore));
        m_.write_word(player + kTried, second);
        m_.write_word(player + kTried + 2, first);
        ScoreColumns(player, second, first);
        if (static_cast<int8_t>(m_.read_byte(player + kBestScore)) < static_cast<int8_t>(m_.read_byte(player + kFirstScore))) {
            m_.write_byte(player + kBestColumn, m_.read_byte(player + kFirstColumn));
            m_.write_byte(player + kTurns, 2);
        }
        const uint16_t place = m_.read_byte(player + kBestColumn);
        m_.write_word(player + kPlace, place);
        if (place == 0 || place == 2 || (m_.read_byte(player + kDanger) && place <= 3)) {
            m_.write_byte(player + kFound, 0);
        }
        // (the game asks whether the stage is the first, but gives the stage's place as a distance from A6,
        // the record, where it means the place itself: it looks at a word some way below the records)
        const uint32_t misplaced = (player + static_cast<uint32_t>(static_cast<int16_t>(kStage & 0xFFFF))) & 0xFFFFFF;
        bool byChance = false;
        if (m_.read_word(misplaced) == 0) {
            const auto random = draw();
            byChance = (random & 0xF) == 0;
        }
        if (!byChance) {
            if (m_.read_byte(player + kDanger) == 2) {
                m_.write_byte(player + kFound, 0);
            }
            if (m_.read_byte(player + kFound) || m_.read_word(player + kPairNext) == kDiamond || m_.read_word(player + kPairsWaiting) != 0) {
                return;
            }
        }
        PlaceByChance(player);
    }

    // 013A0A. Looks along the columns, from the left, for the first whose landing cell has a gem of the
    // colour under it, or a crash gem of it under and to one side. The column is noted as the place, and
    // kScore says whether one was found. (The column pairs come in at only counts over a crash gem;
    // otherwise the cells above its landing cell are looked at in the same way.)
    void FindBeside(uint32_t player, uint16_t colour) {
        colour &= kColour;
        m_.write_byte(player + kScore, 0);
        for (int n = 5; n >= 0; n--) {
            const uint32_t bottom = BottomLeft(player) + 2u * static_cast<uint32_t>(5 - n);
            for (int row = 0; row < kFieldRows; row++) {
                const uint32_t cell = bottom - static_cast<uint32_t>(row) * kBelow;
                if (m_.read_word(cell) != 0) {
                    if (row == kFieldRows - 1) {
                        m_.write_byte(player + kScore, 0);
                    }
                    continue;
                }
                bool hit = (m_.read_word(cell + kBelow) & kColour) == colour;
                if (!hit) {
                    const uint16_t right = m_.read_word(cell + kBelow + 2), left = m_.read_word(cell + kBelow - 2);
                    if ((right & kColour) == colour) {
                        hit = (right & kKind) > 7;
                    } else if ((left & kColour) == colour) {
                        hit = (left & kKind) > 7;
                    }
                    if (!hit) {
                        break;
                    }
                }
                if (n == 2 && static_cast<int16_t>(m_.read_word(cell + kBelow)) <= 7) {
                    if (row == kFieldRows - 1) {
                        m_.write_byte(player + kScore, 0);
                    }
                    continue;
                }
                m_.write_byte(player + kScore, 1);
                m_.write_word(player + kPlace, static_cast<uint16_t>(n));
                return;
            }
        }
        m_.write_word(player + kPlace, 0);
    }

    // 013B3C. The matcher's plan. With nothing found by its first look: a column where one of the pair's
    // gems lands beside its colour (the second gem's by turning the pair over), else a place by chance. Then
    // the presses for the place, kept from the top and from a high side.
    void MatcherPlans() {
        const uint32_t player = player_;
        if (!m_.read_byte(player + kFound)) {
            const uint16_t first = m_.read_word(player + kPairNext), second = m_.read_word(player + kPairNext + 2);
            bool found;
            if (static_cast<int16_t>(second) < 7) {
                FindBeside(player, first);
                found = m_.read_byte(player + kScore) != 0;
                if (!found) {
                    FindBeside(player, second);
                    found = m_.read_byte(player + kScore) != 0;
                    if (found) {
                        m_.write_byte(player + kTurns, 2);
                    }
                }
            } else {
                FindBeside(player, second);
                found = m_.read_byte(player + kScore) != 0;
                if (found) {
                    m_.write_byte(player + kTurns, 2);
                } else {
                    FindBeside(player, second);
                    found = m_.read_byte(player + kScore) != 0;
                }
            }
            if (!found) {
                const auto random = draw();
                m_.write_word(player + kPlace, static_cast<uint16_t>((random & 7) + 8));
            }
            if (second == kDiamond) {
                m_.write_byte(player + kTurns, 0);
            }
        }
        const uint32_t presses = kPresses + static_cast<uint16_t>(m_.read_word(player + kPlace) * 2);
        m_.write_byte(player + kPressesRight, code_byte(presses));
        m_.write_byte(player + kPressesLeft, code_byte(presses + 1));
        SetPresses(player, m_.read_byte(player + kPressesRight), m_.read_byte(player + kPressesLeft), m_.read_byte(player + kTurns));
        m_.write_byte(player + kFound, 0);
        KeepFromHighSide(player);
        KeepFromTheTop(player);
        m_.write_byte(player + kStartWait, 1);
    }

    // 013846. The stacker's plan: it goes by the pair's colours against one colour it keeps (kStackerKept),
    // not by the field: pairs with that colour to the left, turned so that it comes where the others are,
    // and the rest to the right (to the left when the right is high).
    void StackerPlans() {
        const uint32_t player = player_;
        const uint16_t first = m_.read_word(player + kPairNext), second = m_.read_word(player + kPairNext + 2), kept = m_.read_word(player + kStackerKept);
        const auto otherTurn = [&]() {
            m_.write_byte(player + kStackerTurn, (m_.read_byte(player + kStackerTurn) + 1) & 1);
            return m_.read_byte(player + kStackerTurn) != 0;
        };
        if (first == second && first == kept) {
            SetPresses(player, 0, 1, 1);
        } else if (second == kept) {
            if (otherTurn()) {
                SetPresses(player, 1, 1, 1);
            } else {
                SetPresses(player, 0, 3, 3);
            }
        } else if (first == kept) {
            if (otherTurn()) {
                SetPresses(player, 0, 1, 3);
            } else {
                SetPresses(player, 0, 2, 1);
            }
        } else {
            SetPresses(player, 3, 0, 0);
            if (m_.read_word(m_.read_long(player + kFieldBottom) + 0x4A) != 0) {
                SetPresses(player, 0, 3, 0);
            }
        }
        KeepFromTheTop(player);
    }

    // --- the seeker ---
    // 015984. The seeker's plan: it scores every way the pair can be put down (upright in each column either
    // way up, and lying across each two columns either way round), by what each gem would land among, and
    // takes the best. Its sums are kept in the game's variables:
    static constexpr uint32_t kLanding = 0xFF0CCC;      // six longs: in each column, the cell a gem would come to rest in
    static constexpr uint32_t kDepth = 0xFF0CE4;        // six bytes: how many rows down that is
    static constexpr uint32_t kOpponentLanding = 0xFF0D1C, kOpponentDepth = 0xFF0D34; // the same of the opponent's field
    static constexpr uint32_t kGem = 0xFF0CEA, kOtherGem = 0xFF0D14; // words: the gem being scored, and the pair's other
    static constexpr uint32_t kScores = 0xFF0D10;       // long: where the scores being worked out go
    static constexpr uint32_t kWeights = 0xFF0D0C;      // long: the table that scores the gem's neighbours
    // The scores, a byte a column (and two bytes more): upright with the first gem under; with the second
    // under; lying with the first gem in the column and the second to its left; and the second to its right
    static constexpr uint32_t kFirstUnder = 0xFF0CEC, kSecondUnder = 0xFF0CF4, kSecondLeft = 0xFF0CFC, kSecondRight = 0xFF0D04;
    static constexpr uint32_t kBestSeekerColumn = 0xFF0CF2; // word (it lies over the end of kFirstUnder)
    static constexpr uint32_t kCrowding = 0xFF0D16;     // word: how full the two fields are: 0, 4, 8 or 12 for each, added
    static constexpr uint32_t kSharp = 0xFF0D1B;        // byte: the computer plays this pair its sharper way
    static constexpr uint32_t kChosen = 0xFF0D18;       // three bytes: the presses chosen (left, right, turns)
    static constexpr int32_t kShowThinking = 0x85;      // from A5, byte: a leftover of the game's making (see ShowThinking)
    // Tables. Some are read as the program is (Code8, Code16) and some as data (Mem8...): see spf2.h
    static constexpr uint32_t kCrowdingTable = 0x01604C;   // bytes: by depth (twelve) and column (six, from the right)
    static constexpr uint32_t kSharpChances = 0x0166C4;    // by character: words to rows of longs, by crowding; a bit in 32
    static constexpr uint32_t kWeightSets = 0x015AAE;      // by character: words to rows of longs, the tables by kind of gem
    static constexpr uint32_t kWeightSetOfGem = 0x015AC2;  // bytes: which of the row, by the gem (gems, crash gems, the diamond)
    static constexpr uint32_t kSharpWeights = 0x40;        // (the sharper way's table follows the usual one)
    static constexpr uint32_t kColumnScore = 0x015B2E;     // bytes: what a column scores to begin with, from the right
    static constexpr uint32_t kNeighbours = 0x015B34;      // words: where the seven neighbours of a cell are, from it
    static constexpr int kNeighbourCount = 7;
    static constexpr uint32_t kColumnPresses = 0x015DA0;   // bytes: presses to the left and to the right, by column
    static constexpr uint32_t kChanceTurns = 0x015DC2, kChancePresses = 0x015E34;
    // What a neighbour is to the gem, for the weights (a row of eight for each)
    enum : uint32_t { kIsEmpty = 0, kIsSame = 1, kIsSameColour = 2, kIsCounterOfColour = 4, kIsCounter = 5, kIsWall = 6, kIsBreaking = 7 };
    static constexpr int kRowsLooked = 12;
    static constexpr uint16_t kStageSeekerSure = 6;
    // (the time the game's code takes: the plan, and more for the sharper way)
    static constexpr uint32_t kSeekerTime = 34500, kSharpTime = 2000;

    // 015F18. Where a gem would land in each column of a field, and how far down. (It looks at twelve rows
    // from the second from the top, which leaves out the bottom row: an empty column counts as if it had
    // one gem in it.)
    void FindLandings(uint32_t field, uint32_t landing, uint32_t depth) {
        for (int column = 0; column < kFieldColumns; column++) {
            const uint32_t top = m_.read_long(field + kFieldBottom) + 2 + 2u * static_cast<uint32_t>(column);
            int row = 0;
            while (row < kRowsLooked && m_.read_word(top + static_cast<uint32_t>(row) * kRowSize) == 0) {
                row++;
            }
            const int down = row == kRowsLooked ? kRowsLooked - 1 : row;
            const uint32_t cell = top + static_cast<uint32_t>(row) * kRowSize - kRowSize;
            m_.write_long(landing + 4u * static_cast<uint32_t>(column), cell);
            m_.write_byte(depth + static_cast<uint32_t>(column), static_cast<uint8_t>(down));
        }
    }
    // 015F60. How full a field is, from its columns' depths: 0, 4, 8 or 12
    uint16_t Crowding(uint32_t depth) {
        uint16_t sum = 0;
        for (int column = 0; column < kFieldColumns; column++) {
            sum = static_cast<uint16_t>(sum + m_.read_byte(kCrowdingTable + m_.read_byte(depth + static_cast<uint32_t>(column)) * 6u +
                                                          static_cast<uint32_t>(5 - column)));
        }
        return sum < 0x10 ? 0 : sum < 0x20 ? 4 : sum < 0x30 ? 8 : 12;
    }
    // 015FA0. Whether this pair is played the sharper way: by chance, more often for some characters and
    // as the fields fill
    void DrawSharp(uint32_t player) {
        m_.write_byte(kSharp, 0);
        const auto random = draw();
        const uint32_t chance = random & 0x1F;
        const uint32_t rows = kSharpChances + m_.read_word(kSharpChances + m_.read_byte(player + kCharacter) * 2u);
        if (m_.read_long(rows + m_.read_word(kCrowding)) & (1u << chance)) {
            m_.write_byte(kSharp, 1);
        }
    }
    // 015FD6. With a switch that nothing in the game sets, the computer's thinking is shown as numbers in
    // the text layer by its field: the crowding, the sharper way, and its level
    void ShowThinking(uint32_t player) {
        if (!m_.read_byte(globals_ + kShowThinking)) {
            return;
        }
        const uint32_t at = m_.read_byte(player + kPlayerNumber) ? 0x900E28 : 0x9007A8;
        const auto put = [&](uint32_t where, uint16_t number) {
            m_.write_word(where, number);
            m_.write_word(where + 2, 0);
        };
        put(at, (m_.read_word(kCrowding) >> 2) & 0xF);
        put(at + 0x100, m_.read_byte(kSharp) & 1);
        put(at + 0x200, (m_.read_byte(player + kLevel) >> 4) & 1);
        put(at + 0x280, m_.read_byte(player + kLevel) & 0xF);
    }

    // 015B42. What a neighbour of the landing cell adds to the score: by what it is to the gem, from the
    // weights. A gem of another colour adds nothing.
    uint8_t NeighbourScore(uint16_t cell, int neighbour) {
        const uint16_t gem = m_.read_word(kGem);
        uint32_t is;
        if ((cell & 0xFF) == kWall) {
            is = kIsWall;
        } else if ((cell & kCellBreaking) && (cell & kColour) == gem) {
            is = kIsBreaking;
        } else {
            const uint16_t kind = (cell & kCellBreaking) ? (cell & kColour) : (cell & kKind);
            if (kind == kEmpty) {
                is = kIsEmpty;
            } else if (kind == kCounter) {
                is = ((cell >> 12) & 0xF) == (gem & 0xFF) ? kIsCounterOfColour : kIsCounter;
            } else if ((cell & kKind) == gem) {
                is = kIsSame;
            } else if ((cell & kKind) == (gem & kColour)) {
                is = kIsSameColour;
            } else {
                return 0;
            }
        }
        return m_.read_byte(m_.read_long(kWeights) + is * 8 + static_cast<uint32_t>(neighbour));
    }
    // 015BF4. Whether a gem beside where the pair's other gem will rest (on top of the first) is of its
    // colour. Crash gems only count the sharper way.
    bool BesideMatches(uint16_t beside, uint16_t other) {
        if (((other | beside) & kCrash) && !m_.read_byte(kSharp)) {
            return false;
        }
        const uint8_t colour = (beside & kCrash) || (other & kCrash) ? (beside & kColour) : (beside & 0xFF);
        return colour == (other & kColour);
    }
    // 015A8C. The scores of the pair upright in each column, with kGem under kOtherGem: the column's own
    // score, four for each row down, what the neighbours of the landing cell add, and sixteen for each gem
    // of the other's colour beside where the other comes
    void ScoreUpright(uint32_t player, uint16_t gem, uint16_t other, uint32_t scores) {
        m_.write_word(kGem, gem);
        m_.write_word(kOtherGem, other);
        m_.write_long(kScores, scores);
        const uint32_t sets = kWeightSets + m_.opcode_word(kWeightSets + m_.read_byte(player + kCharacter) * 2u);
        m_.write_long(kWeights, m_.read_long(sets + code_byte(kWeightSetOfGem + gem)) + (m_.read_byte(kSharp) ? kSharpWeights : 0));
        for (int column = 0; column < kFieldColumns; column++) {
            const uint32_t cell = m_.read_long(kLanding + 4u * static_cast<uint32_t>(column));
            uint8_t score = code_byte(kColumnScore + static_cast<uint32_t>(5 - column));
            score = static_cast<uint8_t>(score + (m_.read_byte(kDepth + static_cast<uint32_t>(column)) << 2));
            for (int n = kNeighbourCount - 1; n >= 0; n--) {
                const uint32_t there = cell + static_cast<uint32_t>(static_cast<int16_t>(m_.opcode_word(kNeighbours + 2u * static_cast<uint32_t>(n))));
                score = static_cast<uint8_t>(score + NeighbourScore(m_.read_word(there), n));
            }
            const uint32_t above = cell - kRowSize;
            score = static_cast<uint8_t>(score + (BesideMatches(m_.read_word(above - 2), other) ? 0x10 : 0) +
                                         (BesideMatches(m_.read_word(above + 2), other) ? 0x10 : 0));
            m_.write_byte(scores + static_cast<uint32_t>(column), score);
        }
    }
    // 015C6A. The sharper way's bonus: thirty-two where the gem would land on a pile of its colour that
    // rests on the other gem's colour, with a breaking gem there. (As the game has it this never comes
    // about: it asks for the mark of a cell that is breaking where a crash gem seems to be meant, and
    // nothing is breaking while the computer plans.)
    void ScoreSharp(uint16_t gem, uint16_t other, uint32_t scores) {
        m_.write_word(kGem, gem);
        m_.write_word(kOtherGem, other);
        m_.write_long(kScores, scores);
        for (int column = 0; column < kFieldColumns; column++) {
            uint32_t cell = m_.read_long(kLanding + 4u * static_cast<uint32_t>(column)) + kRowSize;
            const uint16_t under = m_.read_word(cell);
            if (!(gem & kCellBreaking) && !(under & kCellBreaking)) {
                continue;
            }
            if ((under & kColour) != (gem & kColour)) {
                continue;
            }
            // (down the pile to the first cell of another colour, twelve rows at most)
            uint16_t colour = gem & kColour;
            for (int n = 0; n < kRowsLooked; n++) {
                cell += kRowSize;
                colour = m_.read_word(cell) & kColour;
                if (colour != (gem & kColour)) {
                    break;
                }
            }
            if (colour == (gem & kColour) && (gem & kCellBreaking)) {
                continue; // (the pile goes all the way)
            }
            if (colour != (other & kColour)) {
                continue;
            }
            const auto breaking = [&](uint16_t there) { return !(there & 0x8000) && ((other | there) & kCellBreaking); };
            const uint16_t right = m_.read_word(cell - kRowSize + 2);
            if (breaking(m_.read_word(cell)) || breaking(m_.read_word(cell - kRowSize - 2)) || breaking(right)) {
                m_.write_byte(scores + static_cast<uint32_t>(column), static_cast<uint8_t>(m_.read_byte(scores + static_cast<uint32_t>(column)) + 0x20));
            }
        }
    }
    // 015A74. The score of the pair lying across two columns, from the two gems' upright scores there:
    // half their sum, and thirty-two more when both are good
    uint8_t ScoreLying(uint8_t one, uint8_t two) {
        const uint8_t half = static_cast<uint8_t>(static_cast<uint8_t>(one + two) >> 1);
        return one >= 0x40 && two >= 0x40 ? static_cast<uint8_t>(half + 0x20) : half;
    }

    void SeekerPlans() {
        const uint32_t player = player_;
        FindLandings(player, kLanding, kDepth);
        FindLandings(static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(m_.read_word(player + 0x34)))), kOpponentLanding, kOpponentDepth);
        m_.write_word(kCrowding, static_cast<uint16_t>(Crowding(kDepth) + Crowding(kOpponentDepth)));
        DrawSharp(player);
        ShowThinking(player);

        const uint16_t first = m_.read_word(player + kPairNext), second = m_.read_word(player + kPairNext + 2);
        ScoreUpright(player, first, second, kFirstUnder);
        ScoreUpright(player, second, first, kSecondUnder);
        m_.write_byte(kSecondLeft, 0);
        m_.write_byte(kSecondRight + 5, 0);
        for (uint32_t column = 1; column < kFieldColumns; column++) {
            m_.write_byte(kSecondLeft + column, ScoreLying(m_.read_byte(kSecondUnder + column - 1), m_.read_byte(kFirstUnder + column)));
            m_.write_byte(kSecondRight + column - 1, ScoreLying(m_.read_byte(kFirstUnder + column - 1), m_.read_byte(kSecondUnder + column)));
        }
        if (m_.read_byte(kSharp)) {
                ScoreSharp(first, second, kFirstUnder);
            ScoreSharp(second, first, kSecondUnder);
        }

        // The best: of equals, the one looked at last (the ways in this order, each from the right)
        const struct { uint32_t scores; uint8_t turns; } ways[4] = {{kSecondUnder, 2}, {kSecondLeft, 1}, {kSecondRight, 3}, {kFirstUnder, 0}};
        uint8_t best = 0, turns = 0;
        uint16_t bestColumn = 0;
        for (const auto &way : ways) {
            for (int column = kFieldColumns - 1; column >= 0; column--) {
                const uint8_t score = m_.read_byte(way.scores + static_cast<uint32_t>(column));
                if (score >= best) {
                    best = score;
                    bestColumn = static_cast<uint16_t>(column);
                    turns = way.turns;
                }
            }
        }
        m_.write_word(kBestSeekerColumn, bestColumn);

        if (m_.read_word(kStage) < kStageSeekerSure && m_.read_word(kCrowding) == 0) {
            // Against the first opponents, while the fields are still empty enough, it throws the plan away:
            // the place goes by the count of frames
            const uint8_t frames = m_.read_byte(globals_ + kFrameCount + 1);
            m_.write_byte(player + kPressTurnA, code_byte(kChanceTurns + (frames & 0xE)));
            m_.write_byte(player + kPressTurnB, code_byte(kChanceTurns + (frames & 0xE) + 1));
            m_.write_byte(player + kPressLeft, code_byte(kChancePresses + (frames & 0x1C)));
            m_.write_byte(player + kPressRight, code_byte(kChancePresses + (frames & 0x1C) + 1));
            return;
        }
        m_.write_byte(player + kPressLeft, code_byte(kColumnPresses + bestColumn * 2u));
        m_.write_byte(player + kPressRight, code_byte(kColumnPresses + bestColumn * 2u + 1));
        m_.write_byte(player + kPressTurnA, turns);
        m_.write_byte(kChosen, m_.read_byte(player + kPressLeft));
        m_.write_byte(kChosen + 1, m_.read_byte(player + kPressRight));
        m_.write_byte(kChosen + 2, turns);
    }

    // 013620. The plan for the pair that comes next, the character's way.
    void PlanPair() {
        const uint32_t player = player_;
        m_.write_byte(player + kStartWait, 1);
        if (m_.read_byte(player+kCharacter)>=12) throw std::runtime_error("Classic CPU character outside original table");
        switch (kPlans[m_.read_byte(player + kCharacter)]) {
        case Plan::Seeker:
            SeekerPlans();
            break;
        case Plan::Matcher:
            MatcherLooks();
            MatcherPlans();
            break;
        case Plan::Stacker:
            StackerPlans();
            break;
        }
    }
    // 012424. The wait before the next pair is steered.
    void SetThinkTime() {
        const uint32_t player = player_;
        m_.write_byte(player + kPacePairs, static_cast<uint8_t>(m_.read_byte(player + kPacePairs) + 1));
        if (static_cast<int8_t>(m_.read_byte(player + kPacePairs)) >= kPairsAStep) {
            m_.write_byte(player + kPacePairs, 0);
            m_.write_byte(player + kPaceStage, static_cast<uint8_t>(m_.read_byte(player + kPaceStage) + 1));
            if (static_cast<int8_t>(m_.read_byte(player + kPaceStage)) >= kPaceStageMost) {
                m_.write_byte(player + kPaceStage, kPaceStageMost);
            }
        }
        const uint16_t entry = static_cast<uint16_t>(m_.read_byte(player + kLevel) * 32 + m_.read_byte(kCourse) * 8 + m_.read_byte(player + kPaceStage));
        m_.write_byte(player + kThinkFrames, m_.read_byte(kThinkTable + entry));
        const auto random = draw();
        const uint16_t change = m_.read_word(kThinkChanges + 2 * (random & 0xF));
        const int16_t wait = static_cast<int16_t>(m_.read_byte(player + kThinkFrames) + change);
        m_.write_byte(player + kThinkFrames, wait < 0 ? 0 : static_cast<uint8_t>(wait));
    }

    // 012314. The pair's slips.
    void DrawSlips() {
        const uint32_t player = player_;
        m_.write_byte(player + kBeforeLetGo, 0);
        m_.write_byte(player + kLetGo, 0);
        if ((m_.read_byte(kCourse) == 2 && static_cast<int16_t>(m_.read_word(kStage)) > 2) || static_cast<int16_t>(m_.read_word(player + kPairsWaiting)) > 1) {
            return;
        }
        const auto random = draw();
        m_.write_byte(player + kBeforeLetGo, m_.read_byte(kBeforeLetGoTable + (random & 0xF) + (m_.read_byte(player + kLevel) >> 4)));
        const auto random2 = draw();
        int hurry = m_.read_byte(kLetGoTable + (random2 & 0xF) + (m_.read_byte(player + kLevel) >> 4));
        hurry -= m_.read_byte(kCourse) == 2 ? 20 : m_.read_byte(kCourse) == 1 ? 10 : 0;
        if (hurry < 0) {
            hurry = 0;
        }
        m_.write_byte(player + kLetGo, static_cast<uint8_t>(hurry));
        const int wait = m_.read_byte(player + kThinkFrames) - hurry;
        m_.write_byte(player + kThinkFrames, wait < 0 ? 0 : static_cast<uint8_t>(wait));
    }
    void clamp_skill() {
        const auto skill=static_cast<int16_t>(m_.read_word(globals_-0x733a));
        if(skill<0)m_.write_word(globals_-0x733a,0);
        else if(skill>=256)m_.write_word(globals_-0x733a,255);
    }
    void add_skill(uint16_t amount) {
        m_.write_word(globals_-0x733a,uint16_t(m_.read_word(globals_-0x733a)+amount));clamp_skill();
    }
    void level_from_skill(){m_.write_word(globals_+0x12e,m_.read_word(globals_-0x733a)>>3);}
public:
    ClassicCpu(Memory &memory,Effects &effects,uint32_t globals,uint32_t player):m_(memory),effects_(effects),globals_(globals),player_(player) {
        if(globals!=0xff8000)throw std::runtime_error("Classic CPU requires original global layout");
    }
    // 1565A: once on a new arcade game, after operator difficulty is installed.
    void start_skill(){
        auto level=m_.read_word(0x15834+2u*m_.read_byte(globals_+0xbe));
        m_.write_word(globals_+0x12e,level);m_.write_word(globals_-0x733a,uint16_t(level<<3));
    }
    // 15676: before a match, not before each pair. Tables handle all eight settings.
    void level_for_match(){
        m_.write_word(globals_-0x7336,0);
        auto hold=m_.read_word(globals_-0x7338);
        if(hold)m_.write_word(globals_-0x7338,uint16_t(hold-1));
        else {
            auto setting=m_.read_byte(globals_+0xbe);auto stage=m_.read_word(globals_+0x112);
            auto held=uint16_t(m_.read_word(stage>=8?0x158c4+2u*setting:0x15844+uint16_t(16u*setting+2u*stage))<<3);
            bool at_least=setting>=4;
            if(!at_least){
                if(held<m_.read_word(globals_-0x733a))m_.write_word(globals_-0x733a,held);
                at_least=stage==8||stage==7||stage==2||stage==5;
            }
            if(at_least&&held>=m_.read_word(globals_-0x733a))m_.write_word(globals_-0x733a,held);
        }
        clamp_skill();level_from_skill();
        for(auto offset:{0x300u,0x700u})if(m_.read_byte(globals_+offset+0x9e))
            m_.write_byte(globals_+offset+0xa8,uint8_t(m_.read_word(globals_+0x12e)));
    }
    // 1575E: called for a HUMAN chain; mode 3 bypasses adaptive difficulty.
    void skill_for_chain(uint16_t length){
        if(m_.read_byte(globals_+0xac)==3||m_.read_byte(player_+0x9e))return;
        length=length>=8?7:uint8_t(length);add_skill(m_.opcode_word(0x15784+2u*length));
    }
    // 157F0: A8D2 invokes this at each round result, after play-frame counting.
    void skill_after_match(){
        if(m_.read_byte(globals_+0xac)==3)return;
        auto time=m_.read_byte(globals_-0x7336);
        add_skill(time>=150?32:time>=120?16:time>=90?4:time>=60?8:16);
        m_.write_word(globals_-0x733a,uint16_t(m_.read_word(globals_-0x733a)-m_.read_word(0x15934+2u*m_.read_byte(globals_+0xbe))));
        auto random=draw();m_.write_word(globals_-0x733a,uint16_t(m_.read_word(globals_-0x733a)-m_.read_word(0x15944+(random&0x3e))));
        clamp_skill();level_from_skill();
    }
    void skill_after_continue(){
        level_for_match();m_.write_word(globals_-0x7338,1);
        auto count=m_.read_byte(player_+0x98);auto drop=count<16?m_.read_word(0x158d4+2u*count):0x48;
        m_.write_word(globals_-0x733a,uint16_t(m_.read_word(globals_-0x733a)-drop));
        auto random=draw();add_skill(m_.read_word(0x158f4+(random&0x3e)));clamp_skill();
    }
    // 5D6E prefix and 5EB4..5EC4. Call before the original fall-speed init;
    // this intentionally does not clear shared +355 or overwrite other stream state.
    void initialize_first_pair(){
        if(m_.read_byte(player_+0x9e)) {
            auto random=draw();m_.write_byte(player_+((random&1)?0x15a:0x15b),3);
        }
        m_.write_word(player_+0x356,uint16_t((m_.read_byte(player_+0x380)&3)+1));
        m_.write_word(player_+0x30e,0);m_.write_word(player_+0x310,0);m_.write_word(player_+0x312,4);
    }
    // 5F36..5F40: follows set_fall_speed at initialization, even for a human.
    void initialize_pacing(){SetGap();SetThinkTime();}
    // 5D56: P2-only CPU post-generation draw. Returned byte is not stored by original.
    void finish_second_player_initialization(){
        if(m_.read_byte(player_+0x9e)){auto random=draw();(void)code_byte(0x5d66+(random&7));}
    }
    void plan_pair(){PlanPair();}
    void set_think_time(){SetThinkTime();}
    void draw_slips(){DrawSlips();}
    void set_gap(){SetGap();}
    void steer(){ComputerSteers();}
};
} // namespace puzzle_fighter::prototype
