// SPDX-License-Identifier: GPL-3.0-only
// USA2716..2AA8: idle credit gate, paid title, Start acceptance and job handoff.
// Finite source continuations; no CPU or generic job/script interpreter.
#pragma once
#include "scene_reset.hpp"
#include <cstdint>
#include <stdexcept>
namespace puzzle_fighter {
enum class SourceIdleStartAction {
 credit_cue,                 //2BC0C: ungatedFF05 then0090/pan1000
 title_setup,title_picture,  //2DE8,2D32: real title source bodies
 show_credit_totals,         //54D8
 prompt,                    //5670; argument is exact source text index
 first_pool,objects,        //16A3A,1495A, in that order
 persist_operator_counter,  //1E6F4; globals already contain count/address/data
 stop_title_child,          //12CC slot20
 start_paid_title,          //1288 slot60, entry27B6
 start_credit_job,          //1288 slot80, entry98D0
 start_arcade_job,          //1288 slotA0, entry9FB8 (notA03C directly)
 retire_current_job         //12AA: never execute the old caller tail afterward
};
enum class SourceIdleStartResult {complete,wait_irq,clear_screen,retired};
// 9FB8..9FDE: newly scheduled arcade job initialization. Its next operation
// is one mandatory IRQ wait; only then execute9FEA main prefix andA03C.
template<class M> void initialize_source_arcade_job_before_irq(std::uint32_t g,M&m){
 for(unsigned o=0x22;o<=0x2c;o+=2)m.write_word(g+o,0);
 m.write_byte(g+0x4781,0);m.write_byte(g+0x4782,0);m.write_byte(g+0x8f,1);
}
// 25C6..260A: completed idle fade advances its parent scene and starts the
// full2CEE4 clear. Caller must run MatchObjectClear after this preparation.
template<class M> void prepare_source_idle_transition_clear(std::uint32_t g,M&m){
 m.write_word(g,m.read_word(g)+2);
 for(unsigned o:{4u,8u,0xcu,0x10u,0x14u})m.write_word(g+o,0);
 for(unsigned o=0x22;o<=0x2c;o+=2)m.write_word(g+o,0);
 m.write_word(g+0x32,0x1b0e);m.write_word(g+0x3a,0x2461);clear_scene_records(g,m,true);
}
// 2610: end of attract cycle. Unlike2156 this retains A8, seed, credits,
// palette flags and the rest of the cabinet/scene state.
template<class M> void restart_source_idle_cycle(std::uint32_t g,M&m){
 m.write_byte(g-0x7398,m.read_byte(g-0x7398)+2);
 for(unsigned o=0;o<0x18;o+=4)m.write_long(g+o,0);
}
// Every action requires the named original native body or an explicit throw.
// clear_screen means caller starts/resumes MatchObjectClear2CEE4, then invokes
// resume_after_clear(). wait_irq from paid_title_begin() is exactly one IRQ.
template<class M,class H> struct SourceIdleStart {
 M&m;H&h;std::uint32_t g;
 enum class Pending {none,paid_after_clear,arcade_after_clear,paid_entry_irq};
 Pending pending=Pending::none;
 unsigned b(unsigned o){return m.read_byte(g+o);}unsigned w(unsigned o){return m.read_word(g+o);}
 void byte(unsigned o,unsigned v){m.write_byte(g+o,std::uint8_t(v));}void word(unsigned o,unsigned v){m.write_word(g+o,std::uint16_t(v));}
 void action(SourceIdleStartAction a,unsigned v=0){h.action(a,v);}
 void profile(){if(w(0x90)!=2||b(0x86)||b(0xce))throw std::runtime_error("Unbound source idle cabinet profile");}
 void available(){profile();if(pending!=Pending::none)throw std::runtime_error("Source idle continuation pending");}
 void clear_steps(){for(unsigned o=0;o<0x18;o+=4)m.write_long(g+o,0);}
 void tails(){action(SourceIdleStartAction::first_pool);action(SourceIdleStartAction::objects);}
 unsigned start_edge(){return (b(0x60)&~b(0x61))&3;}
 // 1E370: free-play accounting is active even with86=0. Persist is an
 // explicit synchronous EEPROM adapter boundary; paid play returns immediately.
 void start_accounting(){
  if(!b(0x9c))return;
  const auto total=std::uint32_t(m.read_long(g+0xc8)+1);m.write_long(g+0xc8,total);
  if(total==m.read_long(g-0x7008))return;
  m.write_long(g-0x7008,total);m.write_word(g-0x6f6c,1);m.write_word(g-0x6f68,0x68);
  action(SourceIdleStartAction::persist_operator_counter);
 }
 // 278E: no edge means existingA8 and credits are left unchanged.
 void accept_free_start_edge(){
  auto edge=start_edge();if(!edge)return;if(b(0xa5)==3)edge=3;
  byte(0xa8,b(0xa8)|edge);byte(0x76,0);byte(0x7e,0);
 }
 // 28D6: X is explicitly clear. Preserve the byte/decimal correction and
 // borrow test, rather than translating BCD balances to host integers.
 void accept_paid_start(unsigned side,unsigned credit_offset){
  if(b(0xa8)&(1u<<side))return;
  const unsigned have=b(credit_offset+6),price=std::uint8_t(1+b(0x9d));
  int value=int(have)-int(price);if(int(have&15)-int(price&15)<0)value-=6;
  if(value<0)return;
  byte(credit_offset+6,unsigned(value));byte(0xa8,b(0xa8)|(1u<<side));
  if(b(0xa5)==3)byte(0xa8,3);
 }
 // 2900..2982, also used by the free-play branch of2716.
 SourceIdleStartResult accepted_start(){
  if(!b(0xa8)){tails();return SourceIdleStartResult::complete;}
  clear_scene_records(g,m,true);
  byte(0x4781,0);byte(0x4782,0);clear_steps();
  for(unsigned o:{0x83u,0x8du,0x8eu})byte(o,0);
  byte(0x8f,1);byte(0xbd,1);byte(0x381,0);byte(0x781,1);
  // Exact source order: P2 accounting before P1; both may incrementC8.
  if(b(0xa8)&2){byte(0x780,2);start_accounting();}
  if(b(0xa8)&1){byte(0x380,2);start_accounting();}
  byte(0xa7,1);pending=Pending::arcade_after_clear;return SourceIdleStartResult::clear_screen;
 }
 SourceIdleStartResult idle_credit_gate(){
  available();
  if(b(0x9c)){accept_free_start_edge();if(!b(0xa8))return SourceIdleStartResult::complete;return accepted_start();}
  if(!(b(0x76)|b(0x71)|b(0x7e)|b(0x79)))return SourceIdleStartResult::complete;
  clear_steps();pending=Pending::paid_after_clear;return SourceIdleStartResult::clear_screen;
 }
 SourceIdleStartResult resume_after_clear(){
  profile();const auto was=pending;
  if(was!=Pending::paid_after_clear&&was!=Pending::arcade_after_clear)throw std::runtime_error("No source idle clear pending");
  pending=Pending::none;
  if(was==Pending::paid_after_clear){clear_scene_records(g,m,true);action(SourceIdleStartAction::credit_cue);action(SourceIdleStartAction::stop_title_child);action(SourceIdleStartAction::start_paid_title);}
  else{action(SourceIdleStartAction::stop_title_child);action(SourceIdleStartAction::start_credit_job);action(SourceIdleStartAction::start_arcade_job);}
  action(SourceIdleStartAction::retire_current_job);return SourceIdleStartResult::retired;
 }
 SourceIdleStartResult paid_title_begin(){
  available();for(unsigned o:{0x4781u,0x4782u,0x8fu,0x8du,0xacu,0xadu,0xabu,0xbdu,0xd8u})byte(o,0);
  byte(0x83,1);word(0x20,0x92a0);word(0x48,0x92a0);pending=Pending::paid_entry_irq;return SourceIdleStartResult::wait_irq;
 }
 SourceIdleStartResult resume_after_irq(){
  profile();if(pending!=Pending::paid_entry_irq)throw std::runtime_error("No source paid-title IRQ pending");pending=Pending::none;
  word(0x22,0);word(0x24,0xf0);for(unsigned o:{0x26u,0x28u,0x2au,0x2cu})word(o,0);
  clear_scene_records(g,m,true);return SourceIdleStartResult::complete;
 }
 // One281A dispatcher call only. Caller owns1760,29C4,one RNG draw,IRQ tail.
 // A retirement/clear return suspends that tail exactly where the source does.
 SourceIdleStartResult paid_title_tick(){
  available();switch(w(0)){
  case 0:word(0,2);m.write_long(g+0x14,0);action(SourceIdleStartAction::title_setup);action(SourceIdleStartAction::title_picture);return SourceIdleStartResult::complete;
  case 2:word(0,4);if(!b(0x9c))action(SourceIdleStartAction::show_credit_totals);tails();return SourceIdleStartResult::complete;
  case 4:{word(0x20,0x90c0);const auto edge=start_edge();
   // Source priority is P2 even when it cannot afford the cost. P1 does not
   // get a fallback attempt in the same simultaneous-edge frame.
   if(edge&2)accept_paid_start(1,b(0x9e)?0x78:0x70);
   else if(edge&1)accept_paid_start(0,0x70);
   return accepted_start();}
  default:throw std::runtime_error("Unsupported source paid-title state");
  }
 }
 // 29C4..2AA8: text selection, not a timeout. Exact low-byte arithmetic and
 // P2-before-P1 draw order are retained; prompt callback implements5670.
 void paid_title_prompts(){
  available();auto draw=[&](unsigned index){action(SourceIdleStartAction::prompt,index);};
  if(b(0x9c)){draw(12);return;}
  if(!b(0x9d)){
   if(!b(0x9e)){draw(b(0x76)?0x28:unsigned(std::uint8_t(b(0x72)-b(0x71)))+0x1f);return;}
   for(int side=1;side>=0;--side){const auto c=0x70+unsigned(side)*8;unsigned index=0x29;
    if(!b(c+6)){auto ratio=b(c+2);const auto partial=b(c+1);if(partial){ratio=std::uint8_t(ratio-partial);ratio=std::uint8_t(ratio*2);index=std::uint8_t(0x3b+ratio);}else{ratio=std::uint8_t(ratio*2);index=std::uint8_t(0x29+ratio);index|=(b(0x80)&0x10)<<3;}}
    draw(std::uint8_t(index+unsigned(side)));
   }return;
  }
  if(!b(0x9e)){if(b(0x76)>1)draw(0x28);else{draw(0x8d);draw(0x20);}return;}
  for(int side=1;side>=0;--side){const auto credits=b(0x76+unsigned(side)*8);unsigned index=0x29;
   if(credits<=1){index=credits?0x3d:(0x2d|((b(0x80)&0x10)<<3));}draw(index+unsigned(side));}
 }
};
} // namespace puzzle_fighter
