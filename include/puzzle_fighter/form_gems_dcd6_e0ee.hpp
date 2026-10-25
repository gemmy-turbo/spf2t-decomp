// SPDX-License-Identifier: GPL-3.0-only
// USA DCD6..E0ED and E0EE..E505, including the actual E1CE -> DDC2 handoff.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
struct FormGemsDcd6E0eeResult{std::uint32_t formations=0,cross_handoffs=0;};
namespace form_gems_dcd6_e0ee_detail {
constexpr bool signed_le(std::uint32_t a,std::uint32_t b){return (a^0x80000000u)<=(b^0x80000000u);}
constexpr bool signed_word_lt(std::uint16_t a,std::uint16_t b){return (a^0x8000u)<(b^0x8000u);}
template<class Memory>
struct Access {
    Memory&m;
    std::uint32_t pointer(std::uint32_t a){const auto high=m.read_word(a);const auto low=m.read_word(a+2u);return (std::uint32_t(high)<<16)|low;}
    void move_pointer(std::uint32_t a,std::uint32_t v){m.write_word(a,static_cast<std::uint16_t>(v>>16));m.write_word(a+2u,static_cast<std::uint16_t>(v));}
    void add_pointer(std::uint32_t a,std::uint32_t n){const auto v=pointer(a)+n;m.write_word(a+2u,static_cast<std::uint16_t>(v));m.write_word(a,static_cast<std::uint16_t>(v>>16));}
    void add_word(std::uint32_t a,std::uint16_t n){m.write_word(a,static_cast<std::uint16_t>(m.read_word(a)+n));}
};
template<class Memory>
bool candidate(std::uint32_t p,std::uint32_t anchor,std::uint32_t picture,bool&second,FormGemsDcd6E0eeResult&result,Memory&m){
    Access<Memory>a{m};
    m.write_byte(p+0x232u,0);m.write_byte(p+0x230u,0);m.write_byte(p+0x233u,0);m.write_word(p+0x236u,0);
    const auto initial=m.read_word(anchor);
    if(!initial||initial==255u||(initial&15u)==6u||(initial&15u)==7u)return false;
    const auto initial_id=m.read_byte(anchor+0x400u);
    if(initial_id){
        if(m.read_byte(picture+1u)!=(second?4u:1u))return false;
        if(m.read_byte(p+0x230u)&&initial_id!=m.read_byte(p+0x230u))return false;
        a.add_word(p+0x236u,1);m.write_byte(p+0x233u,1);
        const auto id=m.read_byte(anchor+0x400u);m.write_byte(p+0x230u,id);
    }
    const auto colour=m.read_word(anchor)&15u;
    auto upper=anchor;
    a.move_pointer(p+0x21au,anchor);a.move_pointer(p+0x222u,anchor);
    a.move_pointer(p+0x21eu,picture);a.move_pointer(p+0x226u,picture);
    std::uint16_t vertical=10;
    do{
        upper-=16u;a.add_pointer(p+0x226u,0xfffffff0u);
        auto meta=a.pointer(p+0x226u);
        if((m.read_word(meta)&15u)==(second?4u:1u)){
            if(m.read_byte(p+0x230u)){
                const auto group=m.read_byte(upper+0x400u);
                if(!group){
                    // E1CE really transfers into DDC2. All subsequent work,
                    // including later candidates/columns, uses the first body.
                    if(second){second=false;++result.cross_handoffs;}
                }else if(group!=m.read_byte(p+0x230u))break;
            }
            m.write_byte(p+0x233u,1);const auto id=m.read_byte(upper+0x400u);m.write_byte(p+0x230u,id);
        }
        if(m.read_word(upper)&128u){
            if(!m.read_byte(p+0x233u))break;
            const auto id=m.read_byte(upper+0x400u);if(id&&id!=m.read_byte(p+0x230u))break;
        }
        const auto cell_colour=m.read_word(upper)&15u;
        if(!cell_colour||cell_colour==7u||cell_colour!=colour)break;
        if(signed_le(upper,a.pointer(p+0x398u)))break;
        meta=a.pointer(p+0x226u);a.add_word(p+0x236u,m.read_word(meta)&15u);
        vertical=static_cast<std::uint16_t>(vertical-1u);
    }while(vertical!=0xffffu);
    const auto height=static_cast<std::uint16_t>(10u-vertical);
    if(!height)return false;
    m.write_word(p+0x1e6u,height);m.write_word(p+0x386u,static_cast<std::uint16_t>(height-1u));
    auto column=a.pointer(p+0x21au);std::uint16_t horizontal=5;
    do{
        const auto step=second?0xfffffffeu:2u;column+=step;a.add_pointer(p+0x21eu,step);
        const auto group=m.read_byte(column+0x400u);
        if(group){
            auto meta=a.pointer(p+0x21eu);
            if(m.read_byte(meta+1u)==(second?4u:1u)){
                if(m.read_byte(p+0x230u)&&group!=m.read_byte(p+0x230u))break;
                m.write_byte(p+0x233u,1);const auto id=m.read_byte(column+0x400u);m.write_byte(p+0x230u,id);
            }
            if(!m.read_byte(p+0x233u)||group!=m.read_byte(p+0x230u))break;
        }
        const auto cell_colour=m.read_word(column)&15u;if(cell_colour==7u)break;
        auto meta=a.pointer(p+0x21eu);
        if(m.read_byte(meta+1u)==(second?8u:2u)&&m.read_byte(p+0x233u))break;
        if(cell_colour!=colour)break;
        meta=a.pointer(p+0x21eu);a.add_word(p+0x236u,m.read_word(meta)&255u);
        auto cell=column;const auto saved=a.pointer(p+0x21eu);a.move_pointer(p+0x22au,saved);
        auto rows=m.read_word(p+0x1e6u);
        do{
            cell-=16u;a.add_pointer(p+0x22au,0xfffffff0u);
            const auto base_group=m.read_byte(column+0x400u);
            if(base_group&&(!m.read_byte(p+0x233u)||base_group!=m.read_byte(p+0x230u)))break;
            const auto value=m.read_word(cell)&15u;if(value==7u||value!=colour)break;
            const auto id=m.read_byte(cell+0x400u);if(id&&id!=m.read_byte(p+0x230u))break;
            if(signed_le(cell,a.pointer(p+0x398u)))break;
            meta=a.pointer(p+0x22au);a.add_word(p+0x236u,m.read_word(meta)&15u);
            meta=a.pointer(p+0x22au);
            if(m.read_byte(meta+1u)==(second?4u:1u)){
                bool copy=true;
                if(m.read_byte(p+0x230u)){const auto id_at_base=m.read_byte(column+0x400u);if(id_at_base!=m.read_byte(p+0x230u))copy=false;}
                if(copy){m.write_byte(p+0x233u,1);const auto new_id=m.read_byte(cell+0x400u);m.write_byte(p+0x230u,new_id);}
            }
            rows=static_cast<std::uint16_t>(rows-1u);
        }while(rows!=0xffffu);
        if(rows==m.read_word(p+0x1e6u))break;
        const auto current=m.read_word(p+0x1e6u);const auto count=static_cast<std::uint16_t>(current-rows);
        if(signed_word_lt(count,m.read_word(p+0x1e6u)))m.write_word(p+0x1e6u,count);
        horizontal=static_cast<std::uint16_t>(horizontal-1u);
    }while(horizontal!=0xffffu);
    auto width=static_cast<std::uint16_t>(5u-horizontal);
    const auto final_height=m.read_word(p+0x1e6u);
    if(!width)return false;
    if(m.read_byte(p+0x233u)){
        m.write_word(p+0x23cu,width);m.write_word(p+0x236u,0);a.move_pointer(p+0x21eu,picture);
        do{
            auto meta=a.pointer(p+0x21eu);auto rows=final_height;
            do{a.add_word(p+0x236u,m.read_byte(meta+1u));meta-=16u;rows=static_cast<std::uint16_t>(rows-1u);}while(rows!=0xffffu);
            a.add_pointer(p+0x21eu,second?0xfffffffeu:2u);width=static_cast<std::uint16_t>(width-1u);
        }while(width!=0xffffu);
        width=m.read_word(p+0x23cu);const auto sum=m.read_word(p+0x236u);
        if(sum&&(sum&255u)!=15u&&(sum&255u)!=30u)return false;
    }
    a.move_pointer(p+0x21au,anchor);a.move_pointer(p+0x21eu,picture);
    m.write_byte(p+0x32cu,static_cast<std::uint8_t>(m.read_byte(p+0x32cu)+1u));
    const auto serial=m.read_byte(p+0x32cu);m.write_byte(p+0x32cu,serial);if(!serial)m.write_byte(p+0x32cu,1);
    m.write_byte(p+0x194u,static_cast<std::uint8_t>(m.read_byte(p+0x194u)+1u));
    m.write_byte(p+0x194u,m.read_byte(p+0x194u)&7u);
    if(m.read_byte(p+0x194u)==7u)m.write_byte(p+0x194u,1);
    else if(!m.read_byte(p+0x194u))m.write_byte(p+0x194u,1);
    auto id=m.read_byte(p+0x32cu);m.write_byte(anchor+0x400u,id);
    m.write_word(anchor,m.read_word(anchor)|128u);m.write_byte(picture+1u,second?8:1);
    m.write_byte(p+0x22eu,0);m.write_byte(p+0x22fu,0);
    std::uint32_t last_meta=0;
    do{
        auto cell=a.pointer(p+0x21au);auto meta=a.pointer(p+0x21eu);auto rows=final_height;
        do{
            if(!(m.read_word(cell)&128u))m.write_byte(p+0x267u,1);
            m.write_word(cell,m.read_word(cell)|128u);m.write_word(meta,0);
            id=m.read_byte(p+0x32cu);m.write_byte(meta,id);
            if(!width&&!m.read_byte(p+0x22fu)){m.write_byte(meta+1u,second?1:4);m.write_byte(p+0x22fu,1);}
            cell-=16u;meta-=16u;rows=static_cast<std::uint16_t>(rows-1u);
        }while(rows!=0xffffu);
        if(!m.read_byte(p+0x22eu)){m.write_byte(meta+17u,second?8:2);m.write_byte(p+0x22eu,1);}
        a.add_pointer(p+0x21au,second?0xfffffffeu:2u);a.add_pointer(p+0x21eu,second?0xfffffffeu:2u);
        last_meta=meta;width=static_cast<std::uint16_t>(width-1u);
    }while(width!=0xffffu);
    m.write_byte(last_meta+17u,second?2:8);m.write_byte(p+0x171u,1);
    id=m.read_byte(p+0x32cu);m.write_byte(anchor+0x400u,id);
    m.write_word(anchor,m.read_word(anchor)|128u);m.write_byte(picture+1u,second?8:1);
    return true;
}
template<class Memory>
FormGemsDcd6E0eeResult run(std::uint32_t p,bool second,Memory&m){
    Access<Memory>a{m};FormGemsDcd6E0eeResult result;
    m.write_byte(p+0x267u,0);
    const auto field=a.pointer(p+0x164u);a.move_pointer(p+0x212u,field+(second?0xccu:0xc2u));
    const auto picture=a.pointer(p+0x168u);a.move_pointer(p+0x216u,picture+(second?0xccu:0xc2u));
    m.write_word(p+0x238u,0);
    for(;;){auto anchor=a.pointer(p+0x212u);auto meta=a.pointer(p+0x216u);std::uint16_t rows=10;
        do{result.formations+=candidate(p,anchor,meta,second,result,m);anchor-=16u;meta-=16u;rows=static_cast<std::uint16_t>(rows-1u);}while(rows!=0xffffu);
        a.add_pointer(p+0x212u,second?0xfffffffeu:2u);a.add_pointer(p+0x216u,second?0xfffffffeu:2u);
        a.add_word(p+0x238u,1);
        if(!signed_word_lt(m.read_word(p+0x238u),7u))break;
    }
    if(m.read_byte(p+0x267u))m.write_byte(p+0x266u,1);
    return result;
}
}
template<class Memory>FormGemsDcd6E0eeResult form_gems_dcd6(std::uint32_t p,Memory&m){return form_gems_dcd6_e0ee_detail::run(p,false,m);}
template<class Memory>FormGemsDcd6E0eeResult form_gems_e0ee(std::uint32_t p,Memory&m){return form_gems_dcd6_e0ee_detail::run(p,true,m);}
template<class Memory>FormGemsDcd6E0eeResult form_gems_dcaa(std::uint32_t p,Memory&m){if(m.read_byte(p+0x31au))return {};return form_gems_dcd6(p,m);}
template<class Memory>FormGemsDcd6E0eeResult form_gems_dcb6(std::uint32_t p,Memory&m){if(m.read_byte(p+0x31au))return {};return form_gems_e0ee(p,m);}
}
