// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy (source C++ guide).
// Modified 2026-10-25: USA native memory/typed-call bindings.
// USA selection 68B4..875C. Source guide: Tommy McLeroy's GPL select.cpp;
// native memory/typed-call port checked against USA DATA and OPCODE streams.
#pragma once
#include <cstdint>
#include <utility>
#include "sound_requests.hpp"
#include "match_interlude.hpp"
namespace puzzle_fighter {
enum class SelectionAction { reset_pools, scene_actor_setup, first_pool, third_pool,
 objects, end_fade, character_accounting };
enum class SelectionResult { complete, wait_object_clear, unsupported };
template<class M,class H> struct SourceSelection {
 M&m; H&h; uint32_t g; uint32_t active_character=g+0x2900;
 const uint32_t kVars=g,kGameStep=g+4,kMatchStep=g+8,kRoundStep=g+12,kSubStep=g+16;
 const uint32_t kModeCursor=g-0x7375,kCharactersChosen=g-0x7373;
 static constexpr uint32_t kCharacter=0x82,kPlayerNumber=0x81,kPadRead=0x9a,kPadReadBefore=0x9c,kComputer=0x9e,kHandicap=0xa9,kFrameCount=0x80;
 uint32_t Player(unsigned n)const{return g+0x300+n*0x400;}
 uint8_t Ram8(uint32_t a){return m.read_byte(a);}uint16_t Ram16(uint32_t a){return m.read_word(a);}uint32_t Ram32(uint32_t a){return m.read_long(a);}
 void SetRam8(uint32_t a,uint8_t v){m.write_byte(a,v);}void SetRam16(uint32_t a,uint16_t v){m.write_word(a,v);}void SetRam32(uint32_t a,uint32_t v){m.write_long(a,v);}
 uint16_t Opcode16(uint32_t a){return m.opcode_word(a);}uint8_t Opcode8(uint32_t a){const auto w=m.opcode_word(a&~1u);return a&1?w:uint8_t(w>>8);}
 int16_t Offset(uint32_t a){return int16_t(Ram16(a));}
 void Write(uint32_t a,unsigned size,uint32_t v){if(size==1)SetRam16(a,v);else SetRam32(a,v);}
 uint32_t NewTask(uint16_t kind,bool third=false){const auto a=third?h.allocate_third():h.allocate_first();if(a){SetRam8(a,Ram8(a)+1);SetRam16(a+2,kind);}return a;}
 enum SoundKind { kSoundMove,kSoundChoose,kSoundCursor,kSoundTaken,kSoundModeScreen,kSoundLevelScreen,kSoundCharacters,kSoundMode,kSoundLevel,kSoundDone };
 void Sound(SoundKind kind){
  uint32_t number=0;
  switch(kind){
  case kSoundMove:number=0xe2;break;case kSoundChoose:number=0xe4;break;
  case kSoundCursor:number=0xe2+(Ram8(active_character+0x81)&1);break;
  case kSoundTaken:number=0xe4+(Ram8(active_character+0x81)&1);break;
  case kSoundModeScreen:number=0xa0;break;case kSoundLevelScreen:number=0xa2;break;case kSoundCharacters:number=0xa1;break;
  case kSoundMode:if(Ram8(g+0xbc))return;number=Opcode16(0x2b8ca+2*(Ram8(kModeCursor)&1));break;
  case kSoundLevel:number=Opcode16(0x2b8eeu+2*(Ram8(kLevelCursor)&3));break;
  case kSoundDone:number=Opcode16(0x2b928+2*(Ram8(active_character+0x82)+(Ram16(g+0x90)?0x10:0)));if(Ram8(active_character+0x81))number+=0x10;break;
  }
  queue_sound_request(g,m,{number,0x1000,0},true);
 }
    // --- the game's variables used here ---
    enum : uint16_t { kStepSetUp = 0, kStepChoosing = 2, kStepLeaving = 4 }; // kMatchStep
    enum : uint16_t { // kRoundStep, while choosing
        kScreenMode = 0, kScreenLevel = 2, kScreenCharacter = 4, kScreenCharacters = 6, kScreenJoining = 8, kScreenOpponent = 0x0A,
    };
    const uint32_t kTimer = kVars + 0x12;       // word: frames the step still has
    const uint32_t kSeconds = kVars + 0x16, kSecondFrames = kVars + 0x17; // bytes: a choice's time (decimal digits), and frames of its second
    const uint8_t kChoiceSeconds = 0x19, kHandicapSeconds = 0x09, kFramesASecond = 60;
    const uint32_t kTasksDone = (g + -29558);       // byte
    const uint8_t kTasksGone = 0xFF;
    const uint32_t kLevelCursor = (g + -29556);     // byte: 0 easy, 1 normal, 2 hard
    const uint32_t kPlayersSeen = (g + -29554);     // byte: kChoosing as it was when the screen came up
    const uint32_t kPortraitsStay = (g + -29553);   // byte: given to the tasks that frame the portraits
    const uint32_t kOpponentShowing = (g + -29552), kBackdropOn = (g + -29551); // bytes
    const uint32_t kLadderPage = (g + -29550);      // word: which of the layer's two pages the ladder is drawn in
    const uint32_t kHelpText = (g + -29548);        // word: the line of help shown (kNoHelp: none)
    const uint16_t kNoHelp = 0xFFFF;
    const uint32_t kLadderMark = (g + -29546);      // long: where a beaten opponent's mark goes
    // What each player has on the screen: the character, the cursor's place (0-7), the handicap level (0-4)
    const uint32_t kChosen[2] = {(g + -29566), (g + -29562)};
    const uint32_t kCursor = 1, kHandicapChosen = 2;
    const uint32_t kOrder = (g + -29530);           // the ladder's order: a character a stage (match.cpp)
    const uint32_t kChoosing = kVars + 0x8D;    // byte: the players choosing now (bit 0 player 1, bit 1 player 2)
    const uint32_t kPlaying = kVars + 0xAC;     // byte: the players already in the game
        const uint32_t kHandicaps = kVars + 0xBB;   // byte: not zero when the players choose handicap levels
    const uint32_t kShortLadder = kVars + 0xBF; // byte: the ladder is a stage shorter
    const uint32_t kDebug = kVars + 0x85;       // byte: left over from the game's making; nothing sets it
    const uint32_t kOtherLanguage = kVars + 0x92; // word: the help comes from another list
    const uint32_t kStage = kVars + 0x112, kLastStage = kVars + 0x114, kOpponentsMet = kVars + 0x110; // words (match.cpp)
    const uint16_t kLastStageNumber = 8;
    const uint32_t kLastOpponentIn = kVars + 0x138; // byte
    const uint32_t kHeld1 = kVars + 0x8F, kHeld2 = kVars + 0xBD; // bytes: set while the screen changes
    // For the video hardware: words the frame interrupt hands on
    const uint32_t kScreenSetting = kVars + 0x20, kLayerOrder = kVars + 0x32, kLayerSetting = kVars + 0x3A;
    const uint32_t kScrollX = kVars + 0x26, kScrollY = kVars + 0x28; // the backdrop's place
    const uint16_t kScreenPlain = 0x90C0, kScreenPortraits = 0x9280;

    // --- the two characters on the screen (A6 in the game's code): records like the players' ---
    const uint32_t kCharacters[2] = {kVars + 0xB00, kVars + 0xC00};
    const uint32_t kOn = 1;                     // byte
    const uint32_t kChoiceStep = 4;             // byte: 0 choosing a character, 2 a handicap level
    const uint32_t kFacing = 0x0B, kColoursAt = 0x0D; // bytes
    const uint32_t kChoiceSecondsLeft = 0x0E, kChoiceFrames = 0x0F; // bytes
    const uint32_t kX = 0x10, kY = 0x14;        // words
    const uint32_t kCodes = 0x50;               // three records of four bytes: how far each code has been followed
    const uint32_t kCodeStep = 0, kCodePlace = 1, kCodeDone = 2, kCodeNumber = 3;
    const uint32_t kOtherColours = 0x9F;        // byte (in the player's record too)
    // In a player's record
    const uint32_t kLevel = 0xA1, kPlayerState = 0x80;
    const uint16_t kButtons = 0x0300, kSecondButton = 0x0200, kStick = 0x000F, kStickRight = 1, kStickLeft = 2, kStartHeld = 0x8000;
    const uint16_t kKicks = 0x7000, kFirstKick = 0x1000; // (the panel's fourth to sixth buttons, which the game does not use)
    const int kHandicapLevels = 5;

    // --- the game's tables ---
    const uint32_t kLastStages = 0x006FBC, kLastStagesShort = 0x006FC4; // words, by level
    const uint32_t kCursorMoves = 0x0078FA;     // bytes, by the stick: how far the cursor goes
    const uint32_t kCharacterAt = 0x007906;     // bytes: the character at each of the cursor's places
    const uint32_t kPlaceOf = 0x00867A;         // bytes: the place of each character
    const uint32_t kLadderPlaces = 0x008682, kLadderMarks = 0x0086A2; // longs, by stage
    const uint32_t kCodeLists = 0x0086C2;       // the codes: for each, player 1's and player 2's moves of the stick
    const uint32_t kDigitTiles = 0x008246;      // words
    const uint32_t kWords = 0x008602;           // the screens' headings
    const uint32_t kShownAfterLadder = 0x007DEA; // bytes: the small pictures the ladder then shows
    const uint32_t kPortraits = 0x084560, kSmallPortraits = 0x08DC3C, kColours = 0x0A0000;
    const uint8_t kHiddenTimed = 8, kHiddenFirst = 9, kHiddenSecond = 10;

    // --- video memory ---
    // Colours: sixteen words a row; a colour's top four bits are how bright it is
    const uint32_t kColourRows = 0x90C000, kColourRow = 0x20;
    const uint16_t kDim = 0x8000, kBright = 0xF000;
    const uint32_t kModeColours[2] = {0x90C060, 0x90C080}, kLevelColours[3] = {0x90C040, 0x90C000, 0x90C020};
    // The text layer (banner.cpp): a tile is two words, the next tile down the next in memory
    const uint32_t kDown = 4, kAcross = 0x80;
    const uint32_t kBlank = 0x00200000;
    const uint32_t kTimerAt = 0x903310, kChoiceTimerAt[2] = {0x902838, 0x902F38};
    const uint32_t kPortraitAt[2] = {0x90212C, 0x90302C}, kPortraitFrameAt[2] = {0x902028, 0x903028};
    const uint32_t kStageNumberAt = 0x90420C;

    void NextSubStep() {
        SetRam16(kSubStep, static_cast<uint16_t>(Ram16(kSubStep) + 2));
    }
    void Add16(uint32_t at, int amount) {
        SetRam16(at, static_cast<uint16_t>(Ram16(at) + amount));
    }
    // Counts the step's frames down: true when they are over
    bool TimerOut() {
        Add16(kTimer, -1);
        return Ram16(kTimer) == 0;
    }
    // A choice's time: a second less every kFramesASecond frames, in decimal digits. True when it has run out.
    bool SecondsOut(uint32_t seconds, uint32_t frames) {
        SetRam8(frames, static_cast<uint8_t>(Ram8(frames) - 1));
        if (Ram8(frames) != 0) {
            return false;
        }
        SetRam8(frames, kFramesASecond);
        const uint8_t left = Ram8(seconds);
        if (left == 0) {
            return true;
        }
        SetRam8(seconds, static_cast<uint8_t>((left & 0x0F) ? left - 1 : left - 7));
        return false;
    }
    // A new task of this kind, or 0 when there is no room for one
    // --- putting things into video memory ---

    // 008282. How bright a row of colours is.
    void SetBrightness(uint32_t row, uint16_t brightness) {
        for (int n = 0; n < 16; n++) {
            Write(row + 2 * n, 1, (Ram16(row + 2 * n) & 0x0FFF) | brightness);
        }
    }
    // A letter or digit of four tiles
    void PutBig(uint32_t at, uint16_t tile, uint16_t colours) {
        Write(at, 1, tile);
        Write(at + 2, 1, colours);
        Write(at + kAcross, 1, static_cast<uint16_t>(tile + 1));
        Write(at + kAcross + 2, 1, colours);
        Write(at + kDown, 1, static_cast<uint16_t>(tile + 0x10));
        Write(at + kDown + 2, 1, colours);
        Write(at + kAcross + kDown, 1, static_cast<uint16_t>(tile + 0x11));
        Write(at + kAcross + kDown + 2, 1, colours);
    }
    void PutBlank(uint32_t at) {
        Write(at, 2, kBlank);
        Write(at + kDown, 2, kBlank);
        Write(at + kAcross, 2, kBlank);
        Write(at + kAcross + kDown, 2, kBlank);
    }
    // 008220. A time of two decimal digits, in big digits.
    void ShowSeconds(uint32_t at, uint8_t seconds, uint16_t colours) {
        PutBig(at, Opcode16(kDigitTiles + 2 * (seconds >> 4)), colours);
        PutBig(at + 2 * kAcross, Opcode16(kDigitTiles + 2 * (seconds & 0x0F)), colours);
    }
    // 00857A. One of the screens' headings (a list: where, the colours, then a tile a letter).
    void PutWords(int which) {
        uint32_t list = kWords + Offset(kWords + 2 * which);
        const uint16_t column = Ram16(list), row = Ram16(list + 2), colours = Ram16(list + 4);
        uint32_t at = 0x900000 + static_cast<int16_t>((column << 7) + ((row & 0x1F) << 2) + ((row & 0x20) << 8));
        for (list += 6; !(Ram16(list) & 0x8000); list += 2, at += 2 * kAcross) {
            if (Ram16(list) == 0) {
                PutBlank(at);
            } else {
                PutBig(at, Ram16(list), colours);
            }
        }
    }
    // 008470. The backdrop of the screens before a match: a pattern over the whole of its layer; and the
    // tasks that go with it.
    void DrawBackdrop() {
        uint32_t tiles[4] = {0x63D30016, 0x63E30016, 0x63F30016, 0x64030016};
        uint32_t at = 0x908000;
        for (int column = 0; column < 64; column++) {
            for (int part = 0; part < 4; part++) {
                for (int n = 0; n < 4; n++) {
                    at -= 16;
                    for (int tile = 0; tile < 4; tile++) {
                        Write(at + 4 * tile, 2, tiles[tile]);
                    }
                }
                for (uint32_t &tile : tiles) {
                    tile = (tile + 0x30000) & 0xFFF3FFFF;
                }
            }
            std::swap(tiles[0], tiles[2]);
            std::swap(tiles[1], tiles[3]);
        }
        if (NewTask(0x2700)) {
            NewTask(0x0400, true);
        }
        SetRam16(kLayerOrder, 0x1B0E);
        SetRam8(kBackdropOn, 1);
        SetRam16(kHelpText, 0);
    }
    // 008094. The layers the choices are drawn in, emptied.
    void ClearLayers() {
        const uint32_t kEmptyTile = 0x58200000;
        const uint32_t kParts[][2] = {{0x904000, 0x906000}, {0x906800, 0x907000}, {0x907800, 0x908000}};
        for (const auto &part : kParts) {
            for (uint32_t at = part[1]; at > part[0];) {
                at -= 4;
                Write(at, 2, kEmptyTile);
            }
        }
    }
    // 008532. A character's colours (two rows; its other colours are the next two in the game's list) put
    // where the character's picture looks for them, as bright as what was there.
    void SetColours(uint32_t character) {
        active_character = character;
        if (!Ram8(character + kOn)) {
            return;
        }
        uint32_t from = kColours + ((Ram8(character + kCharacter) * 2u + Ram8(character + kOtherColours)) << 7);
        uint32_t to = kColourRows + ((Ram8(character + kColoursAt) + 0x0Cu) & 0x1F) * kColourRow;
        const uint32_t brightness = Ram32(to) & 0xF000F000;
        for (int n = 0; n < 16; n++, from += 4, to += 4) {
            Write(to, 2, Ram32(from) | brightness);
        }
    }
    // 0079B0. The big portrait of the character a player's cursor is on.
    void ShowPortrait(int n) {
        const uint32_t character = kCharacters[n];
        uint32_t from = kPortraits + Offset(kPortraits + 0x0C);
        from = Ram32(from + (Ram8(character + kCharacter) << 4) + 4) + 6;
        uint32_t to = kPortraitAt[n];
        for (int row = 0; row < 14; row++, to += kAcross) {
            for (int tile = 0; tile < 5; tile++, from += 4) {
                Write(to + 4 * tile, 2, Ram32(from));
            }
        }
    }
    // 007A00. What is around the portrait, taken away.
    void ClearPortraitFrame(int n) {
        uint32_t to = kPortraitFrameAt[n];
        for (int row = 0; row < 16; row++, to += kAcross) {
            for (int tile = 0; tile < 6; tile++) {
                Write(to + 4 * tile, 2, kBlank);
            }
        }
    }
    // 008102, 00815E. The time a player still has to choose, in tall digits, and the same taken away.
    void ShowChoiceTimer(uint32_t character) {
        uint32_t at = kChoiceTimerAt[Ram8(character + kPlayerNumber) ? 1 : 0];
        const uint8_t seconds = Ram8(character + kChoiceSecondsLeft);
        for (const uint16_t digit : {static_cast<uint16_t>(seconds >> 4), static_cast<uint16_t>(seconds & 0x0F)}) {
            Write(at, 1, static_cast<uint16_t>(0xA0 + digit));
            Write(at + 2, 1, 0x1F);
            Write(at + kDown, 1, static_cast<uint16_t>(0xB0 + digit));
            Write(at + kDown + 2, 1, 0x1F);
            at += kAcross;
        }
    }
    void ClearChoiceTimer(uint32_t character) {
        PutBlank(kChoiceTimerAt[Ram8(character + kPlayerNumber) ? 1 : 0]);
    }
    // 007D5E. A small portrait: three tiles by three, in the ladder's layer.
    void PutSmall(uint32_t to, uint32_t pictures, uint32_t which) {
        uint32_t from = Ram32(pictures + (which << 4) + 4) + 6;
        for (int row = 0; row < 3; row++, to += 0x40) {
            for (int tile = 0; tile < 3; tile++, from += 4) {
                Write(to + 4 * tile, 2, Ram32(from) + 0x58000000);
            }
        }
    }
    uint32_t SmallPictures() {
        return kSmallPortraits + Offset(kSmallPortraits + 4);
    }
    uint32_t LadderPlace(int stage, uint16_t page) {
        return Ram32(kLadderPlaces + 4 * stage) + static_cast<int16_t>(page);
    }
    // Which page of the ladder's layer is showing
    uint16_t PageShowing() {
        return Ram16(kScrollX) ? 0x800 : 0;
    }

    // --- the characters on the screen ---

    void SetAct(uint32_t character, uint32_t act) {
        active_character=character; h.actor_act(character,act);
    }
    static constexpr uint32_t kActWaiting = 0x17, kActChosen = 0x18;

    // The start of 0074DE and 008324: a player's character put in its place at its side of the screen
    void PlaceCharacter(int n, uint32_t character) {
        SetRam8(character + kOn, 1);
        SetRam16(character + kX, n ? 0x150 : 0x30);
        SetRam16(character + kY, 0x30);
        SetRam16(character + 0xD6, 0x30);
        SetRam16(character + 0x30, 0xC000);
        SetRam8(character + kFacing, n ? 0 : 1);
        SetRam16(character + 0x0C, n ? 0 : 0xFF19);
    }
    // The tasks that go with a character: what it stands on, and the frame of its portrait
    bool CharacterTasks(int n, uint16_t stand) {
        uint32_t task = NewTask(stand);
        if (!task) {
            return false;
        }
        SetRam16(task + kX, n ? 0x150 : 0x30);
        SetRam16(task + kY, 0x28);
        SetRam16(task + 0x0C, 0xFF08);
        SetRam16(task + 0x32, static_cast<uint16_t>(kCharacters[n]));
        task = NewTask(static_cast<uint16_t>(0x1800 + n));
        if (!task) {
            return false;
        }
        SetRam8(task + 0x0A, 0);
        SetRam32(task + 0x32, kChosen[n]);
        return true;
    }
    // 0074DE, 0075DA. A player's character comes on to be chosen.
    void BringIn(int n) {
        const uint32_t character = kCharacters[n], player = Player(n);
        const bool choosing = Ram8(kChoosing) & (1 << n);
        active_character = character;
        PlaceCharacter(n, character);
        SetRam8(character + kCharacter, Ram8(player + kCharacter));
        SetRam8(kChosen[n], Ram8(player + kCharacter));
        SetRam8(character + kPlayerNumber, static_cast<uint8_t>(n));
        SetRam8(character + kChoiceSecondsLeft, kChoiceSeconds);
        SetRam8(character + kChoiceFrames, kFramesASecond);
        SetRam16(character + kPadRead, 0);
        SetRam16(character + kPadReadBefore, 0);
        if (choosing) {
            SetRam8(character + kOtherColours, 0);
            SetRam8(player + kOtherColours, 0);
        }
        SetRam8(player + kHandicap, 2);
        if (!CharacterTasks(n, 0x2200)) {
            return;
        }
        if (choosing) {
            const uint32_t task = NewTask(static_cast<uint16_t>(0x2100 + n)); // the cursor
            if (!task) {
                return;
            }
            SetRam32(task + 0x32, kChosen[n]);
        }
        SetRam8(kChosen[n] + kCursor, Ram8(kPlaceOf + Ram8(kChosen[n])));
        SetAct(character, Ram8(kPlaying) & (1 << n) ? kActChosen : kActWaiting);
        SetColours(character);
    }
    // 008324, 0083CC. A character comes back on as it was chosen (when the opponent is shown).
    void BringBack(int n) {
        const uint32_t character = kCharacters[n], player = Player(n);
        active_character = character;
        PlaceCharacter(n, character);
        SetRam8(character + kCharacter, Ram8(kChosen[n]));
        SetRam8(character + kPlayerNumber, 0); // (for player 2 as well)
        if (Ram8(player + kComputer)) {
            SetRam8(character + kOtherColours, 0);
            SetRam8(player + kOtherColours, 0);
        }
        if (!CharacterTasks(n, 0x2202)) {
            return;
        }
        SetAct(character, Ram8(kPlaying) & (1 << n) ? kActChosen : kActWaiting);
        SetColours(character);
    }
    // 0082B0. The frames of the two portraits.
    void PortraitFrames() {
        for (int n = 0; n < 2; n++) {
            const uint32_t task = NewTask(static_cast<uint16_t>(0x1800 + n));
            if (!task) {
                return;
            }
            SetRam8(task + 0x0A, 1);
            SetRam8(task + 0x0E, Ram8(kPortraitsStay));
        }
    }
    // 008294. Eight tasks: the small portraits coming in.
    void SmallPortraitTasks() {
        for (int n = 7; n >= 0; n--) {
            const auto task=h.allocate_first();
            if (!task) {
                return;
            }
            SetRam8(task, static_cast<uint8_t>(Ram8(task) + 1));
            SetRam8(task + 2, 0x17);
            SetRam8(task + 3, static_cast<uint8_t>(n));
        }
    }
    // 0077BE. The two characters are shown.
    void ShowCharacters() {
        for (const uint32_t character : kCharacters) {
            active_character = character;
            if (Ram8(character + kOn)) {
                h.actor_step(character);
            }
        }
    }

    // --- a player choosing ---

    uint16_t Pressed(uint32_t character) {
        return Ram16(character + kPadRead) & ~Ram16(character + kPadReadBefore);
    }
    // 008186. A code is followed: while Start is held with the player's cursor on the code's character
    // (character 0 for player 1, 6 for player 2), each move of the stick has to be the code's next; any
    // other starts it again. Once the code is done, any move at all undoes it.
    void FollowCode(uint32_t character, uint32_t chosen, uint32_t code) {
        const auto startAgain = [&]() {
            SetRam8(code + kCodeStep, 0);
            SetRam8(code + kCodePlace, 0);
            SetRam8(code + kCodeDone, 0);
        };
        const bool second = Ram8(character + kPlayerNumber) != 0;
        if (!(Ram16(character + kPadRead) & kStartHeld) || Ram8(chosen) != (second ? 6 : 0)) {
            startAgain();
            return;
        }
        const uint16_t moved = Pressed(character) & kStick;
        if (Ram8(code + kCodeStep) != 0) {
            if (moved) {
                startAgain();
            }
            return;
        }
        const uint32_t list = kCodeLists + Offset(kCodeLists + Ram8(code + kCodeNumber) * 4 + (second ? 2 : 0));
        const uint16_t next = Ram16(list + Ram8(code + kCodePlace));
        if (next == 0xFFFF) {
            SetRam8(code + kCodeStep, 2);
            SetRam8(code + kCodeDone, 1);
        } else if (moved == next) {
            SetRam8(code + kCodePlace, static_cast<uint8_t>(Ram8(code + kCodePlace) + 2));
        } else if (moved) {
            startAgain();
        }
    }
    // 00790E. A player who has done a code gets its character in place of the one chosen: the third code
    // (thirteen downs) while the time shows 10; else the second (the same thirteen downs); else the first.
    void HiddenChoice(uint32_t character, uint32_t chosen) {
        uint8_t hidden;
        if (Ram8(character + kCodes + 8 + kCodeDone) && Ram8(character + kChoiceSecondsLeft) == 0x10) {
            hidden = kHiddenTimed;
        } else if (Ram8(character + kCodes + 4 + kCodeDone)) {
            hidden = kHiddenSecond;
        } else if (Ram8(character + kCodes + kCodeDone)) {
            hidden = kHiddenFirst;
        } else {
            return;
        }
        SetRam8(character + kCharacter, hidden);
        SetRam8(chosen, hidden);
    }
    // 00794C. Which colours the character has: the other ones when chosen with the second button; and when
    // the other player, a person, has already chosen the same character, whichever that player has not.
    void ChooseColours(int n) {
        const uint32_t character = kCharacters[n], other = kCharacters[n ^ 1];
        uint8_t colours = Pressed(character) & kSecondButton ? 1 : 0;
        if (Ram8(character + kCharacter) == Ram8(other + kCharacter) && !Ram8(Player(n ^ 1) + kComputer) &&
            (Ram8(kCharactersChosen) & (1 << (n ^ 1)))) {
            colours = Ram8(other + kOtherColours) ^ 1;
        }
        SetRam8(character + kOtherColours, colours);
        SetRam8(Player(n) + kOtherColours, colours);
    }
    // 0077F2. A frame of a player choosing a character.
    void ChoosingCharacter(int n) {
        const uint32_t character = kCharacters[n], chosen = kChosen[n];
        ShowChoiceTimer(character);
        ShowPortrait(n);
        if (!SecondsOut(character + kChoiceSecondsLeft, character + kChoiceFrames)) {
            for (uint32_t code = 0; code < 3; code++) {
                SetRam8(character + kCodes + 4 * code + kCodeNumber, static_cast<uint8_t>(code));
                FollowCode(character, chosen, character + kCodes + 4 * code);
            }
            const uint16_t pressed = Pressed(character);
            if (!(pressed & kButtons)) {
                // The cursor moves, but not off the end of a row of four on to the other row
                const int8_t move = static_cast<int8_t>(Opcode8(kCursorMoves + (pressed & kStick)));
                if (!(pressed & kStick) || move == 0) {
                    return;
                }
                if (Ram8(chosen + kCursor) >= 8) {
                    SetRam8(chosen + kCursor, 0);
                }
                const uint8_t was = Ram8(chosen + kCursor), place = static_cast<uint8_t>(was + move);
                if (place >= 8 || (place == 4 && was == 3) || (place == 3 && was == 4)) {
                    return;
                }
                SetRam8(chosen + kCursor, place);
                SetRam8(chosen, Opcode8(kCharacterAt + place));
                SetRam8(character + kCharacter, Ram8(chosen));
                active_character = character;
                Sound(kSoundCursor);
                SetAct(character, kActWaiting);
                SetColours(character);
                return;
            }
        }
        // Chosen, or out of time
        SetRam8(kCharactersChosen, Ram8(kCharactersChosen) | (1 << n));
        HiddenChoice(character, chosen);
        active_character = character;
        Sound(kSoundTaken);
        Sound(kSoundDone);
        h.action(SelectionAction::character_accounting,Ram8(character+kCharacter),character);
        ClearChoiceTimer(character);
        ChooseColours(n);
        SetAct(character, kActChosen);
        SetColours(character);
        ClearPortraitFrame(n);
    }
    // 007A38. A frame of a player choosing a handicap level, with the stick left and right.
    void ChoosingHandicap(int n) {
        const uint32_t character = kCharacters[n], chosen = kChosen[n];
        ShowChoiceTimer(character);
        const bool out = SecondsOut(character + kChoiceSecondsLeft, character + kChoiceFrames);
        const uint16_t pressed = Pressed(character);
        if (out || (pressed & kButtons)) {
            SetRam8(kCharactersChosen, Ram8(kCharactersChosen) | (1 << n));
            active_character = character;
            Sound(kSoundTaken);
            ClearChoiceTimer(character);
            return;
        }
        if (!(pressed & (kStickLeft | kStickRight))) {
            return;
        }
        int level = Ram8(chosen + kHandicapChosen);
        level += pressed & kStickLeft ? -1 : 1;
        if (level < 0 || level >= kHandicapLevels) {
            return;
        }
        SetRam8(chosen + kHandicapChosen, static_cast<uint8_t>(level));
        SetRam8(Player(n) + kHandicap, static_cast<uint8_t>(level));
        active_character = character;
        Sound(kSoundCursor);
    }
    // 0076D6. A frame of the players choosing, each until done; when both are, the screen's next step.
    void PlayersChoose() {
        for (int n = 0; n < 2; n++) {
            const uint32_t character = kCharacters[n], chosen = kChosen[n];
            if (Ram8(kCharactersChosen) & (1 << n)) {
                continue;
            }
            active_character = character;
                if (Ram8(character + kChoiceStep) == 0) {
                ChoosingCharacter(n);
            } else {
                ChoosingHandicap(n);
            }
            if (Ram8(kDebug)) {
                // (the panel's fourth to sixth buttons take the hidden characters outright)
                for (int button = 0; button < 3; button++) {
                    if (Ram16(character + kPadRead) & (0x1000 << button)) {
                        SetRam8(character + kCharacter, static_cast<uint8_t>(8 + button));
                        SetRam8(Player(n) + kCharacter, static_cast<uint8_t>(8 + button));
                        SetRam8(chosen, static_cast<uint8_t>(8 + button));
                    }
                }
            }
        }
        if (Ram8(kCharactersChosen) == 3) {
            NextSubStep();
            SetRam16(kTimer, 0x1E);
            SetRam8(kHeld1, 1);
            SetRam8(kHeld2, 1);
        }
    }

    // --- the ladder of opponents ---

    // 008006. A small portrait put on the ladder, and the computer's side made that character.
    void PutOpponent(uint32_t to, uint32_t character, uint32_t picture) {
        const int n = Ram8(Player(0) + kComputer) ? 0 : 1;
        SetRam8(kChosen[n], static_cast<uint8_t>(character));
        SetRam8(kCharacters[n] + kCharacter, static_cast<uint8_t>(character));
        PutSmall(to, SmallPictures(), picture);
        SetAct(kCharacters[n], kActWaiting);
        SetColours(kCharacters[n]);
    }
    // 007C7E, 007C86. The ladder drawn: each stage's opponent if already beaten (with a mark over it), else
    // a blank.
    void DrawLadder(uint16_t page) {
        SetRam16(kLadderPage, page);
        for (int stage = 7; stage >= 0; stage--) {
            const uint32_t character = Ram8(kOrder + stage);
            const uint32_t to = LadderPlace(stage, page);
            SetRam32(kLadderMark, Ram32(kLadderMarks + 4 * stage));
            if (!(Ram16(kOpponentsMet) & (1u << (character & 31)))) {
                // (007FEA: the blank)
                uint16_t tile = 0x5BAA;
                for (int column = 0; column < 3; column++, tile += 0x10) {
                    for (int row = 0; row < 3; row++) {
                        Write(to + 4 * column + 0x40 * row, 1, static_cast<uint16_t>(tile + row));
                        Write(to + 4 * column + 0x40 * row + 2, 1, 0x17);
                    }
                }
                continue;
            }
            PutOpponent(to, character, character + 0x0B);
            uint32_t from = kPortraits + Offset(kPortraits + 0x0A);
            from = Ram32(from + 4) + 6;
            uint32_t mark = Ram32(kLadderMark);
            for (int row = 0; row < 6; row++, mark += kAcross) {
                for (int tile = 0; tile < 6; tile++, from += 4) {
                    Write(mark + 4 * tile, 2, Ram32(from));
                }
            }
        }
    }
    void SpinSound() {
        if ((Ram8(kVars + kFrameCount) & 3) == 0) {
            Sound(kSoundMove);
        }
    }
    // 007D0C. All eight places of the ladder run through the characters.
    void SpinLadder() {
        SpinSound();
        for (int stage = 7; stage >= 0; stage--) {
            PutSmall(LadderPlace(stage, PageShowing()), SmallPictures(), ((Ram16(kTimer) >> 2) + stage) & 7);
        }
    }
    // 007D94. The stage's place on the ladder runs through the characters.
    void SpinStage() {
        SpinSound();
        const uint16_t stage = Ram16(kStage);
        if (stage < 8) {
            const uint32_t character = (Ram16(kTimer) & 0x1C) >> 2;
            PutOpponent(LadderPlace(stage, PageShowing()), character, character);
        }
    }
    // The computer's side becomes a character no one chose, where a person is not playing it
    void GiveSides(uint8_t character) {
        for (int n = 0; n < 2; n++) {
            if ((Ram8(kChoosing) | Ram8(kPlaying)) & (1 << n)) {
                continue;
            }
            SetRam8(Player(n) + kCharacter, character);
            SetRam8(kCharacters[n] + kCharacter, character);
            SetRam8(kChosen[n], character);
            BringBack(n);
            SetAct(kCharacters[n], kActChosen);
            ShowPortrait(n);
        }
    }
    // 007EA2. The whole ladder shows the last opponent, who comes on.
    void ShowLastOpponent() {
        for (int stage = 7; stage >= 0; stage--) {
            PutSmall(LadderPlace(stage, PageShowing()), SmallPictures(), 9);
        }
        GiveSides(9);
    }
    // 007DF2. The same for the opponent who comes after the ladder, unless the last opponent has come in.
    void ShowAfterLadder() {
        if (Ram8(kLastOpponentIn)) {
            ShowLastOpponent();
            return;
        }
        for (int stage = 7; stage >= 0; stage--) {
            PutSmall(LadderPlace(stage, PageShowing()), SmallPictures(), Opcode8(kShownAfterLadder + stage));
        }
        GiveSides(0x0A);
    }
    // 007F46, 007F4C. The stage's opponent is put on the ladder and on the computer's side of the screen, in
    // its other colours if the person has the same character in the first.
    void ShowStageOpponent(uint16_t page) {
        const uint16_t stage = Ram16(kStage);
        if (stage >= 8) {
            return;
        }
        const uint8_t character = Ram8(kOrder + stage);
        PutOpponent(LadderPlace(stage, page), character, character);
        const int n = Ram8(Player(0) + kComputer) ? 0 : 1;
        const uint32_t side = kCharacters[n], player = Player(n), person = Player(n ^ 1);
        SetRam8(side + kCharacter, character);
        SetRam8(player + kCharacter, character);
        SetRam8(side + kOtherColours, 0);
        SetRam8(player + kOtherColours, 0);
        if (Ram8(person + kCharacter) == character) {
            const uint8_t colours = Ram8(person + kOtherColours) ^ 1;
            SetRam8(player + kOtherColours, colours);
            SetRam8(side + kOtherColours, colours);
            SetColours(side);
        }
        ShowPortrait(n);
    }
    // 0082EA. The characters come back on after a match.
    void BringBackAfterMatch() {
        const bool last = Ram16(kStage) == kLastStageNumber;
        for (int n = 0; n < 2; n++) {
            if (!last || (Ram8(kPlaying) & (1 << n))) {
                BringBack(n);
            }
        }
        SetRam8(kPortraitsStay, 1);
        PortraitFrames();
    }
    // 007C5E. Whether a player who is choosing has pressed a button.
    bool AnyButton() {
        uint16_t pressed=0;
        for(unsigned n=0;n<2;++n) if(Ram8(kChoosing)&(1u<<n)) {
            const auto off=n?0x5c:0x58;
            pressed|=(Ram16(g+off)&~Ram16(g+off+2))&0x7700;
            if((Ram8(g+0x60)&~Ram8(g+0x61))&(1u<<n))pressed|=0x8000;
        }
        return pressed!=0;
    }

    // --- the screens ---

    void ScrollBackdrop() {
        Add16(kScrollX, -3);
        Add16(kScrollY, 2);
    }
    // The backdrop is stopped where its pattern repeats, and the portraits' frames and the small portraits
    // start coming in (the end of the choice of mode, and of level)
    void TurnToCharacters() {
        SetRam8(kTasksDone, 0);
        SetRam8(kBackdropOn, 0);
        SetRam16(kHelpText, kNoHelp);
        SetRam16(kTimer, 2);
        PutWords(2);
        SetRam16(kScrollX, Ram16(kScrollX) & 0x3F);
        SetRam16(kScrollY, static_cast<uint16_t>((Ram16(kScrollY) & 0x3F) + 0x100));
        SetRam8(kPortraitsStay, 0);
        PortraitFrames();
        SmallPortraitTasks();
    }
    // The backdrop has gone by: on to a screen of characters
    bool BackdropGone() {
        SetRam16(kScrollY, Ram16(kScrollY) & 0x3FF);
        if (Ram16(kScrollY) < 0x200) {
            return false;
        }
        SetRam16(kScrollX, 0);
        SetRam16(kScrollY, 0x300);
        SetRam16(kSubStep, 0);
        SetRam16(kTimer, 0x14);
        return true;
    }
    // The stick's and buttons' new presses on the choice of mode and of level: both players' when both are
    // in, else the one's who is
    uint16_t MenuPressed(bool both) {
        uint16_t now = Ram16(kCharacters[0] + kPadRead), before = Ram16(kCharacters[0] + kPadReadBefore);
        if (!(Ram8(kChoosing) & 1)) {
            now = Ram16(kCharacters[1] + kPadRead);
            before = Ram16(kCharacters[1] + kPadReadBefore);
        } else if (both && (Ram8(kChoosing) & 2)) {
            now |= Ram16(kCharacters[1] + kPadRead);
            before |= Ram16(kCharacters[1] + kPadReadBefore);
        }
        return now & ~before;
    }
    void ShowHelp(uint8_t cursor, uint16_t first, uint16_t firstOther) {
        SetRam16(kHelpText, static_cast<uint16_t>(cursor + (Ram16(kOtherLanguage) ? firstOther : first)));
        ShowSeconds(kTimerAt, Ram8(kSeconds), 0x1F);
    }

    // 0069F8. The choice of mode.
    void ShowModeCursor() {
        SetBrightness(kModeColours[0], Ram8(kModeCursor) ? kDim : kBright);
        SetBrightness(kModeColours[1], Ram8(kModeCursor) ? kBright : kDim);
    }
    void ChooseMode() {
        switch (Ram16(kSubStep)) {
        case 0: { // the two choices come in
            NextSubStep();
            SetBrightness(kModeColours[0], kDim);
            SetBrightness(kModeColours[1], kDim);
            PutWords(0);
            for (int n = 0; n < 2; n++) {
                const uint32_t task = NewTask(static_cast<uint16_t>(0x1900 + n));
                if (!task) {
                    break;
                }
                SetRam32(task + 0x32, kModeCursor);
                if (n == 1) {
                    SetRam16(kLayerOrder, 0x1B0E);
                }
            }
            break;
        }
        case 2:
            if (Ram8(kTasksDone) == 3) {
                NextSubStep();
                SetRam8(kSeconds, kChoiceSeconds);
                SetRam8(kSecondFrames, kFramesASecond);
                Sound(kSoundModeScreen);
            }
            break;
        case 4: { // choosing: the stick moves the cursor, a button takes it
            const bool out = SecondsOut(kSeconds, kSecondFrames);
            const uint16_t pressed = out ? 0 : MenuPressed(false);
            if (out || (pressed & kButtons)) {
                NextSubStep();
                SetRam16(kTimer, 0x1E);
                Sound(kSoundChoose);
                SetRam8(kVars + 0xE0, Ram8(kModeCursor));
                Sound(kSoundMode);
            } else {
                if ((pressed & kStickLeft) && Ram8(kModeCursor)) {
                    SetRam8(kModeCursor, 0);
                    Sound(kSoundMove);
                }
                if ((pressed & kStickRight) && !Ram8(kModeCursor)) {
                    SetRam8(kModeCursor, 1);
                    Sound(kSoundMove);
                }
            }
            ShowModeCursor();
            ShowHelp(Ram8(kModeCursor), 0xD2, 0x2C);
            break;
        }
        case 6: // the choice flashes; then two players go on to the characters, one to the choice of level
            if (TimerOut()) {
                if (!Ram8(kModeCursor)) {
                    NextSubStep();
                    SetRam8(kTasksDone, 0);
                    SetRam16(kHelpText, 0);
                    PutWords(2);
                } else {
                    SetRam16(kTimer, 1);
                    if ((Ram16(kScrollX) & 0x7F) < 0x40) {
                        SetRam16(kSubStep, 0x0A);
                        TurnToCharacters();
                    }
                }
            }
            if (Ram16(kTimer) >= 0x0A) {
                SetBrightness(kModeColours[Ram8(kModeCursor) ? 1 : 0], Ram16(kTimer) & 1 ? kDim : kBright);
            }
            break;
        case 8:
            if (Ram8(kTasksDone) == 3) {
                SetRam16(kRoundStep, kScreenLevel);
                SetRam16(kSubStep, 0);
                SetRam8(kTasksDone, 0);
            }
            break;
        case 0x0A:
            if (TimerOut()) {
                NextSubStep();
                SetRam16(kTimer, 0x40);
                ClearLayers();
            }
            break;
        case 0x0C:
            if (BackdropGone()) {
                SetRam16(kRoundStep, kScreenCharacters);
                SetRam8(kChoosing, 3);
                for (int n = 0; n < 2; n++) {
                    SetRam8(Player(n) + kPlayerState, 2);
                    SetRam8(Player(n) + 0x84, 6);
                    SetRam8(Player(n) + 0x85, 0);
                    SetRam8(Player(n) + kComputer, 0);
                    SetRam8(Player(n) + 0xA0, 0);
                }
            }
            ScrollBackdrop();
            break;
        }
        ScrollBackdrop();
    }

    // 006CD2. The choice of level.
    void ShowLevelCursor() {
        for (int level = 0; level < 3; level++) {
            SetBrightness(kLevelColours[level], Ram8(kLevelCursor) == level ? kBright : kDim);
        }
    }
    void ChooseLevel() {
        switch (Ram16(kSubStep)) {
        case 0:
            NextSubStep();
            SetRam8(kTasksDone, 0);
            for (const uint32_t row : kLevelColours) {
                SetBrightness(row, kDim);
            }
            for (uint16_t n = 0; n < 3; n++) {
                if (!NewTask(static_cast<uint16_t>(0x1F00 + n))) {
                    break;
                }
            }
            break;
        case 2:
            if (Ram8(kTasksDone) == 7) {
                NextSubStep();
                SetRam8(kSeconds, kChoiceSeconds);
                SetRam8(kSecondFrames, kFramesASecond);
                Sound(kSoundLevelScreen);
                PutWords(1);
            }
            break;
        case 4: {
            const bool out = SecondsOut(kSeconds, kSecondFrames);
            const uint16_t pressed = out ? 0 : MenuPressed(true);
            if (out || (pressed & kButtons)) {
                NextSubStep();
                SetRam16(kTimer, 0x1E);
                Sound(kSoundChoose);
                Sound(kSoundLevel);
            } else {
                if (pressed & kStickLeft) {
                    if (Ram8(kLevelCursor) == 0) {
                        SetRam8(kLevelCursor, 0);
                    } else {
                        SetRam8(kLevelCursor, static_cast<uint8_t>(Ram8(kLevelCursor) - 1));
                        Sound(kSoundMove);
                    }
                }
                if (pressed & kStickRight) {
                    if (Ram8(kLevelCursor) + 1 >= 3) {
                        SetRam8(kLevelCursor, 2);
                    } else {
                        SetRam8(kLevelCursor, static_cast<uint8_t>(Ram8(kLevelCursor) + 1));
                        Sound(kSoundMove);
                    }
                }
            }
            ShowLevelCursor();
            ShowHelp(Ram8(kLevelCursor), 0xD4, 0x2E);
            break;
        }
        case 6:
            if (TimerOut()) {
                SetRam16(kTimer, 1);
                if ((Ram16(kScrollX) & 0x7F) < 0x40) {
                    NextSubStep();
                    SetRam8(Player(0) + kLevel, Ram8(kLevelCursor));
                    SetRam8(Player(1) + kLevel, Ram8(kLevelCursor));
                    TurnToCharacters();
                }
            }
            if (Ram16(kTimer) >= 0x0A) {
                SetBrightness(kLevelColours[Ram8(kLevelCursor) == 0 ? 0 : Ram8(kLevelCursor) == 1 ? 1 : 2], Ram16(kTimer) & 1 ? kDim : kBright);
            }
            break;
        case 8:
            if (TimerOut()) {
                NextSubStep();
                SetRam16(kTimer, 0x40);
                ClearLayers();
            }
            break;
        case 0x0A:
            if (BackdropGone()) {
                // The level sets how long the ladder is. (When a second player came in during the choice,
                // both choose characters.)
                const uint32_t stages = Ram8(kShortLadder) ? kLastStagesShort : kLastStages;
                SetRam16(kLastStage, Opcode16(stages + 2 * Ram8(kLevelCursor)));
                SetRam16(kRoundStep, Ram8(kModeCursor) ? kScreenCharacters : kScreenCharacter);
            }
            ScrollBackdrop();
            break;
        }
        ScrollBackdrop();
    }

    // The screen of characters comes up
    void DrawCharacterScreen() {
        draw_picture(4,0x902000);draw_picture(5,0x903000);
        draw_picture_wide(0x66,0x904000);draw_picture_wide(0x67,0x904400);
        draw_stage_number(0, kStageNumberAt);
    }
    void SetCharacterScreen() {
        SetRam16(kScrollX, 0);
        SetRam16(kScrollY, 0x300);
        SetRam16(kLayerOrder, 0x270E);
        SetRam16(kLayerSetting, 0x2641);
        SetRam8(kTasksDone, 0);
        SetRam16(kScreenSetting, kScreenPortraits);
        SetRam16(kTimer, 8);
    }
    // 00709E. The players' time to choose starts.
    void StartChoosing() {
        if (!TimerOut()) {
            return;
        }
        NextSubStep();
        for (const uint32_t character : kCharacters) {
            SetRam8(character + kChoiceSecondsLeft, kChoiceSeconds);
            SetRam8(character + kChoiceFrames, kFramesASecond);
            SetRam32(character + kChoiceStep, 0);
        }
        SetRam8(kHeld1, 0);
        SetRam8(kHeld2, 0);
        SetRam16(kScreenSetting, kScreenPlain);
        Sound(kSoundCharacters);
    }
    // 0070DE. The players choose. A player who comes in meanwhile is given a character to choose too, and
    // the screen becomes the two players' one.
    void ChoosingStep() {
        if (Ram8(kChoosing) != Ram8(kPlayersSeen)) {
            for (int n = 0; n < 2; n++) {
                if (!(Ram8(kPlayersSeen) & (1 << n))) {
                    SetRam8(kCharactersChosen, Ram8(kCharactersChosen) & ~(1 << n));
                    BringIn(n);
                }
            }
            SetRam16(kRoundStep, kScreenCharacters);
            SetRam8(kModeCursor, 1);
            SetRam8(kPlayersSeen, Ram8(kChoosing));
        }
        PlayersChoose();
    }
    // 007124. The choices are taken: on to the opponent's being shown, or with two people to the match.
    void TakeChoices() {
        if (!TimerOut()) {
            return;
        }
        for (int n = 0; n < 2; n++) {
            SetRam8(Player(n) + kCharacter, Ram8(kChosen[n]));
            SetRam8(kCharacters[n] + kCharacter, Ram8(kChosen[n]));
            SetRam8(Player(n) + 0xF6, Ram8(kChosen[n]));
        }
        SetRam16(kSubStep, 0);
        SetRam8(kVars + 0xAB, 0);
        if ((Ram8(kChoosing) | Ram8(kPlaying)) != 3) {
            SetRam16(kRoundStep, kScreenOpponent);
        } else {
            SetRam16(kMatchStep, static_cast<uint16_t>(Ram16(kMatchStep) + 2));
            SetRam16(kRoundStep, 0);
            SetRam8(kHeld1, 1);
            SetRam8(kHeld2, 1);
        }
    }
    // 007242 (and the end of 0073F2). With handicaps the levels' pictures come in; without, the choice is over.
    void StartHandicaps(bool handicaps, uint16_t stepWithout) {
        if (!handicaps) {
            SetRam16(kSubStep, stepWithout);
            return;
        }
        NextSubStep();
        SetRam8(kCharactersChosen, 0);
        SetRam8(kTasksDone, 0);
        for (int n = 0; n < 2; n++) {
            const uint32_t task = NewTask(static_cast<uint16_t>(0x2A00 + n));
            if (!task) {
                return;
            }
            SetRam32(task + 0x32, kChosen[n] + kHandicapChosen);
        }
    }
    // 007292. Their cursors come in, and the players' time to choose a level starts.
    void StartChoosingHandicaps() {
        if (Ram8(kTasksDone) != 3) {
            return;
        }
        NextSubStep();
        for (int n = 0; n < 2; n++) {
            const uint32_t task = NewTask(static_cast<uint16_t>(0x2A02 + n));
            if (!task) {
                return;
            }
            SetRam32(task + 0x32, kChosen[n] + kHandicapChosen);
        }
        for (int n = 0; n < 2; n++) {
            SetRam8(kCharacters[n] + kChoiceStep, static_cast<uint8_t>(Ram8(kCharacters[n] + kChoiceStep) + 2));
            SetRam8(kChosen[n] + kHandicapChosen, 2);
            SetRam8(kCharacters[n] + kChoiceSecondsLeft, kHandicapSeconds);
            SetRam8(kCharacters[n] + kChoiceFrames, kFramesASecond);
        }
    }
    // 007304
    void AfterHandicaps() {
        NextSubStep();
        SetRam16(kTimer, 0x3C);
        SetRam8(kTasksDone, 0);
    }
    // The steps the two players' screen and the screen for a player joining have alike
    void CharactersStep() {
        switch (Ram16(kSubStep)) {
        case 2: StartChoosing(); break;
        case 4: ChoosingStep(); break;
        case 8: StartChoosingHandicaps(); break;
        case 0x0A: PlayersChoose(); break;
        case 0x0C: AfterHandicaps(); break;
        case 0x0E: TakeChoices(); break;
        }
    }

    // 006FCC. One player chooses a character.
    void ChooseCharacter() {
        switch (Ram16(kSubStep)) {
        case 0:
            if (Ram8(kTasksDone) == kTasksGone && TimerOut()) {
                NextSubStep();
                SetCharacterScreen();
                DrawCharacterScreen();
                SetRam8(kCharactersChosen, Ram8(kPlaying) | (Ram8(kChoosing) ^ 3));
                SetRam8(kPlayersSeen, Ram8(kChoosing));
                for (int n = 0; n < 2; n++) {
                    if (Ram8(kChoosing) & (1 << n)) {
                        BringIn(n);
                    }
                }
            }
            break;
        case 2: StartChoosing(); break;
        case 4: ChoosingStep(); break;
        case 6: TakeChoices(); break;
        }
        ShowCharacters();
    }
    // 00718A. Two players choose their characters, and with handicaps their levels.
    void ChooseCharacters() {
        switch (Ram16(kSubStep)) {
        case 0:
            if (Ram8(kTasksDone) == kTasksGone && TimerOut()) {
                NextSubStep();
                SetCharacterScreen();
                DrawCharacterScreen();
                SetRam8(kCharactersChosen, 0);
                SetRam8(kPlayersSeen, Ram8(kChoosing));
                BringIn(0);
                BringIn(1);
            }
            break;
        case 6: StartHandicaps(Ram8(kHandicaps) != 0, 0x0C); break;
        default: CharactersStep(); break;
        }
        ShowCharacters();
    }
    // 007314. A player who joins a game that is on, or goes on after losing, chooses a character.
    void ChooseOnJoining() {
        const uint8_t in = Ram8(kPlaying) | Ram8(kChoosing);
        switch (Ram16(kSubStep)) {
        case 0:
            NextSubStep();
            SetCharacterScreen();
            SetRam8(kPortraitsStay, 1);
            PortraitFrames();
            DrawCharacterScreen();
            SetRam8(kCharactersChosen, Ram8(kPlaying) | (Ram8(kChoosing) ^ 3));
            SetRam8(kPlayersSeen, Ram8(kChoosing));
            for (int n = 0; n < 2; n++) {
                if (in & (1 << n)) {
                    BringIn(n);
                }
            }
            break;
        case 6: StartHandicaps(Ram8(kHandicaps) != 0 && in == 3, 0x0E); break;
        default: CharactersStep(); break;
        }
        ShowCharacters();
    }

    // 007AC6. The ladder is shown: against the first opponents the stage's place runs through the
    // characters and stops at the opponent; a button cuts it short.
    void LeaveForMatch() {
        SetRam16(kMatchStep, static_cast<uint16_t>(Ram16(kMatchStep) + 2));
        SetRam16(kRoundStep, 0);
        SetRam16(kSubStep, 0);
        SetRam8(kHeld1, 1);
        SetRam8(kHeld2, 1);
    }
    void ShowOpponentScreen() {
        switch (Ram16(kSubStep)) {
        case 0:
            NextSubStep();
            SetRam8(kTasksDone, 0);
            SetRam16(kTimer, 0x0A);
            SetRam8(kHeld1, 1);
            SetRam8(kHeld2, 1);
            SmallPortraitTasks();
            break;
        case 2:
            if (Ram8(kTasksDone) == kTasksGone && TimerOut()) {
                NextSubStep();
                SetRam16(kScrollX, 0);
                SetRam16(kScrollY, 0x300);
                SetRam8(kTasksDone, 0);
                SetRam8(kOpponentShowing, 1);
                SetRam16(kScreenSetting, kScreenPortraits);
                SetRam16(kTimer, 8);
                if (Ram16(kStage) != kLastStageNumber) {
                    BringBack(Ram8(kChoosing) & 1 ? 1 : 0);
                }
            }
            break;
        case 4:
            if (TimerOut()) {
                NextSubStep();
                SetRam16(kTimer, 0x78);
                DrawLadder(0);
                draw_stage_number((Ram16(kStage)==Ram16(kLastStage)?9u:Ram16(kStage))+1,kStageNumberAt);
                if (Ram8(kLevelCursor) && Ram16(kStage) == kLastStageNumber) {
                    SetRam16(kSubStep, 0x0A);
                }
                SetRam16(kScreenSetting, kScreenPlain);
            }
            break;
        case 6:
            if (AnyButton()) {
                Sound(kSoundChoose);
                LeaveForMatch();
                ShowStageOpponent(0);
            } else if (TimerOut()) {
                NextSubStep();
                SetRam16(kTimer, 0x78);
                Sound(kSoundChoose);
                ShowStageOpponent(0);
                Sound(kSoundDone);
            } else {
                SpinStage();
            }
            break;
        case 8:
            if (AnyButton() || TimerOut()) {
                LeaveForMatch();
                ShowStageOpponent(0);
            }
            break;
        case 0x0A: // (the last stage, but for the easy level: the whole ladder runs)
            if (AnyButton()) {
                Sound(kSoundChoose);
                LeaveForMatch();
                ShowLastOpponent();
            } else if (TimerOut()) {
                NextSubStep();
                SetRam16(kTimer, 0x78);
                Sound(kSoundChoose);
                ShowLastOpponent();
                Sound(kSoundDone);
            } else {
                SpinLadder();
            }
            break;
        case 0x0C:
            if (AnyButton() || TimerOut()) {
                LeaveForMatch();
                ShowLastOpponent();
            }
            break;
        }
        ShowCharacters();
    }

    // 0069E0
    void Choose() {
        switch (Ram16(kRoundStep)) {
        case kScreenMode: ChooseMode(); break;
        case kScreenLevel: ChooseLevel(); break;
        case kScreenCharacter: ChooseCharacter(); break;
        case kScreenCharacters: ChooseCharacters(); break;
        case kScreenJoining: ChooseOnJoining(); break;
        case kScreenOpponent: ShowOpponentScreen(); break;
        }
    }

    // 007452. The screen fades out, and the match is set up.
    void BeforeLeave() {
        switch (Ram16(kRoundStep)) {
        case 0:
            if (h.overlay(0,active_character)) {
                SetRam16(kRoundStep, 2);
                SetRam8(kVars + 0x8C, 0xFF);
            }
            break;
        case 2:
            if (Ram8(kVars + 0x290A)) {
                SetRam16(kRoundStep, 4);
                SetRam8(kCharacters[0] + kOn, 0);
                SetRam8(kCharacters[1] + kOn, 0);
                h.action(SelectionAction::end_fade);
                h.action(SelectionAction::reset_pools);
            }
            break;
        case 4:
            if (!Ram8(kVars + 0x8C)) {
                SetRam16(kGameStep, static_cast<uint16_t>(Ram16(kGameStep) + 2));
                SetRam16(kMatchStep, 0);
                SetRam16(kRoundStep, 0);
                SetRam16(kSubStep, 0);
                SetRam8(kVars + 0xAB, 0);
                SetRam8(kPlaying, Ram8(kPlaying) | Ram8(kChoosing));
                for (int n = 0; n < 2; n++) {
                    if (Ram8(kPlaying) & (1 << n)) {
                        SetRam8(Player(n) + kComputer, 0);
                    }
                }
                finish_source_interlude_scene(g,m);
                SetRam8(kChoosing, 0);
            }
            break;
        }
    }


 // USA179E4 and17BA8: original tilemap codecs (DATA, not opcode data).
 void draw_picture(unsigned index,uint32_t at){
  auto list=0xc0000u+((index&255u)<<7);
  for(unsigned column=0;column<8;++column)for(unsigned block=0;block<8;++block){
   auto from=0xc0a00u+unsigned(Ram16(list))*64;list+=2;
   for(unsigned line=0;line<4;++line)for(unsigned tile=0;tile<4;++tile){SetRam32(at+column*0x200+block*16+line*0x80+tile*4,Ram32(from));from+=4;}
  }
 }
 void draw_picture_wide(unsigned index,uint32_t at){
  auto from=0xc5880u+((index&255u)<<10);
  for(unsigned n=0;n<256;++n){SetRam16(at+n*4,Ram16(from+n*4)+0x5800);SetRam16(at+n*4+2,Ram16(from+n*4+2));}
 }
 void draw_stage_number(unsigned index,uint32_t at){
  auto table=0x8dc3cu+int16_t(Ram16(0x8dc3e));
  auto from=Ram32(table+uint32_t(int32_t(int8_t(index)))*16+4);
  const auto width=Ram16(from+2),height=Ram16(from+4);from+=6;
  for(unsigned row=0;row<=height;++row)for(unsigned col=0;col<=width;++col){SetRam16(at+row*0x40+col*4,Ram16(from)+0x5800);SetRam16(at+row*0x40+col*4+2,Ram16(from+2));from+=4;}
 }
 void selection_palettes(){
  const uint16_t index=Ram16(g+0x128),rot=uint16_t((index>>7)|(index<<9));
  const uint32_t sources[]={0xa12c0u+int16_t(rot),0xa1ec0u,0xa4ec0u+int16_t(rot),0xa86c0u+int16_t(rot)};
  for(unsigned block=0;block<4;++block)for(unsigned n=0;n<0x400;n+=4)SetRam32(0x90c000+block*0x400+n,Ram32(sources[block]+n)|0xf000f000u);
 }
 void frame_tail(){h.action(SelectionAction::first_pool);h.action(SelectionAction::third_pool);h.action(SelectionAction::objects);}
 SelectionResult run(){
  // Normal9FEE caller has just run61CA, which selects the overlay record.
  active_character=g+0x2900;
  switch(Ram16(g+8)){
  case 0:
   SetRam16(g+8,2);SetRam16(g+0xc,0);
   if(Ram8(g+0xa5)==3||Ram8(g+0xad))SetRam16(g+0xc,8);
   else{if(!Ram8(g+0xa5))SetRam16(g+0xc,2);SetRam8(kLevelCursor,1);SetRam8(g+0x3a1,1);SetRam8(g+0x7a1,1);SetRam16(g+0x114,Ram8(g+0xbf)?7:8);}
   return SelectionResult::wait_object_clear;
  case 2:{
   const auto screen=Ram16(g+0xc),step=Ram16(g+0x10);
   if((screen&1)||screen>10||(step&1))return SelectionResult::unsupported;
   const unsigned last[]={12,10,6,14,14,12};if(step>last[screen/2])return SelectionResult::unsupported;
   Choose();break;
  }
  case 4:if((Ram16(g+0xc)&1)||Ram16(g+0xc)>4)return SelectionResult::unsupported;BeforeLeave();break;
  default:return SelectionResult::unsupported;
  }
  frame_tail();return SelectionResult::complete;
 }
 // Caller resumes this only after original2CEE4's object-clear continuation.
 SelectionResult resume_after_object_clear(){
  h.action(SelectionAction::reset_pools);
  for(auto off:{0x131,-0x7376,-0x7375,-0x7370})SetRam8(g+off,0);
  SetRam8(g+0x8f,1);SetRam8(g+0xbd,1);
  for(auto off:{0x210,0x214,0x250,0x290,0x22,0x24,0x26,0x2a})SetRam16(g+off,0);
  SetRam16(g+0x254,0x300);SetRam16(g+0x294,0x700);SetRam16(g+0x28,0x300);SetRam16(g+0x2c,0x700);
  SetRam16(g+0x20,0x90c0);SetRam16(g+0x32,0x1b00);SetRam16(g+0x3a,0x2461);
  h.action(SelectionAction::scene_actor_setup);
  SetRam8(kChosen[0],Ram8(g+0x382));SetRam8(kChosen[1],Ram8(g+0x782));
  if(!Ram8(g+0xa6)){queue_sound_request(g,m,{0xff05,0,0});queue_sound_request(g,m,{0xe,0,0},true);}
  if(Ram16(g+0xc)!=8){DrawBackdrop();}SetRam16(g+0x128,2);selection_palettes();frame_tail();return SelectionResult::complete;
 }

};
} // namespace puzzle_fighter
