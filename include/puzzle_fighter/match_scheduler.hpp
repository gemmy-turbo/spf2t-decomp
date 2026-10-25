// SPDX-License-Identifier: GPL-3.0-only
// USA normal-match caller9FEA..A01A/A660..A6C8 and logical IRQ1130..11E4.
// Native continuations only: no instruction addresses or CPU register state.
#pragma once
#include "frame_clocks.hpp"
#include <cstdint>
namespace puzzle_fighter {
enum class MatchPhase : std::uint16_t { introduction=0, playing=2, round_result=4, round_bonus=6 };
enum class MatchPhaseFlow { complete, clear_screen, unsupported };
enum class MatchRunResult { waiting_for_irq, unsupported };
enum class MatchContinuation { begin_frame, clear_first_wait, clear_second_wait, frame_wait, unsupported };
enum class MatchUnsupported { none, outer_scene, phase, phase_callback, phase_resume, service_callback };

// 2CEE4 fills the four source video ranges, then2CF34 invalidates both object
// banks across two IRQ waits. Hardware object RAM is an explicit host writer.
struct MatchObjectClear {
 enum class State { idle, after_first_irq, after_second_irq, complete };
 State state=State::idle;
 template<class M,class H> void invalidate(std::uint32_t g,M&m,H&h){
  for(unsigned n=0;n<2;++n)h.object_buffer_write(0x708020u+n*4,0x80008000u);
  if(!m.read_byte(g+0x95)){m.write_byte(g+0x94,m.read_byte(g+0x94)^1);m.write_byte(g+0x95,0xff);}
 }
 template<class M,class H> void start(std::uint32_t g,M&m,H&h){
  // 2CFDC writes downward in 64-byte blocks. Preserve source write order.
  auto fill=[&](std::uint32_t end,unsigned blocks,std::uint32_t value){for(unsigned i=0;i<blocks;++i)for(unsigned n=0;n<16;++n){end-=4;m.write_long(end,value);}};
  fill(0x904000,256,0x00200000);fill(0x908000,256,0x58200000);fill(0x90c000,256,0x1cc00000);fill(0x910000,128,0);
  invalidate(g,m,h);state=State::after_first_irq;
 }
 // True means the original helper has returned; false requests one more IRQ.
 template<class M,class H> bool resume_after_irq(std::uint32_t g,M&m,H&h){
  if(state==State::after_second_irq){state=State::complete;return true;}
  if(state!=State::after_first_irq)return state==State::complete;
  for(unsigned n=0;n<2;++n)h.object_buffer_write(0x708020u+n*4,0x80008000u);
  if(m.read_byte(g+0x95)){state=State::complete;return true;}
  m.write_byte(g+0x94,m.read_byte(g+0x94)^1);m.write_byte(g+0x95,0xff);state=State::after_second_irq;return false;
 }
};

// 1760..17BA: source cabinet-output bits; actual hardware output is outside
// this pure RAM update. Coin counters themselves remain IRQ service callbacks.
template<class M> void match_output_bits(std::uint32_t g,M&m){
 auto value=m.read_byte(g+0x6b);
 if(m.read_byte(g+0x9c)){m.write_byte(g+0x6b,value&0xcf);return;}
 auto count=std::uint8_t(m.read_byte(g+0x76)+m.read_byte(g+0x73));
 value=count<=9?std::uint8_t(value|0x10):std::uint8_t(value&0xef);
 bool second=false;
 if(m.read_byte(g+0x9f)){
  count=std::uint8_t(m.read_byte(g+0x76)+m.read_byte(g+0x73));
  if(m.read_byte(g+0x9e))count=std::uint8_t(m.read_byte(g+0x7e)+m.read_byte(g+0x7b));
  second=count<=9;
 }
 value=second?std::uint8_t(value|0x20):std::uint8_t(value&0xdf);m.write_byte(g+0x6b,value);
}

// A810..A82B. Call at the start of the playing phase, before winner
// observation. False is the source8A interlude gate: do not observe a winner.
// These clocks advance even if10F will suppress the later shared work block.
template<class M> bool begin_match_play_phase(std::uint32_t g,M&m){
 if(m.read_byte(g+0x8a)){m.write_byte(g+0xbd,0xff);m.write_byte(g+0x10e,0xff);return false;}
 m.write_byte(g+0x10a,m.read_byte(g+0x10a)-1);
 if(m.read_byte(g+0x10a)&0x80){
  m.write_byte(g+0x10a,59);m.write_byte(g+0x109,m.read_byte(g+0x109)+1);
  if(m.read_byte(g+0x109)==60){m.write_byte(g+0x109,0);m.write_byte(g+0x108,m.read_byte(g+0x108)+1);}
 }
 m.write_word(g-0x7336,m.read_word(g-0x7336)+1);return true;
}

// BDDA third branch (word+13E==4), BE5A..BEF8. This is the reachable
// special overlay/clear interlude; the original character bodies do not yield.
// request_interlude() is source6634(index3), interlude_cue() is2BD70,
// after_interlude_scene() is BA44's scene-selection tail, owned by scene reset.
struct MatchSpecialInterlude {
 template<class M,class H> MatchPhaseFlow step(std::uint32_t g,M&m,H&h){
  if(m.read_word(g+0x13e)!=4)return MatchPhaseFlow::unsupported;
  switch(m.read_word(g+0x14)){
  case 0:
   m.write_word(g+0x14,2);m.write_byte(g+0x10f,0);m.write_byte(g+0xbd,1);m.write_byte(g+0x8f,1);m.write_byte(g+0x139,0xff);m.write_byte(g+0x152,0xff);m.write_byte(g+0x16,0x1e);return MatchPhaseFlow::complete;
  case 2:
   m.write_byte(g+0x16,m.read_byte(g+0x16)-1);
   if((m.read_byte(g+0x16)&0x80)&&h.request_interlude()){m.write_word(g+0x14,4);m.write_byte(g+0x8a,0xff);h.interlude_cue();}
   return MatchPhaseFlow::complete;
  case 4:
   if(m.read_byte(g+0x8a))return MatchPhaseFlow::complete;
   m.write_word(g+8,10);m.write_word(g+0xc,6);m.write_word(g+0xa,0);m.write_word(g+0xe,0);m.write_long(g+0x10,0);m.write_long(g+0x14,0);m.write_word(g+0x13e,0);m.write_byte(g+0x10f,0);m.write_byte(g+0x14d,0);return MatchPhaseFlow::clear_screen;
  default:return MatchPhaseFlow::unsupported;
  }
 }
 template<class M,class H> bool after_clear(std::uint32_t g,M&m,H&h){m.write_word(g+0x20,0x9280);m.write_word(g+0x48,0x9280);return h.after_interlude_scene();}
};

struct MatchScheduler {
 MatchContinuation continuation=MatchContinuation::begin_frame;
 MatchUnsupported unsupported=MatchUnsupported::none;
 MatchObjectClear object_clear;
 bool irq_ready=true;
 std::uint64_t completed_main_cycles=0;
 MatchRunResult reject(MatchUnsupported why){unsupported=why;continuation=MatchContinuation::unsupported;return MatchRunResult::unsupported;}
 // Hooks are the named original units, not a source-address dispatcher.
 // phase() owns the already-recovered phase body, ending exactly at its RTS,
 // or immediately before2CEE4 when it returns clear_screen. phase_after_clear
 // resumes the caller of2CEE4 and must return true only at that phase's RTS.
 template<class M,class H> MatchRunResult resume(std::uint32_t g,M&m,H&h){
  if(continuation==MatchContinuation::unsupported)return MatchRunResult::unsupported;
  if(!irq_ready)return MatchRunResult::waiting_for_irq;
  if(continuation==MatchContinuation::clear_first_wait||continuation==MatchContinuation::clear_second_wait){
   if(!object_clear.resume_after_irq(g,m,h)){continuation=MatchContinuation::clear_second_wait;irq_ready=false;return MatchRunResult::waiting_for_irq;}
   if(!h.phase_after_clear())return reject(MatchUnsupported::phase_resume);
  }else{
   continuation=MatchContinuation::begin_frame;
   main_frame_begin(g,m);h.copy_inputs();
   if(!h.overlay())return reject(MatchUnsupported::service_callback);
   // A01E/A1DC/A24A: standalone active-match scene only. Menu, ladder,
   // continue and boot entry bodies are explicitly outside this scheduler.
   if(m.read_word(g)!=2||m.read_word(g+4)!=4||m.read_word(g+8)!=4)return reject(MatchUnsupported::outer_scene);
   const auto phase=m.read_word(g+0xc);if(phase>6||(phase&1))return reject(MatchUnsupported::phase);
   const auto flow=h.phase(static_cast<MatchPhase>(phase));
   if(flow==MatchPhaseFlow::unsupported)return reject(MatchUnsupported::phase_callback);
   if(flow==MatchPhaseFlow::clear_screen){object_clear.start(g,m,h);continuation=MatchContinuation::clear_first_wait;irq_ready=false;return MatchRunResult::waiting_for_irq;}
  }
  // Gate is sampled AFTER the phase. No host intro/play condition substitutes
  // for10E; a stopped turn still runs the source BEFA partial-score work.
  if(!m.read_byte(g+0x10f)){
   m.write_word(g+0xb4,m.read_word(g+0xb4)+1);
   if(m.read_byte(g+0x10e))h.stopped_turns();else h.turns();
   h.reactions();h.characters();h.first_pool();h.second_pool();h.third_pool();
   h.music_control();h.stage();
   if(m.read_byte(g+0x124))m.write_byte(g+0x124,m.read_byte(g+0x124)-1);
   h.gem_pictures();h.object_packets();
   // A6C2->13A0 is RTS, not another yield.
  }
  match_output_bits(g,m);
  if(!h.main_accounting())return reject(MatchUnsupported::service_callback);
  ++completed_main_cycles;continuation=MatchContinuation::frame_wait;irq_ready=false;
  return MatchRunResult::waiting_for_irq;
 }
 // The host delivers one logical IRQ. Platform writes are explicit callbacks;
 // this does not emulate the old16-entry coroutine kernel or board interrupts.
 // Return false for an unbound active service; the host must surface it.
 template<class M,class H> bool irq(std::uint32_t g,M&m,H&h){
  h.irq_write(0x8040e1,m.read_byte(g+0x94),1);
  if(m.read_byte(g+0xa9)){m.write_byte(g+0xa9,0);m.write_word(g+0x18,0x9000);m.write_word(g+0x1a,0x9040);m.write_word(g+0x1c,0x9080);m.write_word(g+0x1e,0x90e0);}
  for(unsigned n=0;n<4;++n){h.irq_write(0x800102+n*2,m.read_word(g+0x40+n*2),2);m.write_word(g+0x40+n*2,m.read_word(g+0x18+n*2));}
  h.irq_write(0x400000,m.read_word(g+0x36),2);h.irq_write(0x400004,m.read_word(g+0x3a),2);h.irq_write(0x400008,m.read_word(g+0x3c),2);h.irq_write(0x40000a,m.read_word(g+0x3e),2);
  const auto video_control=std::uint16_t((unsigned(m.read_byte(g+0x96))<<8)|m.read_word(g+0x30));
  h.irq_write(0x800122,video_control,2);h.irq_write(0x400002,(video_control&0x8000)?0x808e:0x807d,2);h.irq_write(0x800166,m.read_word(g+0x32),2);
  h.irq_video_input(); //13D0..149E raw poll and scroll latches, before clocks.
  if(!h.irq_panel_service()){reject(MatchUnsupported::service_callback);return false;}
  h.irq_write(0x800170,m.read_word(g+0x34),2);h.irq_write(0x80010a,m.read_word(g+0x48),2);m.write_word(g+0x48,m.read_word(g+0x20));
  h.irq_sound_queue(); //2B118, before1550/16EE and the clock increment.
  if(!h.irq_coin_accounting()||!h.irq_coin_requests()){reject(MatchUnsupported::service_callback);return false;}
  m.write_byte(g+0x85,0);
  if(!m.read_byte(g+0x80)&&!h.irq_wrap_statistics()){reject(MatchUnsupported::service_callback);return false;}
  interrupt_clock_increment(g,m);m.write_byte(g-0x7b9c,0xff);m.write_byte(g+0x84,0);m.write_byte(g+0x95,0);
  irq_ready=true;return true;
 }
};
} // namespace puzzle_fighter
