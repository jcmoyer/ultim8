# ultim8

A highly configurable chip-8 emulator and assembler.

# Dependencies

- SDL2
- gl3w
- imgui
- fmt
- catch2
- toml11

Currently, these libraries are vendored for convenience and reproducibility.

# Building

### Windows

For Visual Studio you can simply clone the repository, open the directory as a cmake project, and build.

### Linux and similar environments

Requires a C++ compiler, GNU make, git, and cmake.

```
git clone https://github.com/jcmoyer/ultim8.git
mkdir ultim8-build && cd ultim8-build
cmake ../ultim8
make
```

# Things that still need to be implemented

- Compatibility settings for older roms
- Some superchip and XO extensions are still missing, but a lot of games work
- Numeric range checking, negative integer literals for assembler

# License

Apache License, Version 2.0

See LICENSE.txt for more information.