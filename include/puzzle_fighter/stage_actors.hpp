// SPDX-License-Identifier: GPL-3.0-only
// USA original normal-stage task producers and tile/palette animation runners.
#pragma once
#include "actor_tasks.hpp"
#include <utility>
namespace puzzle_fighter {
template<class M> void initialize_stage_actors(std::uint32_t g,M&m){
 auto list=0x16792u+ActorTasks<M,void(*)(unsigned,unsigned,unsigned)>::signed_word(m.opcode_word(0x16792+m.read_word(g+0x100)));
 while(!(m.read_byte(list)&0x80)){
  if(!m.read_byte(g+0x581c))return;
  m.write_byte(g+0x581c,m.read_byte(g+0x581c)-1);const auto stack=ActorTasks<M,void(*)(unsigned,unsigned,unsigned)>::ptr(m.read_word(g+0x580a));const auto a=ActorTasks<M,void(*)(unsigned,unsigned,unsigned)>::ptr(m.read_word(stack));m.write_word(g+0x580a,std::uint16_t(stack+2));if(!std::uint16_t(stack+2))return;
  m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,m.read_word(list+4));m.write_word(a+0x10,m.read_word(list));m.write_word(a+0x14,m.read_word(list+2));m.write_byte(a+9,m.read_byte(list+6));m.write_byte(a+0xb,m.read_byte(list+7));m.write_word(a+0x30,m.read_word(list+8));list+=16;
 }
}
template<class M,class H> struct StageActors{
 M&m;H&h;std::uint32_t g,a;
 static int sw(std::uint16_t v){return v&0x8000?int(v)-65536:int(v);}
 std::uint32_t cl(std::uint32_t p){return(std::uint32_t(m.opcode_word(p))<<16)|m.opcode_word(p+2);}
 unsigned b(unsigned o){return m.read_byte(a+o);}unsigned w(unsigned o){return m.read_word(a+o);}
 void byte(unsigned o,unsigned v){m.write_byte(a+o,std::uint8_t(v));}void word(unsigned o,unsigned v){m.write_word(a+o,std::uint16_t(v));}
 void next(unsigned o){byte(o,b(o)+2);}bool expired(unsigned o){byte(o,b(o)-1);return b(o)&0x80;}
 unsigned phase(std::uint32_t table){return m.opcode_byte(table+m.read_byte(g+0x10d));}
 void palette(unsigned index){
  const auto offset=index*16;const auto src=cl(0x22ffa+offset+phase(0x22ff2)),dest=cl(0x23006+offset);const auto mask=m.read_long(dest)&0xf000f000;
  for(unsigned n=0;n<8;++n)m.write_long(dest+n*4,m.read_long(src+n*4)|mask);
 }
 void picture_frame(std::uint32_t frame){m.write_long(a+0x2c,frame);m.write_long(a+0x18,m.read_long(frame));m.write_long(a+0x1c,frame+phase(0x23322));}
 void picture_init(unsigned index,std::uint32_t table){picture_frame(table+sw(m.read_word(table+(index&255)*2)));}
 void picture_step(){word(0x18,w(0x18)-1);if(w(0x18))return;auto frame=m.read_long(a+0x2c)+16;if(b(0x1a)&0x80)frame=m.read_long(frame);picture_frame(frame);}
 void map_frame(std::uint32_t frame,bool large,std::uint32_t phases){
  m.write_long(a+0x1c,frame);m.write_long(a+0x18,m.read_long(frame));
  const auto back=g+(large?0x280:0x240);const auto x=std::uint16_t(w(0x10)+m.read_word(back+0x10)),y=std::uint16_t(-std::uint16_t(w(0x14)+m.read_word(back+0x14)));
  const auto offset=large?((x&0x7e0)+((y&0x700)<<3)+((y&0xe0)>>3)):(((x&0x3f0)<<2)+((y&0x300)<<4)+((y&0xf0)>>2));
  auto dest=(large?0x908000u:0x904000u)+offset,source=m.read_long(frame+4+phase(phases));
  source+=2;const auto height=m.read_word(source)+1u;source+=2;const auto width=m.read_word(source)+1u;source+=2;
  for(unsigned col=0;col<width;++col){auto p=dest;
   for(unsigned row=0;row<height;++row){m.write_word(p,std::uint16_t(m.read_word(source)+(large?0x1cc0:0x5800)));m.write_word(p+2,m.read_word(source+2));source+=4;p+=4;
    const auto mask=large?0x1fu:0x3fu;if(!(p&mask)){p-=4;const auto original=p;const auto low=std::uint16_t((original+(large?0x800:0x1000)))&(large?0x3800:0x3000);p=(original&(large?0xffffc7e3u:0xffffcfc3u))|low;}
   }
   dest=(dest&(large?0xfffff800u:0xfffff000u))|((dest+(large?0x20:0x40))&(large?0x7fc:0xffc));
  }
 }
 void map_init(unsigned index,std::uint32_t table,bool large,std::uint32_t phases){map_frame(table+sw(m.read_word(table+(index&255)*2)),large,phases);}
 void map_step(bool large,std::uint32_t phases){word(0x18,w(0x18)-1);if(w(0x18))return;auto frame=m.read_long(a+0x1c)+16;if(b(0x1a)&0x80)frame=m.read_long(frame);map_frame(frame,large,phases);}
 void palette_sub_step(unsigned o){
  word(o+4,w(o+4)-1);if(w(o+4))return;auto frame=m.read_long(a+o)+8;if(b(o+6)&0x80)frame=m.read_long(frame);m.write_long(a+o,frame);m.write_long(a+o+4,m.read_long(frame));
  auto source=m.read_long(frame+4);const auto brightness=m.read_word(0x90c000)&0xf000;
  for(;;){const auto dest=0x90c000u+sw(m.read_word(source));const auto index=m.read_word(source+2),desc=m.read_word(source+4);source+=6;auto rgb=cl(0x2d3a6+sw(index))+(desc&0xfffe);
   for(unsigned n=0;n<16;++n){m.write_word(dest+n*2,std::uint16_t(m.read_word(rgb+n*2)|brightness));}
   if(desc&1)return;
  }
 }
 bool tick(){const auto kind=b(2),state=b(4);
  if(kind==0){switch(state){case 0:next(4);word(0xc,0);byte(0xa,m.read_byte(g+0x10d));{const auto entry=cl(0x23126+b(3)*4);h.init(a,entry>>24,entry&0xffffff);}palette(b(3));return true;
   case 2:if(m.read_byte(g+0x107)){next(4);const auto entry=cl(0x23186+b(3)*4);h.init(a,entry>>24,entry&0xffffff);}[[fallthrough]];
   case 4:if(b(0xa)!=m.read_byte(g+0x10d)){byte(0xa,m.read_byte(g+0x10d));palette(b(3));}h.step(a);h.display(a);return true;case 6:h.free(a);return true;default:return false;}}
  if(kind==1||kind==0x1c){const bool large=kind==0x1c;switch(state){case 0:next(4);byte(1,0xff);map_init(b(3),large?0x8ea82:0x8dc3c,large,large?0x2599a:0x2327a);return true;case 2:map_step(large,large?0x2599a:0x2327a);return true;case 4:case 6:h.free(a);return true;default:return false;}}
  if(kind==2){switch(state){case 0:next(4);word(0xc,0);byte(0xa,m.read_byte(g+0x10d));{const auto entry=cl(0x2332a+b(3)*4);picture_init(entry>>24,entry&0xffffff);}return true;
   case 2:if(b(0xa)!=m.read_byte(g+0x10d)){byte(0xa,m.read_byte(g+0x10d));const auto entry=cl(0x2332a+b(3)*4);picture_init(entry>>24,entry&0xffffff);}picture_step();h.display(a);return true;case 4:case 6:h.free(a);return true;default:return false;}}
  if(kind==6){switch(state){case 0:next(4);word(0xc,0);[[fallthrough]];case 2:picture_init(b(3),0x7c92e);if(m.read_byte(g+0x105)&&!(m.read_byte(g+0x10c)&(1u<<(b(3)&7)))){next(4);picture_init(2+b(3),0x7c92e);}h.display(a);return true;case 4:picture_step();h.display(a);return true;case 6:h.free(a);return true;default:return false;}}
  if(kind==7){switch(state){case 0:next(4);word(0xc,0);byte(0xe,0x60);byte(0xa,m.read_byte(g+0x10d));palette(0);picture_init(4,0x7c92e);return true;
   case 2:switch(b(5)){case 0:word(0x10,w(0x10)-1);if(expired(0xe)){next(5);picture_init(5,0x7c92e);}break;case 2:if(w(0x1a)&0x8000){next(5);byte(0xe,0x60);picture_init(6,0x7c92e);}break;case 4:word(0x10,w(0x10)+1);if(expired(0xe)){next(5);picture_init(5,0x7c92e);}break;case 6:if(w(0x1a)&0x8000){byte(5,0);byte(0xe,0x60);picture_init(4,0x7c92e);}break;default:return false;}picture_step();if(b(0xa)!=m.read_byte(g+0x10d)){byte(0xa,m.read_byte(g+0x10d));palette(0);}h.display(a);return true;case 4:case 6:h.free(a);return true;default:return false;}}
  if(kind==0x1b){switch(state){case 0:next(4);{const auto table=0x2581au;h.palette_start(a,table+sw(m.read_word(table+b(3)*2)));}return true;case 2:if(b(0x1b)&0x80)next(4);else h.palette_step(a);return true;case 4:case 6:h.free(a);return true;default:return false;}}
  if(kind==0x1d){switch(state){case 0:next(4);for(const auto& pair:{std::pair<unsigned,unsigned>{0x40,0x25a14},{0x48,0x25a38},{0x50,0x25a64}}){m.write_long(a+pair.first,pair.second);m.write_long(a+pair.first+4,m.read_long(pair.second));}return true;case 2:palette_sub_step(0x40);palette_sub_step(0x48);palette_sub_step(0x50);return true;case 4:return true;case 6:h.free(a);return true;default:return false;}}
  if(kind==0x36){switch(state){case 0:next(4);byte(1,0xff);byte(0xa,m.read_byte(g+0x10d));palette(16);map_init(2,0x8ea82,true,0x2826e);return true;case 2:if(m.read_byte(g+0x107)){next(4);map_init(3,0x8ea82,true,0x2826e);return true;}[[fallthrough]];case 4:if(b(0xa)!=m.read_byte(g+0x10d)){byte(0xa,m.read_byte(g+0x10d));palette(16);}map_step(true,0x2826e);return true;case 6:h.free(a);return true;default:return false;}}
  return false;
 }
};
} // namespace puzzle_fighter
