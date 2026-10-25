// SPDX-License-Identifier: GPL-3.0-only
// USA1550..1750 plus1E31C/1E346: source coin/filter/credit work in IRQ order.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
template<class M,class H> void source_coin_operator_count(std::uint32_t g,M&m,H&h,bool service){
 const auto counter=g+(service?0xc4:0xc0),cache=g-(service?0x700c:0x7010);const auto total=std::uint32_t(m.read_long(counter)+1);m.write_long(counter,total);
 if(m.read_byte(g+0x86)||total==m.read_long(cache))return;
 m.write_long(cache,total);m.write_word(g-0x6f6c,1);m.write_word(g-0x6f68,service?0x64:0x60);h.persist_operator_counter();
}
template<class M,class H> bool source_coin_poll(std::uint32_t g,M&m,H&h){
 if(m.read_byte(g+0x82))return true;
 if(m.read_byte(g+0x86))return false; // Active battery-statistics hardware unbound.
 const auto history=[&](unsigned offset,unsigned bit){unsigned result=0;for(unsigned n=0;n<4;++n)result=(result<<1)|((m.read_byte(g+offset+3-n)>>bit)&1);return result;};
 m.write_byte(g+0x75,history(0x60,4));m.write_byte(g+0x7d,history(0x60,5));m.write_byte(g+0x6c,history(0x64,2));
 const auto timer=m.read_byte(g+0x6f);if(timer){m.write_byte(g+0x6f,timer-1);if(std::uint8_t(timer-1)==0x20)m.write_byte(g+0x6b,m.read_byte(g+0x6b)&0xfe);}
 else if(m.read_byte(g+0x6e)){m.write_byte(g+0x6e,m.read_byte(g+0x6e)-1);m.write_byte(g+0x6b,m.read_byte(g+0x6b)|1);m.write_byte(g+0x6f,0x40);}
 for(unsigned side=0;side<(m.read_byte(g+0x9f)?2u:1u);++side){auto a=g+0x70 + side*8;
  if(!m.read_byte(a)){if(m.read_byte(a+5)==3){m.write_byte(a,1);m.write_byte(a+4,50);}continue;}
  if(m.read_byte(a+5)!=12){m.write_byte(a+4,m.read_byte(a+4)-1);if(!m.read_byte(a+4))m.write_byte(a,0);continue;}
  m.write_byte(a,0);source_coin_operator_count(g,m,h,false); //1F070 exits with86=0.
  if(!m.read_byte(g+0x9e))a=g+0x70;
  m.write_byte(g+0x6e,m.read_byte(g+0x6e)+1);const auto old=m.read_byte(a+1);const auto partial=std::uint8_t(old+1);m.write_byte(a+1,partial);
  if(!m.read_byte(g+0x9d)&&std::int8_t(partial)<std::int8_t(m.read_byte(a+2))){if(m.read_byte(a+6)==9)m.write_byte(a+1,0);m.write_byte(g-0x739a,m.read_byte(g-0x739a)+1);continue;}
  //16C0 preserves X from the partial-count ADDQ, including byte overflow.
  const unsigned have=m.read_byte(a+6),payout=m.read_byte(a+3),x=old==255;unsigned sum=have+payout+x;if((have&15)+(payout&15)+x>9)sum+=6;const bool carry=sum>0x99;if(!carry)m.write_byte(a+6,std::uint8_t(sum));
  if(m.read_byte(a+6)>9){m.write_byte(a+6,9);}m.write_byte(a+1,0);m.write_byte(g-0x7399,m.read_byte(g-0x7399)+1);
 }
 if(!m.read_byte(g+0x9c)&&m.read_byte(g+0x6c)==3){source_coin_operator_count(g,m,h,true); //1F0BA exits with86=0.
  for(unsigned side=0;side<=m.read_byte(g+0x9e);++side){const auto a=g+0x70 + side*8;auto credits=std::uint8_t(m.read_byte(a+6)+1);if(credits>9)credits=9;m.write_byte(a+6,credits);}m.write_byte(g-0x7399,m.read_byte(g-0x7399)+1);
 }return true;
}
template<class M,class H> bool source_coin_requests(std::uint32_t g,M&m,H&h){
 bool requested=false;if(m.read_byte(g-0x739a)){requested=true;m.write_byte(g-0x739a,m.read_byte(g-0x739a)-1);h.coin_sound(0xe0);}
 if(m.read_byte(g-0x7399)){requested=true;m.write_byte(g-0x7399,m.read_byte(g-0x7399)-1);h.coin_sound(0xe1);}
 //16EE keeps D0=1 after either request, so the balance ORs cannot suppress
 // the83/paid refresh.54D0 is a one-pass job atE0, after main and before fade.
 if(requested&&m.read_byte(g+0x83)&&!m.read_byte(g+0x9c)){h.request_credit_display();}return true;
}
} // namespace puzzle_fighter
