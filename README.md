# Overview
UCI-compatible chess engine.
Supports standard chess (tested) and crazyhouse (not fully tested).

### Search
- Copy-make
- Iterative deepening
- Transposition table
- Alpha-beta w/ aspiration windows
- Late move reductions
- History + killer heuristics
- Multithreading using Lazy SMP (untested)

### Evaluation
- Material counting
- Mobility
- Pawn structure considering doubled, isolated, backwards, and passed pawns

# Usage
Requires a UCI-compatible chess interface, such as Scid or CuteChess, to be used easily.

# Building
```
git clone --recurse-submodules https://github.com/sb362/chess-engine.git
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

# Dependencies
- A C++17 capable compiler
- [{fmt}](https://github.com/fmtlib/fmt) (submodule)
- [Catch2](https://github.com/catchorg/Catch2) (submodule, used for testing)
