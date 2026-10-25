// SPDX-License-Identifier: GPL-3.0-only
// USA2AE6 two-CPU attract and37F0 how-to gameplay scene controllers.
#pragma once
#include "source_attract_players.hpp"
#include "scene_reset.hpp"
#include "scene_actor_tasks.hpp"
#include "actor_tasks.hpp"
#include <stdexcept>
namespace puzzle_fighter {
enum class SourceAttractAction {
 idle_credit_marker,initialize_players,cpu_start_skill,cpu_match_level,
 round_music,stop_music,reset_music,tutorial_music,scene_palettes,tutorial_palettes,
 demo_text,stage_setup,round_labels,fade_in,text_page,background_page,
 turns,stopped_turns,reactions,characters,first_pool,second_pool,third_pool,
 music_control,stage_tick,gem_pictures,objects
};
enum class SourceAttractResult {complete,clear_screen};
enum class SourceTutorialMotion {enter,face_left,face_right,gesture_2,gesture_3,gesture_4,gesture_5,jump_6,jump_7,exit};
template<class M> void clear_source_tutorial_fields(std::uint32_t g,M&m){
 for(unsigned side=0;side<2;++side){const auto field=g+0x2b00+side*0x200,mirror=g+0x2f10+side*0x200;
  for(unsigned row=0;row<14;++row){m.write_long(field+row*16,0x00ff0000);m.write_long(field+row*16+4,0);m.write_long(field+row*16+8,0);m.write_long(field+row*16+12,0xff);}
  for(unsigned row=0;row<13;++row){m.write_long(mirror+row*16,0x00ff0000);m.write_long(mirror+row*16+4,0);m.write_long(mirror+row*16+8,0);m.write_long(mirror+row*16+12,0xff);}
  for(unsigned n=0;n<4;++n){m.write_long(field+0xe0+n*4,0x00ff00ff);m.write_long(mirror+0xd0+n*4,0x00ff00ff);}
 }
}
template<class M,class H> struct SourceHowToSequence {
 M&m;H&h;std::uint32_t g;unsigned phase=0;bool entered=false;
 void byte(unsigned o,unsigned v){m.write_byte(g+o,v);}void word(unsigned o,unsigned v){m.write_word(g+o,v);}
 void action(SourceAttractAction a,unsigned x=0,unsigned y=0){h.action(a,x,y);}
 void reset(){phase=0;entered=false;}
 // Actor motion is a finite presentation state, not an animation VM. The
 // delivered ActorTasks helper reads the existing original animation frames.
 void motion(SourceTutorialMotion motion){
  const auto a=g+0xb00;auto quiet=[](std::uint32_t,std::uint32_t,std::uint32_t){};ActorTasks<M,decltype(quiet)>v{m,quiet,g};
  unsigned animation=0;for(unsigned o:{0x20u,0x24u,0x28u,0x2cu})m.write_long(a+o,0);m.write_byte(a+0xb,0);m.write_word(a+0xe,0);
  switch(motion){
  case SourceTutorialMotion::enter:m.write_word(a+0x10,0x180);m.write_word(a+0x14,0x40);m.write_word(a+0xd6,0x40);m.write_long(a+0x20,0xfffd0000);m.write_word(a+0xe,0x40);break;
  case SourceTutorialMotion::face_left:animation=1;break;
  case SourceTutorialMotion::face_right:m.write_byte(a+0xb,1);animation=1;break;
  case SourceTutorialMotion::gesture_2:animation=2;break;
  case SourceTutorialMotion::gesture_3:animation=3;break;
  case SourceTutorialMotion::gesture_4:animation=4;break;
  case SourceTutorialMotion::gesture_5:animation=5;break;
  case SourceTutorialMotion::jump_6:case SourceTutorialMotion::jump_7:animation=motion==SourceTutorialMotion::jump_6?6:7;m.write_long(a+0x24,0x5a000);m.write_long(a+0x2c,0xffffa000);m.write_word(a+0xe,0x1f);break;
  case SourceTutorialMotion::exit:m.write_byte(a+0xb,1);m.write_long(a+0x20,0x30000);m.write_word(a+0xe,0x4f);break;
  }v.init(a,animation,0x7abb4);
 }
 void place_pair(unsigned x,unsigned y){word(0x310,x);word(0x314,y);word(0x542,0);m.write_word(g-0x7382,y);m.write_word(g-0x7380,0);}
 void board_pattern(unsigned index){
  byte(0x566,1);byte(0x48e,1);const auto source=0x3f08+index*336;
  for(unsigned row=0;row<14;++row)for(unsigned n=0;n<3;++n){m.write_long(g+0x2f02+row*16+n*4,m.read_long(source+row*12+0xa8+n*4));m.write_long(g+0x2b02+row*16+n*4,m.read_long(source+row*12+n*4));}
 }
 void input(unsigned input,unsigned duration){word(0x39a,input);m.write_word(g-0x7386,duration);}
 void caption(unsigned id){const auto a=std::uint32_t(std::int32_t(std::int16_t(m.read_word(g-0x7384))));m.write_byte(a+5,0);m.write_word(a+0x1a,id);}
 void show_controls(){if(m.read_byte(g+0x581c)<3)return;m.write_byte(g-0x738a,255);for(unsigned subtype=0;subtype<3;++subtype){const auto a=allocate_scene_first(g,m);if(!a)throw std::runtime_error("Source tutorial control allocation invariant");m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,0x3200+subtype);}}
 void finish(){word(8,m.read_word(g+8)+2);m.write_byte(g-0x738c,0);for(unsigned o:{0x301u,0x701u,0xb01u,0xc01u})byte(o,0);}
 void reset_players(std::uint32_t cursor){
  m.write_long(g-0x7390,cursor);for(unsigned d:{0x738cu,0x738bu,0x738au})m.write_byte(g-d,0);for(unsigned d:{0x7388u,0x7386u,0x7382u,0x7380u})m.write_word(g-d,0);
  word(0x39a,0);word(0x39c,0);const auto p=g+0x300;for(unsigned o:{4u,0xd0u,0xd4u})m.write_long(p+o,0);for(unsigned o=0x100;o<0x400;o+=4)m.write_long(p+o,0);
  action(SourceAttractAction::initialize_players);action(SourceAttractAction::cpu_start_skill);action(SourceAttractAction::cpu_match_level);clear_source_tutorial_fields(g,m);
 }
 // 3E90: happens AFTER gameplay and the new input directives. Freeze holds
 // only pair Y/fraction; input duration expires before the next gameplay call.
 void frame_tail(){
  if(m.read_byte(g-0x738b)){word(0x314,m.read_word(g-0x7382));word(0x542,m.read_word(g-0x7380));}
  m.write_word(g-0x7382,m.read_word(g+0x314));m.write_word(g-0x7380,m.read_word(g+0x542));word(0x710,0);word(0x714,0);
  if(m.read_word(g-0x7386)){m.write_word(g-0x7386,m.read_word(g-0x7386)-1);if(!m.read_word(g-0x7386)){m.write_word(g-0x7386,0);word(0x39a,0);}}
  const auto a=g+0xb00;if(m.read_word(a+0xe)){m.write_word(a+0xe,m.read_word(a+0xe)-1);m.write_long(a+0x10,m.read_long(a+0x10)+m.read_long(a+0x20));m.write_long(a+0x20,m.read_long(a+0x20)+m.read_long(a+0x28));m.write_long(a+0x14,m.read_long(a+0x14)+m.read_long(a+0x24));m.write_long(a+0x24,m.read_long(a+0x24)+m.read_long(a+0x2c));}
  auto quiet=[](std::uint32_t,std::uint32_t,std::uint32_t){};ActorTasks<M,decltype(quiet)>{m,quiet,g}.step(a);
 }
 // Source USA4CD4..5453 has been translated to these fixed C++ continuation
 // stages. No original command stream is read or decoded by the product.
 void tick(){
  if(m.read_word(g+0x90)!=2)throw std::runtime_error("Only USA how-to continuation is recovered");
  switch(phase){
#include <puzzle_fighter/generated/source_howto_sequence.inc>
  default:throw std::runtime_error("Finished or invalid source how-to continuation");
  }frame_tail();
 }
};
template<class M,class H> struct SourceAttractGameplay {
 M&m;H&h;std::uint32_t g;SourceHowToSequence<M,H>howto;bool awaiting_clear=false;
 SourceAttractGameplay(M&memory,H&hooks,std::uint32_t globals):m(memory),h(hooks),g(globals),howto{memory,hooks,globals}{}
 unsigned b(unsigned o){return m.read_byte(g+o);}unsigned w(unsigned o){return m.read_word(g+o);}
 void byte(unsigned o,unsigned v){m.write_byte(g+o,v);}void word(unsigned o,unsigned v){m.write_word(g+o,v);}
 void action(SourceAttractAction a,unsigned x=0,unsigned y=0){h.action(a,x,y);}
 void profile(){if(w(0x90)!=2||b(0x86)||b(0xce)||!b(0x83))throw std::runtime_error("Unbound source attract cabinet profile");if(awaiting_clear)throw std::runtime_error("Source tutorial clear pending");}
 void clear_steps(unsigned start){for(unsigned o=start;o<0x18;o+=4)m.write_long(g+o,0);}
 void initial_pair_actor(unsigned side){const auto p=g+0x300+side*0x400,a=g+0xb00+side*0x100;m.write_byte(p,1);m.write_byte(p+1,1);m.write_byte(p+2,side);m.write_byte(p+0x81,side);m.write_byte(p+0x9e,1);m.write_word(p+0x32,a);m.write_byte(p+0x266,1);m.write_byte(a,1);m.write_byte(a+1,1);m.write_byte(a+2,side);m.write_word(a+0x32,p);}
 // 2CF4: opcode table chooses actual fighters and stage; no RNG is drawn.
 void choose_demo_match(){const auto index=(m.read_byte(g-0x7398)&14)*2;byte(0x382,m.opcode_byte(0x2d12+index));byte(0x782,m.opcode_byte(0x2d13+index));word(0x100,m.opcode_word(0x2d14+index));}
 void demo_border(){for(unsigned row=0;row<20;++row){const auto value=row==0?0x04f20000u:row==19?0x04ff0000u:0x00200000u;for(unsigned col=0;col<6;++col)m.write_long(0x900758+row*0x80+col*4,value);}}
 void demo_label(){const auto a=allocate_scene_first(g,m);if(!a)return;m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,0x304);m.write_byte(a+0xa,w(0x90)?0x2f:0x2e);m.write_word(a+0x10,0xc0);m.write_word(a+0x14,0x30);const auto brightness=m.read_long(0x90c140)&0xf000f000u;for(unsigned n=0;n<8;++n)m.write_long(0x90c140+n*4,m.read_long(0xab860+n*4)|brightness);}
 void gameplay(){if(b(0x10e))action(SourceAttractAction::stopped_turns);else action(SourceAttractAction::turns);}
 // Original A66C common work, deliberately bypassing arcade round controller.
 void demo_gameplay_frame(){if(b(0x10f))return;word(0xb4,w(0xb4)+1);gameplay();action(SourceAttractAction::reactions);action(SourceAttractAction::characters);action(SourceAttractAction::first_pool);action(SourceAttractAction::second_pool);action(SourceAttractAction::third_pool);action(SourceAttractAction::music_control);action(SourceAttractAction::stage_tick);if(b(0x124))byte(0x124,b(0x124)-1);action(SourceAttractAction::gem_pictures);action(SourceAttractAction::objects);}
 SourceAttractResult cpu_demo(){profile();switch(w(8)){
 case 0:switch(w(0xc)){
  case 0:word(0xc,2);word(0x20,0x92a0);word(0x48,0x92a0);clear_scene_records(g,m,true);word(0x80,0);for(unsigned o:{0xacu,0x8fu,0x8au,0x8bu,0x4782u})byte(o,0);word(0x112,8);byte(0xdf,1);m.write_byte(g-0x7374,2);action(SourceAttractAction::idle_credit_marker);word(0x102,1);byte(0x133,1);choose_demo_match();initial_pair_actor(0);initial_pair_actor(1);word(0x334,0x8700);word(0x734,0x8300);action(SourceAttractAction::round_music);action(SourceAttractAction::scene_palettes);action(SourceAttractAction::demo_text);demo_label();action(SourceAttractAction::initialize_players);action(SourceAttractAction::cpu_start_skill);return SourceAttractResult::complete;
  case 2:word(8,2);word(0xa,0x1c20);clear_steps(0xc);byte(0x10c,0);action(SourceAttractAction::cpu_match_level);action(SourceAttractAction::stage_setup);action(SourceAttractAction::round_labels);demo_border();byte(0x10d,1);word(0x20,0x90c0);action(SourceAttractAction::fade_in);return SourceAttractResult::complete;
  default:throw std::runtime_error("Unsupported source CPU attract setup");}
 case 2:word(0xa,w(0xa)-1);if((w(0xa)&0x8000)||b(0x10c)){word(8,4);for(unsigned o:{0x301u,0x701u,0xb01u,0xc01u})byte(o,0);byte(0x10e,1);}else demo_gameplay_frame();return SourceAttractResult::complete;
 case 4:word(0,w(0)+2);clear_steps(4);action(SourceAttractAction::stop_music);action(SourceAttractAction::reset_music);return SourceAttractResult::complete;
 default:throw std::runtime_error("Unsupported source CPU attract phase");}
 }
 SourceAttractResult tutorial(){profile();switch(w(8)){
 case 0:switch(w(0xc)){
  case 0:word(0xc,2);word(0x20,0x92a0);word(0x48,0x92a0);clear_scene_records(g,m,true);awaiting_clear=true;return SourceAttractResult::clear_screen;
  case 2:word(8,2);word(0xa,0x258);clear_steps(0xc);action(SourceAttractAction::initialize_players);action(SourceAttractAction::cpu_start_skill);action(SourceAttractAction::cpu_match_level);clear_source_tutorial_fields(g,m);word(0x20,0x90c0);action(SourceAttractAction::fade_in);return SourceAttractResult::complete;
  default:throw std::runtime_error("Unsupported source tutorial setup");}
 case 2:word(0xb4,w(0xb4)+1);gameplay();word(0x39c,w(0x39a));howto.tick();action(SourceAttractAction::first_pool);action(SourceAttractAction::third_pool);action(SourceAttractAction::gem_pictures);action(SourceAttractAction::objects);return SourceAttractResult::complete;
 case 4:word(0,w(0)+2);clear_steps(4);byte(0xd8,0);return SourceAttractResult::complete;
 default:throw std::runtime_error("Unsupported source tutorial phase");}
 }
 SourceAttractResult resume_tutorial_after_clear(){
  if(!awaiting_clear){throw std::runtime_error("No source tutorial clear pending");}awaiting_clear=false;
  clear_steps(0x10);word(0x80,0);for(unsigned o:{0xacu,0x8fu,0x8au,0x8bu,0x4782u,0x10eu})byte(o,0);word(0x112,0);byte(0xdf,3);byte(0xd8,255);m.write_long(g-0x738c,0);m.write_long(g-0x7388,0);word(0x102,1);byte(0x133,1);m.write_long(g-0x7394,0x4cd4);howto.reset();
  for(unsigned o:{0x22u,0x26u,0x2au}){word(o,0);}word(0x24,0x100);word(0x28,0x300);word(0x2c,0x700);
  action(SourceAttractAction::text_page,0x12,0x900000);action(SourceAttractAction::text_page,0x13,0x901000);action(SourceAttractAction::background_page,0x6c,0x904000);action(SourceAttractAction::background_page,0x6d,0x904400);word(0x32,0x1b0e);word(0x3a,0x2461);action(SourceAttractAction::idle_credit_marker);
  byte(0x300,2);byte(0x301,1);byte(0x382,10);word(0x39a,0);word(0x39c,0);byte(0x3a9,3);word(0x332,0x8b00);
  byte(0xb00,b(0xb00)+1);byte(0xb01,1);byte(0xb82,10);word(0xb30,0xa000);word(0xb32,0x8300);word(0xb0c,0);word(0xb0e,0);for(unsigned o:{0xb20u,0xb24u,0xb28u,0xb2cu})m.write_long(g+o,0);
  byte(0x700,0);byte(0x701,0);word(0x79a,0);word(0x79c,0);word(0x732,0x8c00);byte(0xc00,0);byte(0xc01,0);word(0xc32,0x8700);word(0x334,0x8700);word(0x734,0x8300);
  auto quiet=[](std::uint32_t,std::uint32_t,std::uint32_t){};ActorTasks<M,decltype(quiet)>{m,quiet,g}.init(g+0xc00,0,0x7abb4); // inherited source A6=C00
  const auto a=allocate_scene_third(g,m);if(a){m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,1);m.write_word(a+0x1a,0);m.write_word(g-0x7384,a);}
  word(0x128,4);action(SourceAttractAction::tutorial_palettes);action(SourceAttractAction::tutorial_music);return SourceAttractResult::complete;
 }
};
} // namespace puzzle_fighter
