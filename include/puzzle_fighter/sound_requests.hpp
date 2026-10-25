// SPDX-License-Identifier: GPL-3.0-only
// USA2B118..2B342 queue/command control and2BC58/2BD10 music selection.
#pragma once
#include <array>
#include <cstdint>
namespace puzzle_fighter {
struct SourceSoundCommand {std::uint32_t number=0,pan=0,extra=0;};
struct SourceSoundIo {std::uint32_t address;std::uint8_t value;};
struct SourceSoundDispatch {std::array<SourceSoundIo,14>writes{};unsigned count=0;bool sent=false;SourceSoundCommand command;};
template<class M> void clear_sound_requests(std::uint32_t g,M&m){for(unsigned n=0;n<0x1000;n+=4)m.write_long(g+0x47a0+n,0);m.write_long(g+0x57a0,0);}
// This boundary is original2B1CC: a committed request. The optional gate is
// original2B1C0, used by normal song wrappers; control wrappers bypass it.
template<class M> bool queue_sound_request(std::uint32_t g,M&m,SourceSoundCommand request,bool gated=false){
 if(gated&&!m.read_byte(g+0x99)&&m.read_byte(g+0x83))return false;
 const auto index=m.read_word(g+0x57a0);const auto address=g+0x47a0+std::uint32_t(index&0x8000?int(index)-65536:int(index));
 m.write_long(address,request.number);m.write_long(address+4,request.pan);m.write_long(address+8,request.extra);m.write_word(g+0x57a0,std::uint16_t((index+16)&0xff0));return true;
}
template<class M> SourceSoundDispatch dispatch_sound_request(std::uint32_t g,M&m,bool device_ready){
 SourceSoundDispatch result;auto write=[&](std::uint32_t address,std::uint8_t value){result.writes[result.count++]={address,value};};
 m.write_word(g-0x6cae,0);write(0x619ffd,m.read_byte(g+0x9a)?0xff:0);
 const auto index=m.read_word(g+0x57a2);if(index==m.read_word(g+0x57a0)||!device_ready)return result;
 const auto address=g+0x47a0+std::uint32_t(index&0x8000?int(index)-65536:int(index));
 constexpr unsigned ports[]={0x618007,0x618009,0x618001,0x618003,0x618005,0x61800d,0x61800f,0x618011,0x618017,0x618019,0x618013,0x618015};
 for(unsigned n=0;n<12;++n){write(ports[n],m.read_byte(address+n));}
 write(0x61801f,0);
 result.command={m.read_long(address),m.read_long(address+4),m.read_long(address+8)};result.sent=true;m.write_word(g+0x57a2,std::uint16_t((index+16)&0xff0));return result;
}
template<class M> void source_music_tempo(std::uint32_t g,M&m){
 const auto left=m.read_word(g-0x6cb2);if(left){m.write_word(g-0x6cb2,left-1);return;}
 auto danger=m.read_byte(g+0x3d1);const auto other=m.read_byte(g+0x7d1);const auto signed_byte=[](std::uint8_t v){return v&0x80?int(v)-256:int(v);};
 if(signed_byte(other)>signed_byte(danger))danger=other;
 if(danger==m.read_byte(g-0x6cb0))return;
 m.write_byte(g-0x6cb0,danger);m.write_word(g-0x6cb2,180);
 const auto index=std::uint16_t(std::uint16_t(danger*2)+std::uint16_t(m.read_word(g+0x100)*4));const auto offset=std::uint32_t(index&0x8000?int(index)-65536:int(index));
 queue_sound_request(g,m,{(std::uint32_t(m.opcode_word(0x2b344+offset))<<16)|0xff08u,0,0});
}
template<class M> void start_source_round_music(std::uint32_t g,M&m){
 if(m.read_byte(g+0xa6))return;
 queue_sound_request(g,m,{0xff05,0,0});const auto index=m.read_word(g+0x100);queue_sound_request(g,m,{m.opcode_word(0x2bc7a+std::uint32_t(index&0x8000?int(index)-65536:int(index))),0,0},true);
}
// Source2BD10 belongs to the later post-match scene. The native prototype's
// per-round use of this cue is a separately documented enhancement.
template<class M> void source_post_match_jingle(std::uint32_t g,M&m){if(m.read_byte(g+0xa6))return;queue_sound_request(g,m,{0xff05,0,0});queue_sound_request(g,m,{0x10,0,0},true);}
} // namespace puzzle_fighter
