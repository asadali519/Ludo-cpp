# Ludo Board Game — SFML C++

A fully functional **Ludo board game** written in C++ using the **SFML 2.x graphics library**.  
Single-file implementation with no STL containers, no structs/classes — all state is held in global C-style arrays and all logic is in plain functions.

---

## Features

- **Real 15×15 Ludo board** — coloured yards, home columns, safe-spot stars, and centre home triangle
- **Four players**: Red, Green, Yellow, Blue
- **Two game modes**:
  - *Player vs Player* — all four players controlled by humans at the same keyboard/mouse
  - *Player vs Computer* — choose your colour; the other three are AI
- **Graphical dice** drawn with SFML shapes (no image files required)
- **Dice roll animation** cycling through faces before settling
- **Full rule set**:
  - Roll a 6 to exit base
  - Rolling 6 grants another turn
  - Three consecutive 6s cancels all moves from those turns (state is restored)
  - Capturing an opponent sends it back to base and grants a bonus turn
  - Safe spots (spawn cells + star cells) — no captures allowed there
  - Home column entry and exact-landing finish requirement
  - Finishing a piece grants a bonus turn
  - First player to finish all 4 pieces wins
- **Screen state machine**: Main Menu → Colour Select → Game → Win → Pause → Settings
- **Save / Load** — saves full game state to `ludo_save.dat`
- **Game log** showing the last few events
- **AI** with timer-based delays and basic strategy (prefers captures and finishes)

---

## Prerequisites

- **g++** with C++17 support (GCC 7+ recommended)
- **SFML 2.x** development libraries

### Install SFML on Ubuntu / Debian

```bash
sudo apt-get install libsfml-dev
```

### Install SFML on Fedora / RHEL

```bash
sudo dnf install SFML-devel
```

### Install SFML on macOS (Homebrew)

```bash
brew install sfml
```

---

## Build

```bash
cd sfml-ludo-game
g++ -std=c++17 -O2 -o ludo_sfml main.cpp \
    $(pkg-config --cflags --libs sfml-graphics sfml-window sfml-system)
```

Or manually if `pkg-config` is not available:

```bash
g++ -std=c++17 -O2 -o ludo_sfml main.cpp \
    -lsfml-graphics -lsfml-window -lsfml-system
```

---

## Run

```bash
./ludo_sfml
```

A save file `ludo_save.dat` is created in the **current working directory** when you save.

---

## Controls

| Action | Input |
|--------|-------|
| Navigate menus | Left mouse click |
| Roll dice | Click **Roll Dice** button |
| Select a piece to move | Click the highlighted (yellow-ringed) piece |
| Pause / Resume | Click **Pause** button in-game |
| Save game | Pause → **Save Game** |
| Load game | Main Menu → **Load Game** |
| Toggle sound setting | Settings → **Sound: ON/OFF** |

---

## How to Play

1. Launch the game.
2. From the **Main Menu**, choose a game mode:
   - *Player vs Player* — all four players take turns manually.
   - *Player vs Computer* — pick your colour; the rest are AI-controlled.
3. On your turn, click **Roll Dice**.
4. After the dice settles, pieces you can legally move will be highlighted with a yellow ring.  
   Click one of them to move it.
5. **Special rules**:
   - You need to roll a **6** to move a piece out of the base yard.
   - Rolling a **6** gives you an extra turn.
   - Rolling **three 6s in a row** cancels all three turns (the board is restored to before the first 6).
   - Landing on an opponent's piece on a **non-safe** cell sends it back to base and gives you a bonus turn.
   - A piece enters its coloured **home column** after completing one full circuit (≈51 steps from spawn).
   - A piece must land **exactly** on the last home column cell to finish; overshooting is not allowed.
   - Finishing a piece gives you a bonus turn.
6. The **first player to finish all 4 pieces wins**.

---

## Code Style

- All game state lives in **global C-style arrays** (no heap allocation, no STL containers).
- No `struct` or `class` definitions — all logic is in free functions.
- `encode(p, pc)` / `decode(code, p, pc)` pack a player+piece identity into a single `int`.
- `pieceState[p][pc]`: `0`=base, `1`=on track, `2`=in home column, `3`=finished.
- `pieceMoves[p][pc]`: cumulative steps from spawn (0–51 = main track, 52–57 = home column).
- `saveState()` / `restoreState()` snapshot/restore the board for the three-sixes rule.
- Screen state machine driven by `currentScreen` (0–5).
