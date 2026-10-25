// SPDX-License-Identifier: GPL-3.0-only
// USA2E0C animated intro and2506 static title. Explicit original dependencies;
// no job scheduler or script interpreter. All calls run in the idle job context.
#pragma once
#include "scene_reset.hpp"
#include "scene_actor_tasks.hpp"
#include <stdexcept>
namespace puzzle_fighter {
enum class SourceTitlePaletteBank {objects,scroll1,scroll2,scroll3};
//20EC8/20F0E/20F4C/20F7C. Unlike20C4C these force full brightness.
template<class M> void source_title_palette_bank(std::uint32_t g,M&m,SourceTitlePaletteBank bank){
 const auto index=m.read_word(g+0x128);
 const auto offset=std::int16_t(std::uint16_t((index>>7)|(index<<9)));
 const unsigned n=unsigned(bank);if(n>3)throw std::runtime_error("Invalid source title palette bank");
 const std::uint32_t bases[]={0xa12c0,0xa1ec0,0xa4ec0,0xa86c0};
 const auto from=std::uint32_t(bases[n]+(n==1?0:offset)),to=0x90c000+n*0x400;
 for(unsigned i=0;i<0x400;i+=4)m.write_long(to+i,m.read_long(from+i)|0xf000f000u);
}
//20DEE order is object,scroll1,scroll2,scroll3. The intro's explicit four
// calls instead put scroll3 before scroll2 and change128 to4 before scroll2.
template<class M> void source_title_palettes(std::uint32_t g,M&m){
 for(auto bank:{SourceTitlePaletteBank::objects,SourceTitlePaletteBank::scroll1,SourceTitlePaletteBank::scroll2,SourceTitlePaletteBank::scroll3})source_title_palette_bank(g,m,bank);
}
enum class SourceTitleAction {
 prompt, //5670 x=index
 title_palettes, //20DEE
 palette_background, palette_foreground, palette_logo, palette_characters, //20EC8/20F0E/20F7C/20F4C
 fill_title_map, //2D6AC d0=0,d1=29,a1=x
 background_page, //17BA8 d0=x,a1=y
 intro_music,title_cue,stop_music,reset_music, //2BC90/2B9F6/2B25C/2B218
 idle_credit_marker,fade_in,stage_setup,stage_tick,demo_text,round_labels,
 arcade_palettes,initialize_players,cpu_start_skill,cpu_match_level,
 text_clear,turns,reactions,characters,
 first_pool,second_pool,third_pool,gem_pictures,objects
};
//2D83C intentionally skips the background/board reset at2D834 and2D83E.
template<class M> void clear_source_title_transients(std::uint32_t g,M&m){
 for(unsigned o=0x3300;o<0x4780;o+=4)m.write_long(g+o,0);
 clear_round_records(g,m);initialize_task_pool(g,m);initialize_attack_display_pool(g,m);initialize_character_auxiliary_pool(g,m);
 auto quiet=[](std::uint32_t,std::uint32_t,std::uint32_t){};ActorTasks<M,decltype(quiet)>{m,quiet,g}.reset_display();
}
template<class M,class H> struct SourceTitleScene {
 M&m;H&h;std::uint32_t g;
 unsigned b(unsigned o){return m.read_byte(g+o);}unsigned w(unsigned o){return m.read_word(g+o);}
 void byte(unsigned o,unsigned v){m.write_byte(g+o,v);}void word(unsigned o,unsigned v){m.write_word(g+o,v);}
 void action(SourceTitleAction a,unsigned x=0,unsigned y=0){h.action(a,x,y);}
 void profile(){if(w(0x90)!=2||b(0x86)||b(0xce))throw std::runtime_error("Unbound source title cabinet profile");}
 void flag(unsigned v){m.write_byte(g-0x6ccc,v);}void next(){word(0xc,w(0xc)+2);}
 bool expired(){word(0xe,w(0xe)-1);return std::int16_t(w(0xe))<0;}
 void masked(){word(0x20,0x92a0);word(0x48,0x92a0);}
 void scroll(){const auto value=((m.read_word(g-0x7396)+1)&255)+0x200;m.write_word(g-0x7396,value);word(0x28,value);}
 void coordinates(){word(0x22,0);word(0x24,0);word(0x26,0x200);word(0x28,0x200);word(0x2a,0);word(0x2c,0x700);}
 void layers(){word(0x32,0x1b0e);word(0x3a,0x2461);}
 void palettes(){word(0x128,0);action(SourceTitleAction::palette_background);action(SourceTitleAction::palette_foreground);action(SourceTitleAction::palette_logo);word(0x128,4);action(SourceTitleAction::palette_characters);}
 std::uint32_t actor(unsigned kind){const auto a=allocate_scene_first(g,m);if(a){m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,kind);}return a;}
 void logos(std::initializer_list<unsigned>variants){if(b(0x581c)<variants.size())return;for(auto v:variants)if(!actor(0x2e00+v))throw std::runtime_error("Source title pool invariant");}
 //2DE8. USA region is the source WORD90=2 (the index is a byte offset).
 void prompt(){profile();action(SourceTitleAction::prompt,0x53);action(SourceTitleAction::prompt,m.opcode_word(0x2dfe + w(0x90)));}
 //2D32. The group is all-or-none when fewer than five records remain.
 void picture(){profile();layers();word(0x22,0);word(0x24,0xf0);word(0x26,0);word(0x28,0);word(0x2a,0);word(0x2c,0x700);flag(1);word(0x128,0);action(SourceTitleAction::title_palettes);m.write_word(0x90cfa0,0xffde);action(SourceTitleAction::fill_title_map,0x908000);action(SourceTitleAction::fill_title_map,0x908100);logos({0,1,2,3,4});}
 //348E/34CC use DATA descriptors, including the original first-entry size on clear.
 void name_panel(bool clear){auto p=0x84560+std::int16_t(m.read_word(0x8456c));if(!clear)p+=b(0xb82)*16;p=m.read_long(p+4);const unsigned width=m.read_word(p+2)+1u,height=m.read_word(p+4)+1u;p+=6;for(unsigned row=0;row<height;++row)for(unsigned col=0;col<width;++col){m.write_long(0x902f18+row*0x80+col*4,clear?0x00200000u:m.read_long(p));p+=4;}}
 //344C: source target RGB=FDE; brightness nibble is retained.
 void palette_drift(){if(b(0x81)&3)return;for(unsigned n=0;n<16;++n){auto value=m.read_word(0x90c800+n*2);for(unsigned shift=0;shift<12;shift+=4){const unsigned target=shift==0?14:shift==4?13:15,current=(value>>shift)&15;if(current<target)value+=1u<<shift;else if(current>target)value-=1u<<shift;}m.write_word(0x90c800+n*2,value);}}
 void initial_pair(unsigned side,unsigned character){const auto p=g+0x300+side*0x400,a=g+0xb00+side*0x100;m.write_byte(p,1);for(unsigned o:{2u,0x81u,0xbu})m.write_byte(p+o,side);m.write_byte(p+0x82,character);m.write_byte(p+0x9e,0);m.write_byte(p+0x9f,0);m.write_word(p+0x32,a);m.write_byte(a,1);m.write_byte(a+2,side);m.write_byte(a+0xb,side);m.write_byte(a+3,character);m.write_byte(a+0x82,character);m.write_byte(a+0x9f,0);m.write_word(a+0x30,0xa000);m.write_word(a+0x32,p);}
 //24444 selects by actor subtype3, unlike normal character143F6 by82.
 void character_animation(std::uint32_t a,unsigned index){const auto at=0x24464+m.read_byte(a+3)*4;const auto table=(std::uint32_t(m.opcode_word(at))<<16)|m.opcode_word(at+2);auto quiet=[](std::uint32_t,std::uint32_t,std::uint32_t){};ActorTasks<M,decltype(quiet)>{m,quiet,g}.init(a,index,table);}
 void fixed_boards(){for(unsigned side=0;side<2;++side){const auto p=g+0x300+side*0x400,source=0x3550+side*0x150;m.write_byte(p+0x266,1);m.write_byte(p+0x18e,1);for(unsigned row=0;row<14;++row)for(unsigned n=0;n<3;++n){m.write_long(g+0x2f02+side*0x200+row*16+n*4,m.read_long(source+row*12+0xa8+n*4));m.write_long(g+0x2b02+side*0x200+row*16+n*4,m.read_long(source+row*12+n*4));}}}
 void demo_border_label(){for(unsigned row=0;row<20;++row)for(unsigned col=0;col<6;++col)m.write_long(0x900758+row*0x80+col*4,row==0?0x04f20000u:row==19?0x04ff0000u:0x00200000u);const auto a=actor(0x304);if(!a)return;m.write_byte(a+0xa,0x2f);m.write_word(a+0x10,0xc0);m.write_word(a+0x14,0x30);const auto brightness=m.read_long(0x90c140)&0xf000f000u;for(unsigned n=0;n<8;++n)m.write_long(0x90c140+n*4,m.read_long(0xab860+n*4)|brightness);}
 void gameplay(){action(SourceTitleAction::turns);action(SourceTitleAction::reactions);action(SourceTitleAction::characters);}
 void phase(){switch(w(0xc)){
 case 0:next();word(0xe,0x3c);flag(0);scroll();return;
 case 2:if(expired()){const auto a=actor(0x2e05);if(a){next();word(0xe,0xb4);flag(1);}}scroll();return;
 case 4:if(!expired()){scroll();return;}next();masked();clear_source_title_transients(g,m);byte(0x10d,0);action(SourceTitleAction::stage_tick);action(SourceTitleAction::demo_text);if(b(0x581c)>=2){auto a=actor(0x504);m.write_word(a+0x32,0x8300);a=actor(0x2203);m.write_word(a+0x10,0x120);m.write_word(a+0x14,0x40);m.write_word(a+0xc,0xff0b);m.write_word(a+0x32,0x8300);}byte(0xb01,1);word(0xb10,0x110);word(0xb14,0x58);word(0xbd6,0x58);character_animation(g+0xb00,0x18);name_panel(false);action(SourceTitleAction::arcade_palettes);return;
 case 6:next();word(0x20,0x90c0);word(0xe,0xb4);flag(0);return;
 case 8:if(!expired()){auto quiet=[](std::uint32_t,std::uint32_t,std::uint32_t){};ActorTasks<M,decltype(quiet)>{m,quiet,g}.step(g+0xb00);return;}next();flag(1);masked();clear_source_title_transients(g,m);name_panel(true);layers();coordinates();byte(0xb01,0);logos({0,6,7});palettes();return;
 case 10:next();word(0x20,0x90c0);word(0xe,0xb4);scroll();return;
 case 12:if(!expired()){scroll();return;}next();masked();clear_source_title_transients(g,m);byte(0xdf,1);action(SourceTitleAction::initialize_players);action(SourceTitleAction::cpu_start_skill);action(SourceTitleAction::cpu_match_level);fixed_boards();byte(0x133,1);byte(0x10d,1);action(SourceTitleAction::stage_tick);action(SourceTitleAction::round_labels);action(SourceTitleAction::demo_text);action(SourceTitleAction::arcade_palettes);demo_border_label();return;
 case 14:next();word(0x20,0x90c0);word(0xe,0x12c);flag(0);for(unsigned o:{0x301u,0x701u,0xb01u,0xc01u})byte(o,1);action(SourceTitleAction::turns);word(0x452,4);word(0x454,4);word(0x456,12);word(0x458,11);word(0x852,2);word(0x854,2);word(0x856,2);word(0x858,10);action(SourceTitleAction::reactions);action(SourceTitleAction::characters);return;
 case 16:if(!expired()){for(unsigned o:{0x39au,0x39cu,0x79au,0x79cu})word(o,4);gameplay();return;}next();flag(1);masked();clear_source_title_transients(g,m);layers();coordinates();for(unsigned o:{0x125u,0x133u,0x301u,0x701u,0xb01u,0xc01u})byte(o,0);action(SourceTitleAction::text_clear);palettes();logos({0,1,2,8});return;
 case 18:next();word(0x20,0x90c0);scroll();return;
 case 20:if(std::int8_t(m.read_byte(g-0x6ccc))<0){next();word(0xe,16);word(0x22,0);word(0x24,0xf0);}scroll();return;
 case 22:if(expired()){next();word(0xe,0x168);prompt();action(SourceTitleAction::title_cue);actor(0x2e09);}scroll();return;
 case 24:if(expired())word(8,w(8)+2);palette_drift();scroll();return;
 default:throw std::runtime_error("Unbound source title intro phase");}}
 //2500 simply jumps here. Source83 must already be owned by the idle coordinator.
 void intro(){profile();if(!b(0x83))throw std::runtime_error("Title intro requires source attract flag83");switch(w(8)){
 case 0:if(w(0xc)==0){next();masked();clear_scene_records(g,m,true);for(unsigned o:{0xacu,0x8fu,0x8au,0x8bu,0x4782u,0x133u,0x10cu})byte(o,0);word(0x112,0);action(SourceTitleAction::intro_music);word(0x102,1);const auto index=b(0xe1)&14;byte(0xe1,b(0xe1)+2);word(0x100,m.opcode_word(0x2ed8+index));const auto character=w(0x100)>>1;initial_pair(0,character);initial_pair(1,(character-1)&7);word(0x334,0x8700);word(0x734,0x8300);palettes();return;}if(w(0xc)==2){word(8,w(8)+2);for(unsigned o:{0xcu,0x10u,0x14u})m.write_long(g+o,0);byte(0x10c,0);action(SourceTitleAction::stage_setup);for(unsigned p:{0x904800u,0x904c00u,0x905800u,0x905c00u})action(SourceTitleAction::background_page,0x6d,p);coordinates();m.write_word(g-0x7396,0x200);word(0x20,0x90c0);action(SourceTitleAction::fade_in);return;}throw std::runtime_error("Unbound title intro setup");
 case 2:phase();action(SourceTitleAction::first_pool);action(SourceTitleAction::second_pool);action(SourceTitleAction::gem_pictures);action(SourceTitleAction::objects);return;
 case 4:word(0,w(0)+2);for(unsigned o=4;o<0x18;o+=4)m.write_long(g+o,0);action(SourceTitleAction::stop_music);action(SourceTitleAction::reset_music);return;
 default:throw std::runtime_error("Unbound title intro state");}}
 //2506: title-only entry used by the idle sequence, no child job is created.
 void still(){profile();switch(w(4)){
 case 0:word(4,2);word(0x20,0x90c0);action(SourceTitleAction::idle_credit_marker);picture();prompt();action(SourceTitleAction::title_cue);action(SourceTitleAction::fade_in);return;
 case 2:if(!b(0x4782)){word(4,4);word(6,0x168);}break;
 case 4:word(6,w(6)-1);if(!w(6)){word(0,w(0)+2);m.write_long(g+4,0);}break;
 default:throw std::runtime_error("Unbound static title state");}
 action(SourceTitleAction::first_pool);action(SourceTitleAction::third_pool);action(SourceTitleAction::objects);
 }
};
} // namespace puzzle_fighter
