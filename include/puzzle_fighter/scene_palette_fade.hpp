// SPDX-License-Identifier: GPL-3.0-only
// Native typed continuations for source1F62/1FA8/1FEE and1A14 modes0/0C.
#pragma once
#include <cstdint>
#include <stdexcept>
namespace puzzle_fighter {
template<class M> void source_palette_fade_step(std::uint32_t g,M&m,bool fade_in){
 m.write_byte(g+0x4780,0);const auto target=fade_in?0xf000u:0u,step=fade_in?0x1000u:0xf000u;
 for(unsigned bank=0;bank<4;++bank){const auto base=0x90c000+bank*0x400;if((m.read_word(base)&0xf000)==target){m.write_byte(g+0x4780,m.read_byte(g+0x4780)|(1u<<bank));continue;}for(unsigned n=0;n<512;++n)m.write_word(base+n*2,std::uint16_t(m.read_word(base+n*2)+step));}
 if(m.read_byte(g+0x4780)==15)m.write_byte(g+0x4781,0);
}
template<class M> void source_palette_fade_out_step(std::uint32_t g,M&m){source_palette_fade_step(g,m,false);}
template<class M> void source_palette_fade_in_step(std::uint32_t g,M&m){source_palette_fade_step(g,m,true);}
struct SourceScenePaletteFade {
 bool active=false,starting=false,waiting=false,queued=false,fade_in=false,requested_in=false;
 template<class M> void launch(std::uint32_t g,M&m,bool into,bool screen=true){m.write_byte(g+0x4782,0xff);active=starting=true;fade_in=into;queued=false;if(into&&screen){m.write_word(g+0x20,0x90c0);m.write_word(g+0x48,0x90c0);}}
 template<class M> void request_direction(std::uint32_t g,M&m,bool into){if(waiting)throw std::runtime_error("Duplicate source palette wait");waiting=true;requested_in=into;if(m.read_byte(g+0x4782)){if(!active)throw std::runtime_error("Unbound pre-existing palette worker");queued=true;return;}launch(g,m,into);}
 template<class M> void request(std::uint32_t g,M&m){request_direction(g,m,false);}
 template<class M> void request_in(std::uint32_t g,M&m){request_direction(g,m,true);}
 //1FEE is asynchronous and tests4781, not4782, before replacing slot100.
 template<class M> bool request_out_background(std::uint32_t g,M&m){if(m.read_byte(g+0x4781))return false;if(waiting)throw std::runtime_error("Palette caller still suspended");launch(g,m,false);return true;}
 //2012 does not change20/48; the scene owns those writes.
 template<class M> bool request_in_background(std::uint32_t g,M&m){if(m.read_byte(g+0x4781))return false;if(waiting)throw std::runtime_error("Palette caller still suspended");launch(g,m,true,false);return true;}
 template<class M> bool resume_after_irq(std::uint32_t g,M&m){if(!waiting)return false;if(queued){if(!m.read_byte(g+0x4782))launch(g,m,requested_in);return false;}if(starting||m.read_byte(g+0x4781))return false;waiting=false;if(!fade_in){m.write_word(g+0x20,0x92a0);m.write_word(g+0x48,0x92a0);}return true;}
 template<class M> void after_main(std::uint32_t g,M&m){if(!active)return;if(starting){m.write_byte(g+0x4781,0xff);starting=false;}if(!m.read_byte(g+0x4781)){m.write_byte(g+0x4782,0);active=false;return;}source_palette_fade_step(g,m,fade_in);}
};
} // namespace puzzle_fighter
