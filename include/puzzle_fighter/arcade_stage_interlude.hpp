// SPDX-License-Identifier: GPL-3.0-only
// USA21128/2150E and their finite stage-four scene actor closure.
#pragma once
#include "actor_tasks.hpp"
#include "character_special_tasks.hpp"
#include "character_cosmetic_tasks.hpp"
#include "sound_requests.hpp"
namespace puzzle_fighter {
namespace stage_interlude_detail {
inline int sw(std::uint16_t v){return std::int16_t(v);}inline std::uint32_t ptr(std::uint16_t v){return std::uint32_t(sw(v));}
template<class M> std::uint32_t allocate(std::uint32_t g,M&m,bool second=false){const auto count=g+(second?0x581d:0x581c),head=g+(second?0x580c:0x580a);if(!m.read_byte(count))return 0;m.write_byte(count,m.read_byte(count)-1);const auto stack=ptr(m.read_word(head)),actor=ptr(m.read_word(stack));m.write_word(head,stack+2);return actor;}
template<class M> void palette(M&m,std::uint32_t from,std::uint32_t to,unsigned blocks){for(unsigned n=0;n<blocks*32;n+=4)m.write_long(to+n,m.read_long(from+n)|0xf000f000u);}
}
template<class M> bool initialize_stage_interlude_palettes(std::uint32_t g,M&m){
 using namespace stage_interlude_detail;const auto scene=m.read_byte(g+0x120);if(scene>6)return false;
 auto character=[&](unsigned ch,unsigned alt,std::uint32_t to){palette(m,0xa0000+(ch*2+alt)*128,to,4);};
 switch(scene){case 0:character(1,0,0x90c100);character(0,0,0x90c180);character(0,1,0x90c200);break;case 1:character(0,0,0x90c100);character(10,0,0x90c180);break;case 2:character(2,0,0x90c100);character(3,0,0x90c180);palette(m,0xa9720,0x90c1a0,1);break;case 3:character(10,0,0x90c100);character(4,0,0x90c180);break;case 4:character(10,0,0x90c100);character(2,0,0x90c180);character(7,0,0x90c200);break;case 5:character(6,0,0x90c100);character(4,0,0x90c180);character(5,0,0x90c200);break;case 6:character(9,0,0x90c100);palette(m,0xa9fc0,0x90c1a0,3);break;}return true;
}
template<class M> bool tick_stage_interlude_scene(std::uint32_t g,M&m){
 using namespace stage_interlude_detail;const auto phase=m.read_word(g+0x10);if(!phase){m.write_word(g+0x10,2);m.write_word(g+0x14,0);return initialize_stage_interlude_palettes(g,m);}if(phase==4){if(m.read_byte(g+0x106))m.write_word(g+0xc,m.read_word(g+0xc)+2);return true;}if(phase!=2)return false;
 const auto scene=m.read_byte(g+0x120);if(scene>6)return false;const auto sub=m.read_word(g+0x14);if(sub==2){const auto timer=std::uint16_t(m.read_word(g+0x16)-1);m.write_word(g+0x16,timer);if(!timer)m.write_word(g+0x14,4);return true;}if(sub==4){m.write_word(g+0x10,4);return true;}if(sub)return false;
 constexpr unsigned counts[]={3,2,3,2,4,4,7},kinds[]={0x34,0x37,0x38,0x39,0x3a,0x3d,0x3f};const auto count=counts[scene];if(m.read_byte(g+0x581c)<count)return true;m.write_word(g+0x14,2);m.write_word(g+0x16,420);
 for(unsigned n=0;n<count;++n){const auto a=allocate(g,m);m.write_byte(a,m.read_byte(a)+1);m.write_word(a+2,(kinds[scene]<<8)|n);
  if(scene==3){m.write_byte(a+0xb,n);m.write_byte(a+9,n?6:8);}if(scene==4){m.write_byte(a+0xb,n?1:0);m.write_byte(a+9,n==1?6:8);if(!n)m.write_byte(a+0xe,0x30);}if(scene==5){m.write_byte(a+0xb,(n==0||n==2)?1:0);if(!n)m.write_byte(a+0xe,0x30);}
  if(scene!=6||!n)m.write_word(g-0x6cda+n*2,a);
 }return true;
}
// Original 2CE3C and 2CDF8/2CE1A wrappers. Sound register-save scratch is
// transport state; the queue triples and both source gates are preserved.
template<class M> bool queue_stage_interlude_cue(std::uint32_t g,M&m,unsigned index){
 if(m.read_byte(g+0xbc))return false;
 return queue_sound_request(g,m,{m.opcode_word(0x2ce60+std::uint16_t(index*2)),0x1000,0},true);
}
template<class M> bool queue_stage_interlude_voice(std::uint32_t g,M&m,bool second){
 if(m.read_byte(g+0xbc))return false;
 return queue_sound_request(g,m,{second?0x334u:0x134u,second?0x1400u:0xc00u,0},true);
}
template<class M,class H> struct StageInterludeActor {
 M&m;H&h;std::uint32_t g,a;std::uint32_t work=0;
 static int sw(std::uint16_t v){return std::int16_t(v);}static std::uint32_t ptr(std::uint16_t v){return std::uint32_t(sw(v));}
 unsigned b(unsigned o){return m.read_byte(a+o);}unsigned w(unsigned o){return m.read_word(a+o);}std::uint32_t l(unsigned o){return m.read_long(a+o);}
 void byte(unsigned o,unsigned v){m.write_byte(a+o,std::uint8_t(v));}void word(unsigned o,unsigned v){m.write_word(a+o,std::uint16_t(v));}void lng(unsigned o,std::uint32_t v){m.write_long(a+o,v);}void next(unsigned o=5){byte(o,b(o)+2);}bool down(unsigned o=0xe){byte(o,b(o)-1);return !b(o);}
 unsigned cb(std::uint32_t p){auto v=m.opcode_word(p&~1u);return p&1?v&255:v>>8;}std::uint32_t cl(std::uint32_t p){return(std::uint32_t(m.opcode_word(p))<<16)|m.opcode_word(p+2);}
 std::uint32_t peer(unsigned n){return ptr(m.read_word(g-0x6cda+n*2));}unsigned peer_byte(unsigned n,unsigned o){return m.read_byte(peer(n)+o);}void peer_flag(unsigned n,unsigned o,unsigned value){m.write_byte(peer(n)+o,value);}
 void init(unsigned index,std::uint32_t table){h.init(a,index,table);}void render(){h.step(a);h.display(a);}bool negative(){return b(0x1a)&0x80;}bool event(){if(!b(0x1b))return false;byte(0x1b,0);return true;}
 void sound(unsigned index){queue_stage_interlude_cue(g,m,index);}void side_voice(bool second){queue_stage_interlude_voice(g,m,second);}
 bool spawn(unsigned kind,bool parent=false){work=stage_interlude_detail::allocate(g,m);if(!work)return false;m.write_byte(work,m.read_byte(work)+1);m.write_word(work+2,kind);if(parent)m.write_word(work+0x32,a);return true;}
 void attachment(){const auto child=stage_interlude_detail::allocate(g,m,true);if(!child)return;m.write_byte(child,m.read_byte(child)+1);m.write_byte(child+2,8);m.write_word(child+0x32,a);word(0x38,child);}
 void effect(unsigned type,int dx=0,int dy=0){if(type==0x1300){spawn(type,true);return;}if(type==0x2001){for(unsigned n=0;n<4;++n)if(!spawn(type,true))return;return;}if(type==0xf00){if(!spawn(type,true))return;m.write_byte(work+0x28,255);m.write_word(work+0x10,w(0x10)+(b(0xb)?-dx:dx));m.write_word(work+0x14,w(0x14)+dy);return;}throw std::runtime_error("Unsupported interlude effect");}
 void dust(unsigned variant){if(down(0xf)){byte(0xf,4);spawn(0x3500+variant,true);}}
 void spark(unsigned variant){for(unsigned phase:{0x10u,0x24u,0x34u}){if(!spawn(0x3e00+variant,true))return;m.write_byte(work+0xe,0);m.write_byte(work+0xa,phase);}}
 void move(){lng(0x10,l(0x10)+l(0x20));lng(0x20,l(0x20)+l(0x28));lng(0x14,l(0x14)+l(0x24));lng(0x24,l(0x24)+l(0x2c));}
 bool actor34(){const auto variant=b(3),sub=b(5);if(!b(4)){next(4);attachment();byte(0xb,0);word(0x30,0xa000);byte(9,6);byte(0xa,0);word(0x10,m.opcode_word(0x27eaa+variant*2));word(0xc,m.opcode_word(0x27eb0+variant*2));word(0x14,0x58);if(!variant){byte(0xb,1);byte(0xe,0x40);}init(0,variant?0x54ca4:0x57dde);return true;}
  if(variant==0){switch(sub){case 0:if(down()){next();init(10,0x57dde);}break;case 2:if(negative()){next();byte(0xa,255);init(0,0x57dde);}else if(event())sound(6);break;case 4:if(peer_byte(2,0xa)){next();init(15,0x57dde);}break;case 6:if(event()){sound(8);if(spawn(0x3c03)){m.write_word(work+0x10,w(0x10)-0x26);m.write_word(work+0x14,w(0x14)+0x43);}else init(15,0x57dde);}break;default:return false;}}
  else if(variant==1){switch(sub){case 0:if(peer_byte(2,0xa)){next();init(17,0x54ca4);}break;case 2:if(event())side_voice(true);break;default:return false;}}
  else if(variant==2){switch(sub){case 0:if(peer_byte(0,0xa)){next();byte(0xe,32);const auto delta=std::uint16_t((m.read_word(peer(0)+0x10)-m.read_word(peer(1)+0x10))*2);lng(0x20,std::uint32_t(std::int32_t(std::uint32_t(delta)<<16)>>4));init(20,0x54ca4);}break;case 2:lng(0x10,l(0x10)+l(0x20));if(negative()){next();byte(0xa,255);byte(0xb,b(0xb)^1);init(17,0x54ca4);}break;case 4:if(event())side_voice(false);break;default:return false;}}else return false;
 render();return true;
 }
 bool actor37(){const auto variant=b(3),sub=b(5);if(!b(4)){next(4);attachment();byte(0xb,0);word(0x30,0xa000);byte(9,6);byte(0xe,64);byte(0xa,0);word(0x10,m.opcode_word(0x282fa+variant*2));word(0xc,m.opcode_word(0x282feu+variant*2));word(0x14,0x58);init(variant?9:0,variant?0x7ab6c:0x54ca4);return true;}
  if(!variant){switch(sub){case 0:if(down()){next();byte(0xe,80);init(16,0x54ca4);}break;case 2:if(down()){next();word(0x10,w(0x10)+16);byte(0xe,80);init(17,0x54ca4);}break;case 4:case 6:if(event())sound(sub==4?3:4);if(down()){next();word(0x10,w(0x10)+16);if(sub==4)byte(0xe,80);else byte(0xa,255);init(sub==4?18:19,0x54ca4);}break;case 8:if(event())sound(5);break;default:return false;}}
  else if(variant==1){switch(sub){case 0:if(peer_byte(0,0xa))next();break;case 2:word(0x10,w(0x10)-4);if(sw(w(0x10))<=256){next();word(0x10,256);init(16,0x7ab6c);}break;case 4:break;default:return false;}}else return false;
 render();return true;
 }
 // Source roots 2845C, 28830, 28AAC, 28FD6, 2966A.
 bool actor38();bool actor39();bool actor3a();bool actor3d();bool actor3f();
 bool spark_task(){const auto parent=ptr(w(0x32));auto position=[&]{const auto base=0x29556u+b(0xe)*4;auto dx=m.read_word(base);if(m.read_byte(parent+0xb))dx=std::uint16_t(-dx);const auto circle=0x2956au+b(0xa)*4;word(0x10,m.read_word(parent+0x10)+dx+m.read_word(circle));word(0x14,m.read_word(parent+0x14)+m.read_word(base+2)+m.read_word(circle+2));};auto frame=[&]{byte(9,4);byte(0xb,1);unsigned index=b(3)*2+32;if(b(0xa)>>5){byte(9,8);byte(0xb,0);++index;}init(index,0x81766);};
  if(!b(4)){next(4);word(0x30,0xe000);position();frame();return true;}if(m.read_byte(parent+0x50)){next(4);return true;}byte(0xa,(b(0xa)+1)&63);position();frame();h.display(a);return true;
 }
 bool tick(){const auto kind=b(2),state=b(4);if(kind!=0x34&&kind!=0x37&&kind!=0x38&&kind!=0x39&&kind!=0x3a&&kind!=0x3c&&kind!=0x3d&&kind!=0x3e&&kind!=0x3f)return false;
  if(state==6){h.free(a);return true;}if(state==4){next(4);return true;}if(state!=0&&state!=2)return false;
  switch(kind){case 0x34:return actor34();case 0x37:return actor37();case 0x38:return actor38();case 0x39:return actor39();case 0x3a:return actor3a();case 0x3d:return actor3d();case 0x3f:return actor3f();case 0x3e:return spark_task();case 0x3c:if(!state){next(4);word(0x30,0xa000);byte(9,4);byte(0xb,0);word(0xc,0);byte(0x50,0);init(std::uint8_t(b(3)+12),0x81766);}else if(b(0x50))next(4);else render();return true;default:return false;}
 }
};
template<class M,class H> bool stage_interlude_attachment(std::uint32_t a,std::uint32_t g,M&m,H&h){using namespace stage_interlude_detail;if(m.read_byte(a+2)!=8)return false;const auto parent=ptr(m.read_word(a+0x32)),state=m.read_byte(a+4);if(state==6){if(m.read_word(parent+0x38)==std::uint16_t(a))m.write_word(parent+0x38,0);free_character_second_pool(a,g,m);return true;}if(state==4){m.write_byte(a+4,6);return true;}if(state!=0&&state!=2)return false;if(!state){m.write_byte(a+4,2);m.write_word(a+0x30,m.read_word(parent+0x30));m.write_byte(a+3,m.read_byte(parent+0x82));m.write_word(a+0xc,0);m.write_byte(a+0xa,0);m.write_word(a+0x14,0x58);}m.write_byte(a+1,0);const auto frame=m.read_byte(m.read_long(parent+0x1c)+14);if(!frame){m.write_byte(a+0xa,0);return true;}m.write_word(a+0x10,m.read_word(parent+0x10));m.write_byte(a+0xb,m.read_byte(parent+0xb));m.write_byte(a+1,1);if(frame!=m.read_byte(a+0xa)){m.write_byte(a+0xa,frame);h.init(a,frame,0x817b8);}return true;}
template<class M,class H> bool stage_interlude_actor(std::uint32_t a,std::uint32_t g,M&m,H&h){return StageInterludeActor<M,H>{m,h,g,a}.tick();}
template<class M,class H> bool StageInterludeActor<M,H>::actor38(){
 const auto v=b(3),s=b(5);auto flight=[&]{if(std::int32_t(l(0x20))<0){lng(0x10,l(0x10)+l(0x20));lng(0x20,l(0x20)+l(0x28));}lng(0x14,l(0x14)+l(0x24));lng(0x24,l(0x24)+l(0x2c));};
 if(!b(4)){next(4);if(v!=2)attachment();byte(0xb,v==0);word(0x30,0xa000);byte(9,6);byte(0xa,0);word(0xe,0);word(0x10,m.opcode_word(0x28506+v*2));word(0xc,m.opcode_word(0x2850c+v*2));word(0x14,v==2?0x51:0x58);init(cb(0x284f6+v),cl(0x284fa+v*4));return true;}
 if(v==0){switch(s){case 0:if(negative()){next();init(0,0x5b4ae);}break;case 2:if(peer_byte(1,0xa)){next();init(16,0x5b4ae);}break;case 4:if(peer_byte(1,0xe)){next();byte(0xe,64);sound(0x16);if(spawn(0x3c01)){m.write_word(work+0x10,w(0x10)-0x26);m.write_word(work+0x14,w(0x14)+0x43);word(0x32,work);}init(17,0x5b4ae);}break;case 6:if(down()){next();m.write_byte(ptr(w(0x32))+0x50,255);if(spawn(0x3c05)){m.write_word(work+0x10,w(0x10)-0x26);m.write_word(work+0x14,w(0x14)+0x43);}init(4,0x5b4ae);}break;case 8:break;default:return false;}}
 else if(v==1){switch(s){case 0:next();lng(0x20,0);lng(0x28,0);lng(0x24,0x60000);lng(0x2c,0xffff8000);word(0x50,0x58);break;
 case 2:if(negative()){next();word(0x14,0x58);lng(0x20,0xfffc0000);lng(0x28,0x4000);lng(0x24,0x60000);lng(0x2c,0xffff8000);byte(0xf,1);sound(0x14);init(12,0x5ecb2);}else{if(b(0x1b))flight();if(b(0x1a)==1){byte(0x1a,0);sound(0x12);}}break;
 case 4:case 8:dust(1);if(!b(0x1b))flight();if(std::int32_t(l(0x24))<0){next();init(14,0x5ecb2);}break;
 case 6:dust(1);flight();if(sw(w(0x50))>=sw(w(0x14))){next();word(0x14,w(0x50));lng(0x20,0xfffc0000);lng(0x28,0x4000);lng(0x24,0x80000);lng(0x2c,0xffff8000);init(12,0x5ecb2);}break;
 case 10:dust(1);flight();if(sw(w(0x50))>=sw(w(0x14))){word(0x14,w(0x50));byte(0xa,255);next();init(22,0x5ecb2);}break;
 case 12:if(negative()){sound(16);next();byte(0xe,255);}break;case 14:break;default:return false;}}
 else if(v==2){if(s==0){if(peer_byte(1,0xa)){next();sound(0x15);init(30,0x5ecf2);}}else if(s!=2)return false;}else return false;
 render();return true;
}
template<class M,class H> bool StageInterludeActor<M,H>::actor39(){
 const auto v=b(3),s=b(5);if(!b(4)){next(4);attachment();word(0x30,0xa000);byte(0xa,0);word(0x10,m.opcode_word(0x288a6+v*2));word(0xc,m.opcode_word(0x288aa+v*2));word(0x14,0x58);init(cb(0x2889a+v),cl(0x2889eu+v*4));return true;}
 if(!v){switch(s){case 0:if(sw(m.read_word(peer(1)+0x10))>0xc0){next();init(21,0x617dc);}break;case 2:if(sw(m.read_word(peer(1)+0x10))<0xc0){next();init(20,0x617dc);}break;case 4:if(std::int32_t(m.read_long(peer(1)+0x20))>=0&&sw(m.read_word(peer(1)+0x10))>0x70){next();byte(0xa,0);init(11,0x617dc);}break;case 6:if(!b(0x1b)){next();byte(0xa,255);sound(1);spawn(0x1601,true);}break;case 8:if(negative()){next();init(22,0x617dc);}break;case 10:break;default:return false;}}
 else if(v==1){switch(s){case 0:next();lng(0x20,0x40000);lng(0x24,0);lng(0x28,0);lng(0x2c,0);sound(0);break;case 2:case 4:move();if((s==2&&sw(w(0x10))>0x1c0)||(s==4&&sw(w(0x10))< -64)){next();byte(0xb,b(0xb)^1);lng(0x20,0-l(0x20));sound(0);}break;case 6:move();if(peer_byte(0,0xa)){next();sound(2);effect(0x1300);byte(0xe,64);lng(0x20,0xfffb0000);lng(0x24,0x28000);lng(0x2c,0xffffe000);init(8,0x7ab6c);}break;case 8:move();if(down())next();break;case 10:break;default:return false;}}else return false;
 render();return true;
}
template<class M,class H> bool StageInterludeActor<M,H>::actor3a(){
 const auto v=b(3),s=b(5);auto peerevent=[&](unsigned n){return m.read_byte(m.read_long(peer(n)+0x1c)+8)!=0;};
 if(!b(4)){next(4);if(v!=3)attachment();word(0x30,0xa000);byte(0xa,0);word(0x10,m.opcode_word(0x28b32+v*2));word(0xc,m.opcode_word(0x28b3a+v*2));word(0x14,0x58);init(cb(0x28b1eu+v),cl(0x28b22+v*4));return true;}
 if(v==0){switch(s){case 0:if(down()){next();byte(0xf,1);init(15,0x5b4ae);}break;case 2:dust(1);if(event()){next();effect(0xf00,20,26);}break;case 4:dust(1);if(negative()){next();byte(0xa,255);init(0,0x5b4ae);}else if(event()){sound(0x17);if(spawn(0x1201,true))m.write_byte(work+0x28,255);}break;case 6:if(peer_byte(2,0xa)){next();byte(0xb,b(0xb)^1);byte(0xe,64);init(17,0x5b4ae);}break;case 8:if(down()){next();init(4,0x5b4ae);}break;case 10:break;default:return false;}}
 else if(v==1){switch(s){case 0:if(peer_byte(0,0xa)){next();sound(0x18);byte(0xe,48);if(spawn(0x3c00)){word(0x32,work);m.write_word(work+0x10,w(0x10)+0x30);m.write_word(work+0x14,w(0x14)+0x43);}init(16,0x6e9bc);}break;case 2:if(down()){next();m.write_byte(ptr(w(0x32))+0x50,255);byte(0xf,1);init(18,0x6e9bc);}break;case 4:dust(2);if(event()){next();effect(0xf00,13,34);if(spawn(0x1402,true))m.write_byte(work+0xa,255);}break;case 6:dust(2);if(event()){next();sound(0x19);spawn(0x1205,true);}break;case 8:dust(2);if(b(0x1a)){next();init(4,0x6e9bc);}break;case 10:break;default:return false;}}
 else if(v==2){switch(s){case 0:if(peerevent(1)){next();byte(0xa,255);effect(0x1300);byte(0xb,b(0xb)^1);init(8,0x7ab6c);}break;case 2:if(negative()){next();byte(0xe,64);sound(0x1a);lng(0x20,0x60000);lng(0x24,0x2a000);lng(0x2c,0xffffe000);init(8,0x7ab6c);}else if(peerevent(1)){effect(0x1300);init(8,0x7ab6c);}break;case 4:move();if(down())next();break;case 6:break;default:return false;}}
 else if(v==3){switch(s){case 0:if(peerevent(0)){next();effect(0x1300);byte(0xe,64);lng(0x20,0xfffb0000);lng(0x24,0x28000);lng(0x2c,0xffffe000);init(5,0x5b5b8);}break;case 2:move();if(down())next();break;case 4:break;default:return false;}}else return false;
 render();return true;
}
template<class M,class H> bool StageInterludeActor<M,H>::actor3d(){
 const auto v=b(3),s=b(5);auto peerevent=[&]{return peer_byte(0,0xb)!=b(0xb)&&m.read_byte(m.read_long(peer(0)+0x1c)+8);};
 if(!b(4)){next(4);word(0x30,0xa000);if(v<3)attachment();byte(0xa,0);byte(9,cb(0x2904f+v*2));word(0xc,m.opcode_word(0x29080+v*2));word(0x10,m.opcode_word(0x2906c+v*4));word(0x14,m.opcode_word(0x2906eu+v*4));init(cb(0x2904eu+v*2),cl(0x29058+v*4));return true;}
 if(v==0){switch(s){case 0:if(down()){next();sound(9);init(14,0x617dc);}break;case 2:if(b(0x1a)){next();byte(0xb,b(0xb)^1);sound(9);init(14,0x617dc);}else if(event()){m.write_byte(g+0x124,48);sound(10);effect(0x2001);}break;case 4:if(b(0x1a)){next();byte(0xe,48);spawn(0x3d04,true);sound(15);init(22,0x617dc);}else if(event()){m.write_byte(g+0x124,48);sound(10);effect(0x2001);}break;case 6:if(down()){next();byte(0xa,255);sound(17);spark(1);init(19,0x617dc);}break;case 8:break;default:return false;}}
 else if(v==1||v==2){const auto table=v==1?0x6747au:0x6c290u;switch(s){case 0:if(peerevent()){next();effect(0x1300);init(8,table);}break;case 2:if(negative()){next();if(v==1){sound(12);sound(13);spark(0);init(16,table);}else{spark(3);sound(14);init(5,table);}}else if(peerevent()){effect(0x1300);init(8,table);}break;case 4:break;default:return false;}}
 else if(v==3){switch(s){case 0:if(peer_byte(0,0xa)){next();byte(0xe,56);init(4,0x674ba);}break;case 2:if(down()){next();if(spawn(0x3c04)){m.write_word(work+0x10,w(0x10)+24);m.write_word(work+0x14,w(0x14)+48);byte(0x50,0);}}break;case 4:break;default:return false;}}
 else if(v==4){switch(s){case 0:lng(0x14,l(0x14)-0x60000);if(sw(w(0x14))<0x98){next();word(0x14,0x98);lng(0x20,0xfffc0000);lng(0x28,0x1000);lng(0x24,0x50000);lng(0x2c,0xffffa000);sound(16);init(18,0x61826);}break;case 2:if(std::int32_t(l(0x20))<0){lng(0x10,l(0x10)+l(0x20));lng(0x20,l(0x20)+l(0x28));}lng(0x14,l(0x14)+l(0x24));lng(0x24,l(0x24)+l(0x2c));if(sw(w(0x14))<0x58)next(4);break;default:return false;}}else return false;
 render();return true;
}
template<class M,class H> bool StageInterludeActor<M,H>::actor3f(){
 const auto v=b(3),s=b(5);if(v>6)return false;if(!b(4)){next(4);if(!v)attachment();word(0x30,0xa000);byte(0xb,0);byte(9,cb(0x296e1+v*2));word(0xc,m.opcode_word(0x29726+v*2));word(0x10,m.opcode_word(0x2970a+v*4));word(0x14,m.opcode_word(0x2970c+v*4));init(cb(0x296e0+v*2),cl(0x296eeu+v*4));return true;}
 if(!v){if(s==0){if(event()){next();if(spawn(0x3c04)){m.write_word(work+0x10,w(0x10)+24);m.write_word(work+0x14,w(0x14)+64);byte(0x50,0);}}}else if(s!=2)return false;}else if(s!=0)return false;render();return true;
}
// Normal stage caption requests use indices 0 and 1. Alternate font entries
// (bit 7 set) belong to other scenes and are rejected explicitly here.
template<class M> bool draw_stage_interlude_caption(unsigned index,M&m){
 if(index>1)return false;
 std::uint32_t source=0x20b7a+std::int16_t(m.read_word(0x20b7a+index*2));
 for(unsigned lines=0;lines<128;++lines){const unsigned column=m.read_byte(source++),row=m.read_byte(source++);std::uint32_t to=0x900000+std::int16_t(std::uint16_t(column*128+row));const auto attr=std::uint16_t(std::int16_t(std::int8_t(m.read_byte(source++))));
  for(unsigned glyphs=0;glyphs<256;++glyphs){const auto ch=m.read_byte(source++);if(!ch)return true;if(ch==0x7f)break;const auto code=std::uint8_t(ch-32);const auto tile=(code&128)?0:m.read_word(0x5a12+code*2);if(tile){m.write_word(to,tile);m.write_word(to+2,attr);m.write_word(to+4,tile+16);m.write_word(to+6,attr);m.write_word(to+128,tile+1);m.write_word(to+130,attr);m.write_word(to+132,tile+17);m.write_word(to+134,attr);}else for(auto off:{0u,4u,128u,132u})m.write_long(to+off,0x20000000);to+=256;}
 }throw std::runtime_error("Stage caption data did not terminate");
}

} // namespace puzzle_fighter
