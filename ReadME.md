# Chess AI Engine (C++ & Godot GUI)

A robust chess engine built as a DSA project, demonstrating core game tree search algorithms. It features a highly optimized C++ mathematical backend perfectly decoupled from a beautiful graphical frontend built in the Godot Game Engine. Play against an AI that uses Minimax with Alpha-Beta Pruning, Iterative Deepening Search, or the Maia neural network engine.

---

## Features

- **Graphical User Interface** built in Godot, piping commands synchronously to the C++ backend.
- **Three AI modes:**
  - **Minimax + Alpha-Beta Pruning** — classic game tree search
  - **Iterative Deepening Search (IDS)** — smarter move ordering for better pruning
  - **Maia Engine** — neural network that plays like a human (~1200 ELO)
- Full chess rules: castling, en passant, pawn promotion, 50-move draw rule
- Position evaluation with material count, piece-square tables, and king safety
- 1-Dimensional Array indexing caching for optimized game-tree traversal speeds.

---

## Project Structure

```text
DSA project draft/
├── main.cpp           # Entry point: protocol loops and UI bridging
├── types.h            # Core types: Piece, Color, Move, square helpers
├── board.h / .cpp     # Board state, make/unmake move, check detection
├── movegen.h / .cpp   # Legal move generation (with MVV-LVA ordering)
├── evaluator.h / .cpp # Position evaluation heuristic
├── ai_engine.h / .cpp # Minimax with Alpha-Beta Pruning
├── ids_engine.h / .cpp# Iterative Deepening Search
├── maia_engine.h/.cpp # Maia neural network wrapper (Win32 Pipes)
├── maia-1200.pb.gz    # Maia weights file (1200 ELO)
├── chess.exe          # The compiled backend binary
└── gui/               # The Godot Graphical Interface project
    ├── Chess AI - DSA Project.exe  <-- ** RUN THIS FILE FOR THE GRAPHICAL PRODUCT **
    ├── scenes/
    └── scripts/
```

---

## How to Build & Play

### 1. Playing via the Graphical Interface (Godot)
The recommended way to experience the final product is through the Graphical User Interface (GUI).

1. Just double click the "Chess AI - DSA Project.exe" file in the gui folder.
2. Make sure chess.exe, maia-1200.pb.gz, and lc0.exe are in the same folder as the GUI executable.


### 2. Playing via the Terminal (CLI)
If you prefer raw console interaction:

```bash
make        # Build the chess executable
make run    # Build and run immediately
```

Run the compiled binary:
```bash
./chess.exe     # Windows
./chess         # Linux/macOS
```

Enter moves in **from-to** format (e.g. `e2e4`, `g1f3`, `e1g1`).

---

## DSA Concepts Demonstrated

### Minimax (`ai_engine.cpp`)
Builds a recursive game tree where White maximizes and Black minimizes the position score. Searches all moves to a fixed depth and returns the best move.

### Alpha-Beta Pruning (`ai_engine.cpp`)
Skips branches that can't affect the final decision. With good move ordering, this reduces nodes searched from O(b^d) to O(b^(d/2)) — effectively doubling the searchable depth within the same time limit by cutting 90%+ of useless computations.

### Iterative Deepening Search (`ids_engine.cpp`)
Searches incrementally (Depth 1, 2, 3...). The best move from shallower searches is cached in arrays and forced to be evaluated first at the next depth. This establishes a phenomenally strict Alpha boundary early, supercharging pruning efficiency.

### Position Evaluation (`evaluator.cpp`)
Scores a position using:
- **Material count** — standard piece values (pawn=100, queen=900)
- **Piece-square tables** — Constant grid matrices awarding bonuses for geometric positioning (e.g., knights in the center +20)
- **Dynamic Phase Shifting** — King logic autonomously shifts at < 2600 active piece value to incentivize central attacking.

### Depth-First Backtracking (`board.cpp`)
To avoid memory fragmentation and CPU bottlenecking by dynamically spawning 64-index Arrays for every node tested, the engine applies moves utilizing `makeMove()`, pushes an `UndoInfo` struct to a Stack, and reverses it natively with `unmakeMove()`. 

---

## Maia Neural Network (Optional)

Maia is a neural network trained to predict moves a human player at 1200 ELO would make. This project spans OS threads via the Universal Chess Interface (UCI) protocol to map these probabilities.

**Requirements:**
- [lc0 (Leela Chess Zero)](https://lczero.org/) installed
- `maia-1200.pb.gz` weights file

The Win32 IPC wrapper spawns `lc0` as a background process, communicates via OS hook pipes, and uses `go nodes 1` to get raw predictions mathematically mapping human blunders.

---

## Dependencies

- C++17 Standard Library
- Windows API (`<windows.h>`) for IPC pipe mapping and external `.exe` execution.
- Godot Engine 4.x (to run the graphical client)