// SPDX-License-Identifier: GPL-3.0-only
// USA arcade scene CONTROL. Named service callbacks require real native bodies;
// this header does not implement graphics, audio transport or the old CPU kernel.
#pragma once
#include "global_rng.hpp"
#include <cstdint>
#include <initializer_list>
#include <utility>
#include <stdexcept>
namespace puzzle_fighter {
enum class ArcadeAction {
 reset_pools,reset_third_pool,first_pool,second_pool,third_pool,stage_tick,objects,
 show_totals,clear_totals,loser_colours,quote_init,award_message,award_accounting,
 match_end_song,character_act,stage_setup,gem_coordinates,scene_palettes,scene_prepare,
 scene_static,scene_aux_palette,scene_common_palette,scene_tick,scene_caption,
 interlude_song,sound_command,show_characters,selection_palette,bring_back_characters,
 draw_picture,draw_stage_number,draw_ladder,stage_spin,stage_opponent,ladder_spin,last_opponent,
 selection_start_sound,selection_done_sound,last_opponent_sound,scene_actor_setup,
 continue_palette,continue_sound,continue_song,copy_palette,gameover_song,gameover_palette,
 gameover_voice,kill_job,begin_palette_fade,give_ladder,join_credit_used,join_player_reset,
 player_join_accounting,join_clear_fields,join_cue,player_prompt,player_prompt_clear,
 player_gameover_prompt,player_countdown,end_fade_screen
};
enum class ArcadeFlowResult {complete,wait_palette_fade,clear_screen,unsupported};
enum class ArcadeScene {selection,match_setup,match_intro,playing,post_match,stage_interlude,ladder,continue_screen,game_over,ending,title_reset,other};
// SourceA1DC/A24A dispatch destinations, without executing an unbound scene.
template<class M> ArcadeScene arcade_scene(std::uint32_t g,M&m){
 switch(m.read_word(g+4)){
 case 2:return ArcadeScene::selection;
 case 4:switch(m.read_word(g+8)){case 0:return ArcadeScene::match_setup;case 2:return ArcadeScene::match_intro;case 4:return ArcadeScene::playing;case 6:return ArcadeScene::post_match;case 8:return ArcadeScene::stage_interlude;case 10:return ArcadeScene::ladder;default:return ArcadeScene::other;}
 case 6:return ArcadeScene::continue_screen;case 8:return ArcadeScene::game_over;case 10:return ArcadeScene::ending;case 12:return ArcadeScene::title_reset;default:return ArcadeScene::other;
 }
}
namespace arcade_flow_detail {
inline int sw(std::uint16_t n){return n&0x8000?int(n)-65536:int(n);}
inline int sb(std::uint8_t n){return n&0x80?int(n)-256:int(n);}
inline std::uint32_t ptr(std::uint16_t n){return std::uint32_t(sw(n));}
// B992/B9B2 use raw input edge words and separate system Start bits.
template<class M> bool pressed(std::uint32_t g,M&m,bool players_in=false){
 auto mask=m.read_byte(g+0xac);if(players_in&&!mask)mask=m.read_byte(g+0xa8);
 unsigned pressed=0;for(unsigned side=0;side<2;++side)if(mask&(1u<<side)){pressed|=(m.read_word(g+0x58+side*4)&~m.read_word(g+0x5a+side*4))&0x7700;pressed|=(m.read_byte(g+0x60)&~m.read_byte(g+0x61))&(1u<<side);}return pressed!=0;
}
}
// H: action(ArcadeAction,x=0,y=0,z=0); allocate_first()/allocate_third()
// return source allocated address or0; overlay(kind,caller_actor) returns
// acceptance. Callbacks must preserve source side effects and may not discard
// active work. An absent binding must throw, never act as successful no-op.
template<class M,class H> struct ArcadeMatchFlow {
 M&m;H&h;std::uint32_t g;
 enum class Continuation {none,continue_after_fade,gameover_after_fade,gameover_after_clear};
 Continuation continuation=Continuation::none;
 std::uint8_t b(unsigned o){return m.read_byte(g+o);}std::uint16_t w(unsigned o){return m.read_word(g+o);}
 void byte(unsigned o,unsigned v){m.write_byte(g+o,v);}void word(unsigned o,unsigned v){m.write_word(g+o,v);}
 void inc(unsigned o){word(o,w(o)+2);}void action(ArcadeAction a,std::uint32_t x=0,std::uint32_t y=0,std::uint32_t z=0){h.action(a,x,y,z);}
 bool below(unsigned o){word(o,w(o)-1);return w(o)&0x8000;}bool out(unsigned o){word(o,w(o)-1);return w(o)==0;}
 unsigned random(){const auto r=global_random(w(0xae));word(0xae,r.seed);return r.value;}
 bool pressed(bool players=false){return arcade_flow_detail::pressed(g,m,players);}
 void clear_steps(unsigned start){for(unsigned i=start;i<0x18;i+=4)m.write_long(g+i,0);}
 void next_screen(){inc(8);clear_steps(0xc);}
 std::uint32_t first(unsigned kind){auto a=h.allocate_first();if(a){m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,kind);}return a;}
 void tails(ArcadeScene scene){
  if(scene==ArcadeScene::ladder)action(ArcadeAction::show_characters);
  action(ArcadeAction::first_pool);
  if(scene==ArcadeScene::post_match||scene==ArcadeScene::stage_interlude||scene==ArcadeScene::ladder)action(ArcadeAction::second_pool);
  if(scene==ArcadeScene::post_match||scene==ArcadeScene::ladder||scene==ArcadeScene::continue_screen)action(ArcadeAction::third_pool);
  if(scene==ArcadeScene::post_match||scene==ArcadeScene::continue_screen)action(ArcadeAction::stage_tick);
  action(ArcadeAction::objects);
 }
 void fade_out(unsigned setting){if(h.overlay(0,g+0x2900)){inc(0xc);byte(0x8c,0xff);word(0x290c,setting);}}
 void fade_end(){inc(0xc);action(ArcadeAction::end_fade_screen);action(ArcadeAction::reset_pools);}
 void award(){
  if(!b(0xce)||b(0xd6))return;
  const auto winner=arcade_flow_detail::ptr(w(0x13a));if(m.read_byte(winner+0x9e))return;
  if(b(0x141)?b(0x14e)!=m.read_byte(winner+0xa2):b(0x144)!=b(0x14f))return;
  byte(0xd6,0xff);if(b(0xcf)&0x80)return;byte(0xd0,b(0xd0)+1);word(0x136,0x31);action(ArcadeAction::award_message);action(ArcadeAction::award_accounting);
 }
 // B2FC/B32A/BC02/BB16/AAF6 state closure.
 void note_loss(std::uint32_t p){m.write_byte(p+0xf8,b(0x101));m.write_byte(p+0xf7,m.read_byte(p+0x9f));m.write_byte(p+0xf9,m.read_byte(g-0x7374));m.write_byte(p+0xfa,m.read_byte(p+0xae));m.write_byte(p+0xfb,m.read_byte(p+0xb0));}
 void keep_scores(){for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400;if(m.read_byte(p+0x9e))continue;for(auto offsets:{std::pair<unsigned,unsigned>{0xaf,0xae},{0xb1,0xb0}})if(m.read_byte(p+offsets.first)>=m.read_byte(p+offsets.second))m.write_byte(p+offsets.second,m.read_byte(p+offsets.first));}
  for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400;if(m.read_byte(p+0x9e))continue;if(b(0xac)!=3)m.write_long(p+0x90,m.read_long(p+0xaa));if(!(b(0x10c)&(1u<<(m.read_byte(p+0x81)&7))))note_loss(p);}
 }
 void choose_quote(){const auto winner=arcade_flow_detail::ptr(w(0x13a));if(w(0x92)){word(0x136,(random()&3)+(unsigned(m.read_byte(winner+0x82))<<2));return;}byte(0x153,(b(0x153)+1)&0x3f);unsigned quote=m.read_byte(0xbcf8+b(0x153));if(quote==8)quote=std::uint8_t(quote+m.read_byte(arcade_flow_detail::ptr(w(0x13c))+0x82));word(0x136,quote+m.opcode_word(0xbc44+2*m.read_byte(winner+0x82)));}
 void next_opponent(std::uint32_t winner,std::uint32_t loser){
  if(m.read_byte(winner+0x9e))return;
  auto met=w(0x110);if(m.read_byte(loser+0x9e)){met=std::uint16_t(met|(1u<<(m.read_byte(loser+0x82)&31)));word(0x110,met);}
  if(b(0xce)&&b(0x141)&&m.read_byte(winner+0x81)!=b(0x132)){byte(0x144,b(0xd4));byte(0x14f,0);byte(0xd6,0);}
  byte(0x132,m.read_byte(winner+0x81));for(unsigned i=0;i<16;i+=4)m.write_long(g-0x735a+i,m.read_long(0xbc78+b(0x122)+i));if(b(0x139)&&!(w(0x110)&0x100))m.write_byte(g-0x7354,8);if(b(0x138))m.write_byte(g-0x7352,9);
  unsigned stage=0;std::uint8_t ch=0;for(;;){ch=m.read_byte(g-0x735a+stage);if(stage>=w(0x114)||!(met&(1u<<(ch&31))))break;++stage;}word(0x112,stage);m.write_byte(loser+0x82,ch);word(0x100,std::uint16_t(arcade_flow_detail::sb(ch)*2));if(b(0xac)!=3&&stage==8)word(0x100,0x12);
 }
 void ladder_won(std::uint32_t winner){byte(0x130,m.read_byte(winner+0x82));byte(0x123,m.read_byte(winner+0x9f));note_loss(winner);word(4,10);byte(0x106,1);byte(0x8f,1);clear_steps(8);for(unsigned o:{0xacu,0x107u,0x105u,0x12au,0x133u,0x301u,0x701u,0xb01u,0xc01u,0xb8u,0xbdu,0x380u,0x780u,0x14au})byte(o,0);byte(0x8f,1);byte(0xd0,b(0xd0)+1);}
 void match_over(){
  inc(8);byte(0x10f,1);byte(0x380,0);byte(0x780,0);keep_scores();choose_quote();action(ArcadeAction::reset_third_pool);
  const auto actor=g+((b(0x10c)&1)?0xc00:0xb00);m.write_byte(actor+1,1);byte(0x154,m.read_byte(actor+0x82));byte(0x155,m.read_byte(actor+0x9f));
  //2D930 leaves A1=g2900; the operator-end branch intentionally uses it.
  auto loser=g+0x2900;const bool operator_end=b(0xa5)==3||(b(0xa5)==2&&b(0xe0));
  if(!operator_end){const auto winner=arcade_flow_detail::ptr(w(0x13a));loser=arcade_flow_detail::ptr(w(0x13c));next_opponent(winner,loser);if(b(0x14c)){ladder_won(winner);return;}byte(0xac,b(0xac)&~(1u<<(m.read_byte(loser+0x81)&7)));
   if(!m.read_byte(winner+0x9e)){action(ArcadeAction::give_ladder,0,winner,loser);m.write_byte(winner+0x80,2);m.write_byte(loser+0x9e,1);if(!b(0xa4)&&b(0xe0)){byte(0xe0,0);m.write_byte(loser+0xb4,1);}return;}
  }
  m.write_byte(loser+0x9e,1);byte(0xb8,1);byte(0x10f,1);for(unsigned o:{0xacu,0xe0u,0x380u,0x780u})byte(o,0);
 }
 bool post_match(){switch(w(0xc)){
 case 0:{
  const auto winner=arcade_flow_detail::ptr(w(0x13a));if(b(0x581c)<2)return true;
  auto a=arcade_flow_detail::ptr(h.allocate_first()),c=arcade_flow_detail::ptr(h.allocate_first());if(c>=a){const auto t=a;a=c;c=t;}
  for(auto task:{a,c}){m.write_byte(task,m.read_byte(task)+1);m.write_word(task+2,task==a?0x500:0xa00);m.write_byte(task+0xa,m.read_byte(winner+0x81));m.write_word(task+0x32,winner);}
  inc(0xc);word(0xe,0x30);for(unsigned o:{0x10fu,0x14du,0x133u,0x118u})byte(o,0);byte(0x8f,1);byte(0xbd,0xff);byte(0x10d,3);action(ArcadeAction::match_end_song);
  const auto loser_actor=g+((b(0x10c)&1)?0xc00:0xb00);action(ArcadeAction::character_act,0x1e,loser_actor);action(ArcadeAction::loser_colours,0,loser_actor);return true;
 }
 case 2:{
  if(!below(0xe)&&!pressed(true))return true;
  inc(0xc);byte(0x14d,0xff);byte(0x8f,b(0xac)?0:1);if(auto a=first(0x2500))m.write_word(a+0xc,0xff02);award();
  if(auto a=h.allocate_third()){m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,0);m.write_word(a+0x1a,w(0x136));action(ArcadeAction::quote_init,0,a);}action(ArcadeAction::show_totals);return true;
 }
 case 4:inc(0xc);word(0xe,0xd2);return true;
 case 6:
  if(b(0x8a)){action(ArcadeAction::clear_totals);return true;}if(!pressed(true)&&!below(0xe))return true;
  byte(0x118,0xff);action(ArcadeAction::clear_totals);if(b(0xb8)){inc(4);clear_steps(8);byte(0x14d,0);}else inc(0xc);byte(0x8f,1);action(ArcadeAction::reset_third_pool);return true;
 case 8:fade_out(0xff03);return true;
 case 10:if(b(0x290a))fade_end();return true;
 case 12:if(!b(0x8c))next_screen();return true;
 default:return false;
 }}
 bool stage_interlude(){switch(w(0xc)){
 case 0:
  if(b(0xbf)||w(0x112)!=4||w(0x148)){next_screen();return true;}if(!h.overlay(1,g+0x2900))return true;
  inc(0xc);word(0xe,3);byte(0x148,0xff);word(0x20,0x90c0);byte(0x8c,0xff);word(0x290c,0);byte(0x125,0);byte(0x105,0);byte(0x12a,0);byte(0x120,m.opcode_byte(0xae7a+(random()&15)));
  action(ArcadeAction::reset_pools);action(ArcadeAction::scene_palettes);action(ArcadeAction::scene_prepare);action(ArcadeAction::scene_static);action(ArcadeAction::interlude_song);return true;
 case 2:
  if(below(0xe)){inc(0xc);word(0xe,0x1b);action(ArcadeAction::stage_setup);action(ArcadeAction::stage_tick);action(ArcadeAction::gem_coordinates);action(ArcadeAction::scene_aux_palette);action(ArcadeAction::scene_common_palette);action(ArcadeAction::scene_caption,w(0x90)?1:0);}action(ArcadeAction::scene_tick);return true;
 case 4:if(below(0xe)){inc(0xc);word(0xe,0x1a4);}action(ArcadeAction::scene_tick);return true;
 case 6:if((below(0xe)||pressed())&&h.overlay(0,g+0x2900)){inc(0xc);byte(0x8c,0xff);action(ArcadeAction::sound_command,0x3300000);}action(ArcadeAction::scene_tick);return true;
 case 8:if(b(0x290a))fade_end();else action(ArcadeAction::scene_tick);return true;
 case 10:if(!b(0x8c))next_screen();return true;
 default:return false;
 }}
 void leave_match(){m.write_long(g+8,0);m.write_long(g+0xc,0);byte(0x8f,1);byte(0xbd,0xff);byte(0x301,0);byte(0x701,0);byte(0x133,0);}
 void leave_ladder(unsigned phase){word(0xc,phase);word(0xe,0x10);word(0x10,0);byte(0x14d,0xff);byte(0x8f,1);byte(0xbd,0xff);}
 void last_sound(){action(b(0x138)?ArcadeAction::selection_start_sound:ArcadeAction::last_opponent_sound);}
 bool ladder(){switch(w(0xc)){
 case 0:if(h.overlay(1,g+0x2900)){inc(0xc);word(0x20,0x90c0);byte(0x8c,0xff);word(0x290c,0);byte(0x14d,0);}return true;
 case 2:inc(0xc);word(0xe,2);return true;
 case 4:if(below(0xe)){inc(0xc);word(0xe,0);}return true;
 case 6:{
  inc(0xc);word(0x22,0);word(0x24,0);word(0x26,0x200);word(0x28,0x300);word(0x2a,0x40);word(0x2c,0x710);word(0x32,0x270e);word(0x3a,0x2641);action(ArcadeAction::reset_pools);word(0x128,2);action(ArcadeAction::selection_palette);action(ArcadeAction::bring_back_characters);action(ArcadeAction::draw_picture,4,0x902000);action(ArcadeAction::draw_picture,5,0x903000);action(ArcadeAction::draw_ladder);
  auto stage=w(0x112);if(stage==w(0x114)){stage=9;if(!b(0xbf)&&m.read_byte(g-0x7374)){action(ArcadeAction::scene_actor_setup);return true;}}action(ArcadeAction::draw_stage_number,std::uint16_t(stage+1),0x904a0c);action(ArcadeAction::scene_actor_setup);return true;
 }
 case 8:if(!b(0x8c)){inc(0xc);word(0x10,0);if(!b(0x152)){byte(0x8f,0);byte(0xbd,0);}word(0xe,0x12c);word(0x20,0x90c0);if(m.read_byte(g-0x7374)&&w(0x112)==8)word(0xc,0x12);}return true;
 case 10:
  if(b(0x8a)){word(0x10,6);return true;}
  switch(w(0x10)){
  case 0:inc(0x10);word(0x12,0x78);return true;
  case 2:if(pressed()){action(ArcadeAction::selection_start_sound);leave_ladder(w(0xc)+2);action(ArcadeAction::stage_opponent);return true;}if(out(0x12)){inc(0x10);word(0x12,0x78);action(ArcadeAction::selection_start_sound);action(ArcadeAction::stage_opponent);action(ArcadeAction::selection_done_sound);}else action(ArcadeAction::stage_spin);return true;
  case 4:if(pressed()||out(0x12)){leave_ladder(w(0xc)+2);action(ArcadeAction::stage_opponent);}return true;
  case 6:return true;default:return false;
  }
 case 12:if(below(0xe))fade_out(0);return true;
 case 14:if(b(0x290a))fade_end();return true;
 case 16:if(!b(0x8c))leave_match();return true;
 case 18:
  if(b(0x8a)){word(0x10,10);return true;}
  switch(w(0x10)){
  case 0:inc(0x10);word(0x12,0x2d);return true;
  case 2:if(out(0x12)){inc(0x10);word(0x20,0x9280);word(0x12,8);}return true;
  case 4:if(out(0x12)){inc(0x10);word(0x12,0x78);action(ArcadeAction::draw_stage_number,10,0x904a0c);word(0x20,0x90c0);}return true;
  case 6:if(pressed()){last_sound();leave_ladder(12);action(ArcadeAction::last_opponent);return true;}if(out(0x12)){inc(0x10);word(0x12,0x78);last_sound();action(ArcadeAction::last_opponent);}else action(ArcadeAction::ladder_spin);return true;
  case 8:if(!pressed()&&out(0x12)){leave_ladder(12);action(ArcadeAction::last_opponent);}return true; // B252 branches to B164 on input: source quirk.
  case 10:return true;default:return false;
  }
 default:return false;
 }}
 std::uint32_t continuer(){if(b(0x8e)==3)return arcade_flow_detail::sb(b(0x38c))>=arcade_flow_detail::sb(b(0x78c))?g+0x300:g+0x700;return g+((b(0x8e)&1)?0x300:0x700);}
 ArcadeFlowResult continue_screen(){switch(w(8)){
 case 0:{
  inc(8);byte(0x14d,0);byte(0x10f,0);byte(0x10d,3);byte(0x8f,1);byte(0xbd,0xff);word(0x116,continuer());action(ArcadeAction::reset_third_pool);action(ArcadeAction::continue_palette);
  if(!b(0x11f))break;
  if(b(0xce)&&!b(0x141)&&!b(0xd6)){const auto n=std::uint8_t(b(0x144)-1);if(n!=b(0x14f))byte(0x144,n);}
  action(ArcadeAction::continue_sound);action(ArcadeAction::continue_song);action(ArcadeAction::scene_palettes);action(ArcadeAction::copy_palette,0xab880,0x90c160,0);
  auto a=first(0x302);if(!a)break;m.write_byte(a+0xa,9);m.write_word(a+0x10,0xc0);m.write_word(a+0x14,0xd0);
  a=first(0x303);if(!a)break;m.write_byte(a+0xa,0x13);m.write_word(a+0x10,0x100);m.write_word(a+0x14,0x80);
  (void)h.allocate_first();a=h.allocate_first();if(a){m.write_byte(a,m.read_byte(a)+1);m.write_byte(a+2,0xc);const bool p1=b(0xb9)&1;m.write_word(a+0xc,p1?0:0xff04);m.write_byte(a+3,b((p1?0x300:0x700)+0xf6));}break;
 }
 case 2:inc(8);byte(0x8f,0);byte(0xbd,0);break;
 case 4:
  word(0x116,continuer());if(b(0xb8)&0x80){word(4,0);word(6,0);clear_steps(8);byte(0x14d,0);byte(0xb8,0);byte(0xbd,0);byte(0x10f,0);byte(0x8f,1);action(ArcadeAction::begin_palette_fade);continuation=Continuation::continue_after_fade;return ArcadeFlowResult::wait_palette_fade;}
  if(!b(0x8e)){word(4,8);word(6,0);clear_steps(8);byte(0xb8,0);byte(0xbd,0);word(0x110,0);byte(0x14d,1);byte(0x10f,0);byte(0x8f,1);byte(0x106,0);byte(0x104,1);word(0x20,0x90c0);}break;
 default:return ArcadeFlowResult::unsupported;
 }tails(ArcadeScene::continue_screen);return ArcadeFlowResult::complete;}
 ArcadeFlowResult game_over(){switch(w(8)){
 case 0:
  inc(8);word(0xa,0x1f);byte(0x14d,0);byte(0x10f,0);byte(0x10d,4);action(ArcadeAction::reset_third_pool);action(ArcadeAction::gameover_song);action(ArcadeAction::gameover_palette);action(ArcadeAction::copy_palette,0xab8a0,0x90c160,0);
  for(unsigned part:{0u,2u,8u,10u}){const auto a=first(0x307);if(!a)break;m.write_byte(a+0xa,0x14);m.write_byte(a+0xe,part);}break;
 case 2:if(below(0xa)){inc(8);word(0xa,0x2f);action(ArcadeAction::gameover_voice);}break;
 case 4:if(below(0xa)){inc(8);byte(0x14d,0);word(0xa,0xb4);}break;
 case 6:if(!pressed()&&!below(0xa))break;inc(8);[[fallthrough]];
 case 8:if(b(0xa8))break;word(4,14);clear_steps(8);action(ArcadeAction::kill_job,0x80);action(ArcadeAction::reset_pools);action(ArcadeAction::begin_palette_fade);continuation=Continuation::gameover_after_fade;return ArcadeFlowResult::wait_palette_fade;
 default:return ArcadeFlowResult::unsupported;
 }tails(ArcadeScene::game_over);return ArcadeFlowResult::complete;}
 ArcadeFlowResult run(ArcadeScene scene){
  if(continuation!=Continuation::none)return ArcadeFlowResult::unsupported;
  if(scene==ArcadeScene::continue_screen)return continue_screen();
  if(scene==ArcadeScene::game_over)return game_over();
  const bool ok=scene==ArcadeScene::post_match?post_match():scene==ArcadeScene::stage_interlude?stage_interlude():scene==ArcadeScene::ladder?ladder():false;if(!ok)return ArcadeFlowResult::unsupported;tails(scene);return ArcadeFlowResult::complete;
 }
 ArcadeFlowResult resume_after_palette_fade(){
  if(continuation==Continuation::continue_after_fade){continuation=Continuation::none;action(ArcadeAction::reset_pools);tails(ArcadeScene::continue_screen);return ArcadeFlowResult::complete;}
  if(continuation==Continuation::gameover_after_fade){continuation=Continuation::gameover_after_clear;return ArcadeFlowResult::clear_screen;}return ArcadeFlowResult::unsupported;
 }
 ArcadeFlowResult resume_after_object_clear(){if(continuation!=Continuation::gameover_after_clear)return ArcadeFlowResult::unsupported;continuation=Continuation::none;tails(ArcadeScene::game_over);return ArcadeFlowResult::complete;}
};
//98D0's independent coroutine slice: P1 then P2, then one IRQ wait.
// Integration must schedule this source job; calling only the scene flow never
// accepts a join or advances a continue countdown.
template<class M,class H> struct ArcadePlayerFlow {
 M&m;H&h;std::uint32_t g,p;
 std::uint8_t b(unsigned o){return m.read_byte(p+o);}std::uint8_t global(unsigned o){return m.read_byte(g+o);}
 void byte(unsigned o,unsigned v){m.write_byte(p+o,v);}void set_global(unsigned o,unsigned v){m.write_byte(g+o,v);}
 void action(ArcadeAction a,unsigned x=0){h.action(a,x,p,0);}
 void prompt(unsigned n){action(ArcadeAction::player_prompt,n&255);}void clear_prompt(){action(ArcadeAction::player_prompt_clear);}
 std::uint32_t credits(){return g+0x70+(global(0x9e)?unsigned(b(0x81))*8:0);}
 static std::pair<std::uint8_t,bool> subtract_credit(unsigned have,unsigned price){int value=int(have&255)-int(price&255);if(int(have&15)-int(price&15)<0)value-=6;const bool borrow=value<0;if(borrow)value-=0x60;return {std::uint8_t(value),borrow};}
 bool start(){return !global(0x106)&&!global(0x8f)&&((global(0x60)&~global(0x61))&(b(0x81)?2:1));}
 void no_join(){if(global(0x80)&0x20)prompt(0x19);else clear_prompt();}
 void end_continue(){byte(0x84,b(0x84)+2);for(unsigned o:{0x85u,0x86u,0x87u,0x8au,0x8cu,0x98u,0xb2u,0xb3u})byte(o,0);set_global(0x8e,global(0x8e)&~(1u<<(b(0x81)&7)));clear_prompt();}
 void accepted_player(){
  byte(0x84,6);byte(0x85,0);for(unsigned o:{0x86u,0x88u,0x8au,0x8cu})m.write_word(p+o,0);byte(0x80,2);byte(0x9e,0);byte(0xa3,0);m.write_long(p+0xa2,0);m.write_long(p+0xae,0);byte(0xb4,0);byte(0x82,b(0xf6));action(ArcadeAction::join_player_reset);
  const auto bit=1u<<(b(0x81)&7);set_global(0xa8,global(0xa8)|bit);set_global(0x8d,global(0x8d)|bit);set_global(0xad,1);action(ArcadeAction::player_join_accounting);
 }
 bool accepted_join(){
  if(global(0x106)){end_continue();return true;}
  if(b(0x85)==0){
   if(global(0x8f)||global(0x8a)||global(0x125)){clear_prompt();return true;}
   if(global(0xb8)){if(global(0xa8)!=3)action(ArcadeAction::join_clear_fields);set_global(0xb8,0xff);accepted_player();clear_prompt();return true;}
   if(global(0xab)){accepted_player();clear_prompt();return true;}
   if(!h.overlay(2,p))return true;
   set_global(0x8a,0xff);set_global(0x8b,0xff);byte(0x85,b(0x85)+2);set_global(0x4781,0);set_global(0x4782,0);h.action(ArcadeAction::reset_third_pool,0,0,0);action(ArcadeAction::join_cue);return true;
  }
  if(b(0x85)!=2)return false;
  if(!global(0x8a)){accepted_player();m.write_word(g,2);m.write_word(g+2,0);for(unsigned o=4;o<0x18;o+=4)m.write_long(g+o,0);}clear_prompt();return true;
 }
 void ready_prompt(){if(global(0x80)&15)return;unsigned n=0xc;if(global(0x9c))n=(global(0x80)&0x20)?0:1;prompt(n);}
 bool waiting_credit(){
  if(global(0x104)||global(0x106)){clear_prompt();return true;}if(!global(0xa4)){no_join();return true;}
  const auto c=credits();const auto count=m.read_byte(c+6),partial=m.read_byte(c+1);const auto sub=b(0x85);if(sub!=0&&sub!=2)return false;
  const bool enough=global(0x9d)?!subtract_credit(count,1+global(0x9d)).second:(count||global(0x9c));
  if(enough){byte(0x84,b(0x84)+2);byte(0x85,0);return true;}
  if(sub==0){byte(0x85,2);if(global(0x9d))prompt(count?0x10:4);else prompt(partial?std::uint8_t(m.read_byte(c+2)-partial+15):std::uint8_t(m.read_byte(c+2)+2));
  return true;}
  if(global(0x9d)){if(count)prompt(0x10);else if(!(global(0x80)&15))prompt(global(0x80)&0x20?2:4);}
  else if(partial)prompt(std::uint8_t(m.read_byte(c+2)-partial+15));else if(!(global(0x80)&15))prompt(global(0x80)&0x20?2:std::uint8_t(m.read_byte(c+2)+2));
  return true;
 }
 bool ready_to_join(){
  if(global(0x104)||global(0x106)||global(0x8f)||global(0x125)){clear_prompt();return true;}if(!global(0xa4)){no_join();return true;}
  const auto c=credits();if(!global(0x9c)&&subtract_credit(m.read_byte(c+6),1+global(0x9d)).second){m.write_word(p+0x84,0);return true;}
  if(b(0x85)==0){byte(0x85,2);prompt(global(0x9c)?1:0xc);return true;}if(b(0x85)!=2)return false;ready_prompt();if(!start())return true;
  if(!global(0x9c)){const auto debit=subtract_credit(m.read_byte(c+6),1+global(0x9d));if(debit.second)return true;m.write_byte(c+6,debit.first);}
  action(ArcadeAction::join_credit_used);byte(0x84,4);byte(0x85,0);m.write_long(p+0x90,0);byte(0x98,0);byte(0xf6,b(0x81)+2);return accepted_join();
 }
 bool continue_player(){
  set_global(0x8e,global(0x8e)|(1u<<(b(0x81)&7)));if(b(0xb4)||global(0x106)||!global(0x11f)){end_continue();return true;}
  auto c=credits();if(b(0x85)==0){byte(0x85,2);byte(0x83,m.read_byte(c+6));byte(0x99,m.read_byte(c+1));byte(0x8a,0x7f);byte(0x8c,9);clear_prompt();return true;}if(b(0x85)!=2)return false;
  if(!global(0x8f)&&!global(0xbd)&&((m.read_word(p+0x9a)&~m.read_word(p+0x9c))&0x7700)){if(!b(0x8c))byte(0x8a,1);else{byte(0x8c,b(0x8c)-1);byte(0x8a,0x60);}}
  if(arcade_flow_detail::sb(m.read_byte(c+6))>arcade_flow_detail::sb(b(0x83))||arcade_flow_detail::sb(m.read_byte(c+1))>arcade_flow_detail::sb(b(0x99))){
   byte(0x83,m.read_byte(c+6));byte(0x99,m.read_byte(c+1));if(global(0x8e)==3){for(unsigned side=0;side<2;++side){m.write_byte(g+0x38a+side*0x400,0x7f);m.write_byte(g+0x38c+side*0x400,9);}}else{byte(0x8a,0x7f);byte(0x8c,9);}
  }
  if((global(0x9c)||m.read_byte(c+6))&&start()){
   const auto debit=subtract_credit(m.read_byte(c+6),1);if(global(0x9c)||!debit.second){if(!global(0x9c))m.write_byte(c+6,debit.first);set_global(0x8e,global(0x8e)&~(1u<<(b(0x81)&7)));if(b(0x98)<9)m.write_long(p+0x90,m.read_long(p+0x90)+1);byte(0x84,4);byte(0x85,0);byte(0x8a,0x7f);byte(0x98,b(0x98)+1);action(ArcadeAction::join_credit_used);return accepted_join();}
  }
  if(global(0x8f)||global(0xbd)){clear_prompt();return true;}byte(0x8a,b(0x8a)-1);if(!b(0x8a)){byte(0x8a,0x7f);byte(0x8c,b(0x8c)-1);if(b(0x8c)&0x80){end_continue();return true;}}
  byte(0x83,m.read_byte(c+6));byte(0x99,m.read_byte(c+1));prompt(0xe);action(ArcadeAction::player_countdown);return true;
 }
 bool player_gameover(){
  if(global(0x106)){clear_prompt();return true;}switch(b(0x85)){
  case 0:byte(0x85,2);byte(0x8a,0x5a);prompt(15);return true;
  case 2:if(!((m.read_word(p+0x9a)&~m.read_word(p+0x9c))&0x7700)){byte(0x8a,b(0x8a)-1);if(b(0x8a)){prompt(15);return true;}}byte(0x85,4);byte(0x8a,0x1e);[[fallthrough]];
  case 4:byte(0x8a,b(0x8a)-1);if(b(0x8a)&0x80){byte(0x85,b(0x85)+2);byte(0x98,0);set_global(0xa8,global(0xa8)&~(1u<<(b(0x81)&7)));clear_prompt();}else prompt(15);return true;
  case 6:if(global(0xb8))clear_prompt();else m.write_long(p+0x84,6);return true; // source9F98 retains dispatch D0=6
  default:return false;
  }
 }
 bool one(std::uint32_t player){p=player;if(b(0x80))return true;switch(b(0x84)){
 case 0:return waiting_credit();case 2:return ready_to_join();case 4:return accepted_join();case 6:return continue_player();case 8:if(global(0xbd))clear_prompt();else action(ArcadeAction::player_gameover_prompt);return true;case 10:return player_gameover();default:return false;
 }}
 bool run(){return one(g+0x300)&&one(g+0x700);}
};
} // namespace puzzle_fighter
