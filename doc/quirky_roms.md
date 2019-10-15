# Specific roms

octopeg: relies on atomic vf behavior, 8xy5 takes vf as an operand and writing the carry flag before computing the addition will cause unexpected results

binding of cosmac: re-implements fx29 ('glyph')... relying on the implementation detail that the 4x8 font starts at address 0 and that subsequent digits appear every 5 bytes...

# General quirkiness

- Many roms (mostly modern?) have instructions with mixed alignment. Some are aligned to 2 bytes, some appear at odd addresses, making disassembly difficult.