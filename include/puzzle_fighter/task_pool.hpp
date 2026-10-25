// SPDX-License-Identifier: GPL-3.0-only
// USA 2D8FA..2D92F: first 32 x 128-byte task pool and its free stack.
#pragma once
#include <cstdint>
namespace puzzle_fighter {
template<class Memory>
void initialize_task_pool(std::uint32_t globals,Memory&m) {
    auto stack=globals+0x5866u;
    auto actor=globals+0xd00u;
    m.write_byte(globals+0x581cu,0x20);
    for(unsigned i=0;i<32;++i,actor+=0x80u){
        for(unsigned j=0;j<32;++j)m.write_long(actor+j*4u,0);
        m.write_byte(actor+8u,8);
        stack-=2u;m.write_word(stack,static_cast<std::uint16_t>(actor));
    }
    m.write_word(globals+0x580au,static_cast<std::uint16_t>(stack));
}
} // namespace puzzle_fighter
