// SPDX-License-Identifier: GPL-3.0-only
// Key layout adapted from cps2recomp romset.cpp (Tommy McLeroy, 2026),
// following the MAME CPS2 key-file layout credited in THIRD_PARTY_NOTICES.md.
#include "cps2crypt.h"
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

static std::vector<std::uint8_t> read(const char* path, std::size_t size) {
    std::ifstream file(path, std::ios::binary);
    std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(file)), {});
    if (!file || bytes.size() != size) throw std::runtime_error("Unexpected ROM input size");
    return bytes;
}
static void write(const char* path, const std::vector<std::uint16_t>& words) {
    std::ofstream file(path, std::ios::binary);
    for (auto word : words) { file.put(char(word >> 8)); file.put(char(word)); }
    file.close();
    if (!file) throw std::runtime_error("Cannot write program view");
}
int main(int argc, char** argv) try {
    if (argc != 5) throw std::runtime_error("Usage: pf-rom-views PROGRAM KEY DATA_OUT OPCODE_OUT");
    const auto raw = read(argv[1], 0x100000), key_file = read(argv[2], 20);
    std::array<std::uint16_t, 10> decoded{};
    for (unsigned b = 0; b < 160; ++b) {
        const auto bit = (317 - b) % 160;
        if ((key_file[bit / 8] >> ((bit ^ 7) % 8)) & 1) decoded[b / 16] |= 0x8000 >> (b % 16);
    }
    const std::uint32_t key[] = {std::uint32_t(decoded[0]) << 16 | decoded[1], std::uint32_t(decoded[2]) << 16 | decoded[3]};
    const auto end = (((~decoded[9] & 0x3ffu) << 14) | 0x3fffu) + 1;
    std::vector<std::uint16_t> data(raw.size() / 2), opcodes(data.size());
    for (std::size_t n = 0; n < data.size(); ++n) data[n] = std::uint16_t(raw[n * 2 + 1]) << 8 | raw[n * 2];
    // MAME's upper cipher word is inclusive; the vendored API takes an
    // exclusive endpoint. The extra word preserves the verified USA view.
    cps2recomp::DecryptProgram(data.data(), opcodes.data(), data.size(), key, 0, end / 2 + 1);
    write(argv[3], data); write(argv[4], opcodes);
    std::cout << "Exported DATA and OPCODE views\n";
    return 0;
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
