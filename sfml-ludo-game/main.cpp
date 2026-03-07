// =============================================================
// Ludo Board Game — SFML 2.x
// Single-file implementation: no STL containers, no structs/classes,
// all state in global C-style arrays, all logic in free functions.
// =============================================================

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdio>
using namespace std;

// =============================================================
// CONSTANTS
// =============================================================
const int PLAYERS    = 4;   // Red=0, Green=1, Yellow=2, Blue=3
const int PIECES     = 4;   // 4 pieces per player
const int TOTAL_MAIN = 52;  // cells on the main circular track
const int HOME_SIZE  = 6;   // cells in each player's home column (0..5, 5=finish)

// pieceMoves semantics:
//   0..51  → on main track, track position = (spawnIndex[p] + pieceMoves) % 52
//   52..57 → in home column, home-col index = pieceMoves - 52
//   57     → finished (triggers pieceState = 3)

const float CELL_SIZE     = 52.f;   // pixel size of each board cell
const float GRID_OFFSET_X = 60.f;   // board left margin
const float GRID_OFFSET_Y = 65.f;   // board top margin

const unsigned int WIN_WIDTH  = 900;
const unsigned int WIN_HEIGHT = 1000;

// Dice animation
const float ANIM_DURATION    = 0.75f;  // total animation duration (s)
const float ANIM_FRAME_SPEED = 0.07f;  // seconds per frame change

// AI delay before acting (seconds)
const float AI_ROLL_DELAY = 0.6f;
const float AI_MOVE_DELAY = 0.4f;

// =============================================================
// BOARD LAYOUT — 52-cell main track (row, col in 15×15 grid)
// =============================================================
const int trackRow[52] = {
    // 0-4:  bottom arm going up, col 6
    13, 12, 11, 10, 9,
    // 5-10: row 8 going left
    8,  8,  8,  8,  8,  8,
    // 11-12: up col 0
    7,  6,
    // 13-17: row 6 going right
    6,  6,  6,  6,  6,
    // 18-22: col 6 going up
    5,  4,  3,  2,  1,
    // 23-25: top row going right
    0,  0,  0,
    // 26-30: col 8 going down
    1,  2,  3,  4,  5,
    // 31-36: row 6 going right
    6,  6,  6,  6,  6,  6,
    // 37-38: col 14 going down
    7,  8,
    // 39-43: row 8 going left
    8,  8,  8,  8,  8,
    // 44-48: col 8 going down
    9,  10, 11, 12, 13,
    // 49-51: row 14 going left
    14, 14, 14
};

const int trackCol[52] = {
    // 0-4
    6,  6,  6,  6,  6,
    // 5-10
    5,  4,  3,  2,  1,  0,
    // 11-12
    0,  0,
    // 13-17
    1,  2,  3,  4,  5,
    // 18-22
    6,  6,  6,  6,  6,
    // 23-25
    6,  7,  8,
    // 26-30
    8,  8,  8,  8,  8,
    // 31-36
    9,  10, 11, 12, 13, 14,
    // 37-38
    14, 14,
    // 39-43
    13, 12, 11, 10, 9,
    // 44-48
    8,  8,  8,  8,  8,
    // 49-51
    8,  7,  6
};

// Home column cells for each player (6 cells, index 5 = finish)
// Red:    col 7, rows 13→8
// Green:  row 7, cols 1→6
// Yellow: col 7, rows 1→6
// Blue:   row 7, cols 13→8
const int homeRow[PLAYERS][HOME_SIZE] = {
    { 13, 12, 11, 10,  9,  8 },  // Red
    {  7,  7,  7,  7,  7,  7 },  // Green
    {  1,  2,  3,  4,  5,  6 },  // Yellow
    {  7,  7,  7,  7,  7,  7 }   // Blue
};
const int homeCol[PLAYERS][HOME_SIZE] = {
    {  7,  7,  7,  7,  7,  7 },  // Red
    {  1,  2,  3,  4,  5,  6 },  // Green
    {  7,  7,  7,  7,  7,  7 },  // Yellow
    { 13, 12, 11, 10,  9,  8 }   // Blue
};

// Base positions (starting squares inside each yard)
// Red yard:    rows 9-14, cols 0-5  → pieces at (10,1),(10,3),(12,1),(12,3)
// Green yard:  rows 0-5,  cols 0-5  → pieces at (1,1),(1,3),(3,1),(3,3)
// Yellow yard: rows 0-5,  cols 9-14 → pieces at (1,10),(1,12),(3,10),(3,12)
// Blue yard:   rows 9-14, cols 9-14 → pieces at (10,10),(10,12),(12,10),(12,12)
const int baseRow[PLAYERS][PIECES] = {
    { 10, 10, 12, 12 },  // Red
    {  1,  1,  3,  3 },  // Green
    {  1,  1,  3,  3 },  // Yellow
    { 10, 10, 12, 12 }   // Blue
};
const int baseCol[PLAYERS][PIECES] = {
    {  1,  3,  1,  3 },  // Red
    {  1,  3,  1,  3 },  // Green
    { 10, 12, 10, 12 },  // Yellow
    { 10, 12, 10, 12 }   // Blue
};

// Spawn index on main track for each player
const int spawnIndex[PLAYERS] = { 0, 13, 26, 39 };

// Safe spots: spawn indices + star positions (no capturing allowed)
// Star positions: 8, 21, 34, 47
const int safeSpots[8] = { 0, 8, 13, 21, 26, 34, 39, 47 };

// =============================================================
// PLAYER COLOURS
// =============================================================
const sf::Color playerColor[PLAYERS] = {
    sf::Color(210,  45,  45),   // Red
    sf::Color( 40, 170,  40),   // Green
    sf::Color(210, 190,  30),   // Yellow
    sf::Color( 40,  90, 210)    // Blue
};
const sf::Color playerDark[PLAYERS] = {
    sf::Color(140,  15,  15),
    sf::Color( 10, 110,  10),
    sf::Color(140, 130,  10),
    sf::Color( 10,  50, 140)
};
const sf::Color playerLight[PLAYERS] = {
    sf::Color(255, 180, 180),
    sf::Color(180, 255, 180),
    sf::Color(255, 255, 160),
    sf::Color(180, 200, 255)
};

// =============================================================
// GAME STATE — all global, C-style arrays
// =============================================================

// Screen state machine
//   0=Menu  1=ColorSelect  2=Game  3=Win  4=Pause  5=Settings
int currentScreen = 0;
int gameMode      = 0;   // 0=PvP, 1=PvC
int humanPlayerID = 0;   // which player the human controls in PvC
int winnerID      = -1;
bool soundEnabled = true;

// Piece state: 0=base  1=onTrack  2=inHomeColumn  3=finished
int  pieceState[PLAYERS][PIECES];
// piecePos: track index (0-51) when state==1; home-col index (0-5) when state==2
int  piecePos[PLAYERS][PIECES];
// pieceMoves: cumulative steps from spawn (drives track & home-col position)
int  pieceMoves[PLAYERS][PIECES];
// hasKilled: whether this piece earned a bonus this turn (bookkeeping)
bool hasKilled[PLAYERS][PIECES];

// mainOccupant[i] = encode(p,pc) of the last piece placed at track cell i, or -1
int mainOccupant[TOTAL_MAIN];

// Backup arrays for three-sixes cancellation
int  bk_pieceState[PLAYERS][PIECES];
int  bk_piecePos[PLAYERS][PIECES];
int  bk_pieceMoves[PLAYERS][PIECES];
int  bk_mainOccupant[TOTAL_MAIN];

// Turn state
int  currentPlayer    = 0;
int  currentDice      = 1;
int  consecutiveSixes = 0;
bool waitingForRoll   = true;
bool waitingForMove   = false;
bool bonusTurn        = false;

// Highlighted pieces (valid moves the current player can make)
int highlightedPieces[PIECES];
int highlightedCount  = 0;

// Finished-piece counter per player
int finishedCount[PLAYERS];

// Dice animation
sf::Clock diceAnimClock;
bool  diceAnimating = false;
float diceAnimTime  = 0.f;
float diceFrameAcc  = 0.f;
int   diceAnimFace  = 1;

// AI timers
sf::Clock aiClock;
bool aiThinking = false;  // waiting to roll
bool aiMoving   = false;  // waiting to move after roll

// Game log
const int LOG_MAX = 7;
char gameLog[LOG_MAX][160];
int  logCount = 0;

// =============================================================
// ENCODE / DECODE  (piece identity packed into a single int)
// =============================================================
int encode(int p, int pc)             { return p * 10 + pc; }
void decode(int code, int &p, int &pc){ p = code / 10; pc = code % 10; }

// =============================================================
// LOG
// =============================================================
void logMsg(const char *msg)
{
    if (logCount < LOG_MAX) {
        snprintf(gameLog[logCount], sizeof(gameLog[logCount]), "%s", msg);
        logCount++;
    } else {
        for (int i = 0; i < LOG_MAX - 1; i++)
            memmove(gameLog[i], gameLog[i + 1], sizeof(gameLog[i]));
        snprintf(gameLog[LOG_MAX - 1], sizeof(gameLog[LOG_MAX - 1]), "%s", msg);
    }
}

const char *playerName(int p)
{
    if (p == 0) return "Red";
    if (p == 1) return "Green";
    if (p == 2) return "Yellow";
    return "Blue";
}

// =============================================================
// INIT / SAVE / RESTORE
// =============================================================
void initializeGame()
{
    for (int p = 0; p < PLAYERS; p++) {
        finishedCount[p] = 0;
        for (int pc = 0; pc < PIECES; pc++) {
            pieceState[p][pc] = 0;
            piecePos[p][pc]   = 0;
            pieceMoves[p][pc] = 0;
            hasKilled[p][pc]  = false;
        }
    }
    for (int i = 0; i < TOTAL_MAIN; i++) mainOccupant[i] = -1;

    currentPlayer    = 0;
    currentDice      = 1;
    consecutiveSixes = 0;
    waitingForRoll   = true;
    waitingForMove   = false;
    bonusTurn        = false;
    winnerID         = -1;
    logCount         = 0;
    diceAnimating    = false;
    diceAnimTime     = 0.f;
    diceFrameAcc     = 0.f;
    aiThinking       = false;
    aiMoving         = false;
    highlightedCount = 0;
}

void saveState()
{
    for (int p = 0; p < PLAYERS; p++)
        for (int pc = 0; pc < PIECES; pc++) {
            bk_pieceState[p][pc] = pieceState[p][pc];
            bk_piecePos[p][pc]   = piecePos[p][pc];
            bk_pieceMoves[p][pc] = pieceMoves[p][pc];
        }
    for (int i = 0; i < TOTAL_MAIN; i++)
        bk_mainOccupant[i] = mainOccupant[i];
}

void restoreState()
{
    for (int p = 0; p < PLAYERS; p++) {
        finishedCount[p] = 0;
        for (int pc = 0; pc < PIECES; pc++) {
            pieceState[p][pc] = bk_pieceState[p][pc];
            piecePos[p][pc]   = bk_piecePos[p][pc];
            pieceMoves[p][pc] = bk_pieceMoves[p][pc];
            if (pieceState[p][pc] == 3) finishedCount[p]++;
        }
    }
    for (int i = 0; i < TOTAL_MAIN; i++)
        mainOccupant[i] = bk_mainOccupant[i];
}

// =============================================================
// SAFE-SPOT CHECK
// =============================================================
bool isSafeSpot(int trackIdx)
{
    for (int i = 0; i < 8; i++)
        if (safeSpots[i] == trackIdx) return true;
    return false;
}

// =============================================================
// TRACK OCCUPANCY HELPERS
// =============================================================
// Count pieces (any player) currently standing on a main-track cell
int countAtTrack(int trackIdx)
{
    int cnt = 0;
    for (int p = 0; p < PLAYERS; p++)
        for (int pc = 0; pc < PIECES; pc++)
            if (pieceState[p][pc] == 1 && piecePos[p][pc] == trackIdx)
                cnt++;
    return cnt;
}

// Return the single player who owns piece(s) at trackIdx, or -1 if empty
int ownerAtTrack(int trackIdx)
{
    int owner = -1;
    for (int p = 0; p < PLAYERS; p++)
        for (int pc = 0; pc < PIECES; pc++)
            if (pieceState[p][pc] == 1 && piecePos[p][pc] == trackIdx) {
                if (owner == -1) owner = p;
            }
    return owner;
}

// =============================================================
// VALID MOVE CHECK
// =============================================================
bool canMove(int p, int pc, int dice)
{
    int st = pieceState[p][pc];
    if (st == 3) return false;  // already finished

    if (st == 0) {
        // In base: need a 6 to exit
        if (dice != 6) return false;
        // Cannot land on own piece at spawn
        int sp    = spawnIndex[p];
        int owner = ownerAtTrack(sp);
        if (owner == p) return false;
        return true;
    }

    if (st == 1) {
        int newMoves = pieceMoves[p][pc] + dice;
        // Cannot overshoot the finish cell (home-col index 5 → pieceMoves 57)
        if (newMoves > 51 + HOME_SIZE) return false;

        if (newMoves <= 51) {
            // Stays on main track — cannot land on own piece
            int newPos = (spawnIndex[p] + newMoves) % TOTAL_MAIN;
            int owner  = ownerAtTrack(newPos);
            if (owner == p) return false;
        }
        // (entering home column is always valid if not overshooting)
        return true;
    }

    if (st == 2) {
        // Already in home column — cannot overshoot position 5
        int curIdx = pieceMoves[p][pc] - 52;
        return (curIdx + dice <= 5);
    }

    return false;
}

// Build list of movable piece indices for current roll; returns count
int getValidMoves(int p, int dice, int validPCs[])
{
    int count = 0;
    for (int pc = 0; pc < PIECES; pc++)
        if (canMove(p, pc, dice))
            validPCs[count++] = pc;
    return count;
}

// =============================================================
// EXECUTE A MOVE
// =============================================================
void movePiece(int p, int pc, int dice)
{
    char msg[160];
    int  st = pieceState[p][pc];

    // ── Case 1: exit base ────────────────────────────────────
    if (st == 0) {
        int sp            = spawnIndex[p];
        pieceState[p][pc] = 1;
        piecePos[p][pc]   = sp;
        pieceMoves[p][pc] = 0;
        mainOccupant[sp]  = encode(p, pc);
        snprintf(msg, sizeof(msg), "%s piece %d exits base!", playerName(p), pc + 1);
        logMsg(msg);
        return;
    }

    // ── Case 2: on main track ────────────────────────────────
    if (st == 1) {
        int oldPos   = piecePos[p][pc];
        int newMoves = pieceMoves[p][pc] + dice;

        // Vacate old track cell (another piece of ours may still be there)
        if (mainOccupant[oldPos] == encode(p, pc))
            mainOccupant[oldPos] = -1;
        for (int q = 0; q < PLAYERS; q++)
            for (int qc = 0; qc < PIECES; qc++)
                if (!(q == p && qc == pc) &&
                    pieceState[q][qc] == 1 &&
                    piecePos[q][qc] == oldPos)
                    mainOccupant[oldPos] = encode(q, qc);

        if (newMoves <= 51) {
            // Stays on main track
            int newPos = (spawnIndex[p] + newMoves) % TOTAL_MAIN;

            // Capture any opponent pieces at newPos (not on safe spots)
            if (!isSafeSpot(newPos)) {
                for (int q = 0; q < PLAYERS; q++) {
                    if (q == p) continue;
                    for (int qc = 0; qc < PIECES; qc++) {
                        if (pieceState[q][qc] == 1 && piecePos[q][qc] == newPos) {
                            pieceState[q][qc] = 0;
                            piecePos[q][qc]   = 0;
                            pieceMoves[q][qc] = 0;
                            mainOccupant[newPos] = -1;
                            snprintf(msg, sizeof(msg),
                                "%s piece %d captures %s piece %d!",
                                playerName(p), pc + 1, playerName(q), qc + 1);
                            logMsg(msg);
                            bonusTurn = true;
                        }
                    }
                }
            }

            piecePos[p][pc]      = newPos;
            pieceMoves[p][pc]    = newMoves;
            mainOccupant[newPos] = encode(p, pc);
            snprintf(msg, sizeof(msg), "%s piece %d → track %d",
                playerName(p), pc + 1, newPos);
            logMsg(msg);
        } else {
            // Enters home column
            int homeIdx       = newMoves - 52;
            pieceMoves[p][pc] = newMoves;
            piecePos[p][pc]   = homeIdx;

            if (homeIdx == 5) {
                pieceState[p][pc] = 3;
                finishedCount[p]++;
                bonusTurn = true;
                snprintf(msg, sizeof(msg), "%s piece %d FINISHED!",
                    playerName(p), pc + 1);
            } else {
                pieceState[p][pc] = 2;
                snprintf(msg, sizeof(msg), "%s piece %d enters home col %d",
                    playerName(p), pc + 1, homeIdx);
            }
            logMsg(msg);
        }
        return;
    }

    // ── Case 3: already in home column ──────────────────────
    if (st == 2) {
        int newIdx        = (pieceMoves[p][pc] - 52) + dice;
        pieceMoves[p][pc] += dice;
        piecePos[p][pc]   = newIdx;

        if (newIdx == 5) {
            pieceState[p][pc] = 3;
            finishedCount[p]++;
            bonusTurn = true;
            snprintf(msg, sizeof(msg), "%s piece %d FINISHED!", playerName(p), pc + 1);
        } else {
            snprintf(msg, sizeof(msg), "%s piece %d at home col %d",
                playerName(p), pc + 1, newIdx);
        }
        logMsg(msg);
    }
}

// =============================================================
// WIN CHECK
// =============================================================
bool checkWin(int p) { return finishedCount[p] == PIECES; }

// =============================================================
// ADVANCE TO NEXT PLAYER
// =============================================================
void nextPlayer()
{
    bonusTurn        = false;
    consecutiveSixes = 0;
    currentPlayer    = (currentPlayer + 1) % PLAYERS;
    waitingForRoll   = true;
    waitingForMove   = false;
    highlightedCount = 0;
    diceAnimating    = false;
    aiThinking       = false;
    aiMoving         = false;

    // Skip players who have already won (all pieces finished)
    for (int t = 0; t < PLAYERS; t++) {
        bool allDone = true;
        for (int pc = 0; pc < PIECES; pc++)
            if (pieceState[currentPlayer][pc] != 3) { allDone = false; break; }
        if (!allDone) break;
        currentPlayer = (currentPlayer + 1) % PLAYERS;
    }

    char msg[80];
    snprintf(msg, sizeof(msg), "--- %s's turn ---", playerName(currentPlayer));
    logMsg(msg);
}

// =============================================================
// AFTER A DICE ROLL: apply three-sixes rule, compute valid moves
// =============================================================
void afterRoll(int dice)
{
    currentDice = dice;

    if (dice == 6) {
        consecutiveSixes++;
        if (consecutiveSixes == 1) saveState();   // snapshot before 1st six
        if (consecutiveSixes >= 3) {
            // Three sixes in a row — cancel all moves from those three turns
            restoreState();
            logMsg("Three 6s! All moves cancelled.");
            consecutiveSixes = 0;
            bonusTurn        = false;
            nextPlayer();
            return;
        }
    } else {
        consecutiveSixes = 0;
    }

    int validPCs[PIECES];
    int count = getValidMoves(currentPlayer, dice, validPCs);
    if (count == 0) {
        logMsg("No valid moves — turn skipped.");
        nextPlayer();
        return;
    }

    // Prepare the highlighted pieces list for the UI
    for (int i = 0; i < count; i++) highlightedPieces[i] = validPCs[i];
    highlightedCount = count;
    waitingForRoll   = false;
    waitingForMove   = true;
}

// =============================================================
// HANDLE PIECE SELECTION (human click or AI choice)
// =============================================================
void handlePieceSelected(int pc)
{
    if (!waitingForMove) return;

    // Verify the selected piece is indeed a valid move
    bool valid = false;
    for (int i = 0; i < highlightedCount; i++)
        if (highlightedPieces[i] == pc) { valid = true; break; }
    if (!valid) return;

    movePiece(currentPlayer, pc, currentDice);
    highlightedCount = 0;
    waitingForMove   = false;

    // Check win condition
    if (checkWin(currentPlayer)) {
        winnerID      = currentPlayer;
        currentScreen = 3;
        char msg[80];
        snprintf(msg, sizeof(msg), "*** %s WINS the game! ***", playerName(currentPlayer));
        logMsg(msg);
        return;
    }

    // Grant another turn on a 6 or after a capture / finishing a piece
    if (currentDice == 6 || bonusTurn) {
        waitingForRoll = true;
        bonusTurn      = false;
        char msg[80];
        snprintf(msg, sizeof(msg), "%s gets another turn!", playerName(currentPlayer));
        logMsg(msg);
    } else {
        nextPlayer();
    }
}

// =============================================================
// AI LOGIC
// =============================================================
bool isAI(int p)
{
    if (gameMode == 0) return false;   // PvP: no AI
    return (p != humanPlayerID);       // PvC: all non-human players are AI
}

void aiDoMove()
{
    int validPCs[PIECES];
    int count = getValidMoves(currentPlayer, currentDice, validPCs);
    if (count == 0) { nextPlayer(); return; }

    // Simple AI strategy:
    //   1. Prefer moves that finish a piece in home column
    //   2. Prefer moves that capture an opponent
    //   3. Prefer moving a piece out of base
    //   4. Otherwise pick randomly
    int best = validPCs[rand() % count];

    for (int i = 0; i < count; i++) {
        int pc = validPCs[i];
        // Priority 1: finishing a piece
        if (pieceState[currentPlayer][pc] == 2) {
            int newIdx = (pieceMoves[currentPlayer][pc] - 52) + currentDice;
            if (newIdx == 5) { best = pc; break; }
        }
        // Priority 2: capture
        if (pieceState[currentPlayer][pc] == 1) {
            int newMoves = pieceMoves[currentPlayer][pc] + currentDice;
            if (newMoves <= 51) {
                int newPos = (spawnIndex[currentPlayer] + newMoves) % TOTAL_MAIN;
                if (!isSafeSpot(newPos) && ownerAtTrack(newPos) >= 0 &&
                    ownerAtTrack(newPos) != currentPlayer) {
                    best = pc; break;
                }
            }
        }
        // Priority 3: exit base
        if (pieceState[currentPlayer][pc] == 0 && currentDice == 6) {
            best = pc;
        }
    }

    handlePieceSelected(best);
    aiMoving = false;
}

// =============================================================
// SAVE / LOAD
// =============================================================
void saveGame()
{
    FILE *f = fopen("ludo_save.dat", "w");
    if (!f) { logMsg("Save failed!"); return; }

    fprintf(f, "%d %d %d\n", currentScreen, gameMode, humanPlayerID);
    fprintf(f, "%d %d %d\n", currentPlayer, currentDice, consecutiveSixes);
    fprintf(f, "%d %d %d\n", (int)waitingForRoll, (int)waitingForMove, (int)bonusTurn);
    fprintf(f, "%d\n", winnerID);

    for (int p = 0; p < PLAYERS; p++) {
        for (int pc = 0; pc < PIECES; pc++)
            fprintf(f, "%d %d %d  ", pieceState[p][pc], piecePos[p][pc], pieceMoves[p][pc]);
        fprintf(f, "\n");
    }
    for (int p = 0; p < PLAYERS; p++) fprintf(f, "%d ", finishedCount[p]);
    fprintf(f, "\n");
    for (int i = 0; i < TOTAL_MAIN; i++) fprintf(f, "%d ", mainOccupant[i]);
    fprintf(f, "\n");
    fprintf(f, "%d\n", logCount);
    for (int i = 0; i < logCount; i++) fprintf(f, "%s\n", gameLog[i]);

    fclose(f);
    logMsg("Game saved.");
}

bool loadGame()
{
    FILE *f = fopen("ludo_save.dat", "r");
    if (!f) { logMsg("Load failed: no save file found."); return false; }

    fscanf(f, "%d %d %d", &currentScreen, &gameMode, &humanPlayerID);
    fscanf(f, "%d %d %d", &currentPlayer, &currentDice, &consecutiveSixes);
    int wr, wm, bt;
    fscanf(f, "%d %d %d", &wr, &wm, &bt);
    waitingForRoll = (bool)wr; waitingForMove = (bool)wm; bonusTurn = (bool)bt;
    fscanf(f, "%d", &winnerID);

    for (int p = 0; p < PLAYERS; p++)
        for (int pc = 0; pc < PIECES; pc++)
            fscanf(f, "%d %d %d", &pieceState[p][pc], &piecePos[p][pc], &pieceMoves[p][pc]);

    for (int p = 0; p < PLAYERS; p++) fscanf(f, "%d", &finishedCount[p]);
    for (int i = 0; i < TOTAL_MAIN; i++) fscanf(f, "%d", &mainOccupant[i]);

    fscanf(f, "%d\n", &logCount);
    for (int i = 0; i < logCount; i++) {
        fgets(gameLog[i], (int)sizeof(gameLog[i]), f);
        // strip trailing newline
        int len = (int)strlen(gameLog[i]);
        if (len > 0 && gameLog[i][len - 1] == '\n') gameLog[i][len - 1] = '\0';
    }
    fclose(f);

    highlightedCount = 0;
    diceAnimating    = false;
    aiThinking       = false;
    aiMoving         = false;
    currentScreen    = 2;   // always jump to game screen on load
    return true;
}

// =============================================================
// DRAWING HELPERS
// =============================================================

// Pixel centre of a 15×15 grid cell
sf::Vector2f cellCentre(int row, int col)
{
    return sf::Vector2f(
        GRID_OFFSET_X + col * CELL_SIZE + CELL_SIZE * 0.5f,
        GRID_OFFSET_Y + row * CELL_SIZE + CELL_SIZE * 0.5f);
}

sf::Vector2f trackCentre(int idx)   { return cellCentre(trackRow[idx], trackCol[idx]); }
sf::Vector2f homeCentre(int p, int i){ return cellCentre(homeRow[p][i], homeCol[p][i]); }
sf::Vector2f baseCentre(int p, int pc){ return cellCentre(baseRow[p][pc], baseCol[p][pc]); }

// Draw a filled cell rectangle
void drawCell(sf::RenderWindow &w, int row, int col,
              sf::Color fill,
              sf::Color outline = sf::Color(160, 160, 160))
{
    sf::RectangleShape rect(sf::Vector2f(CELL_SIZE - 1.f, CELL_SIZE - 1.f));
    rect.setPosition(GRID_OFFSET_X + col * CELL_SIZE + 0.5f,
                     GRID_OFFSET_Y + row * CELL_SIZE + 0.5f);
    rect.setFillColor(fill);
    rect.setOutlineThickness(1.f);
    rect.setOutlineColor(outline);
    w.draw(rect);
}

// Draw a single dice dot at relative position (dx,dy) within the dice square
void drawDot(sf::RenderWindow &w, float x, float y, float size, float dx, float dy)
{
    float r = size * 0.09f;
    sf::CircleShape c(r);
    c.setFillColor(sf::Color(30, 30, 30));
    c.setPosition(x + dx * size - r, y + dy * size - r);
    w.draw(c);
}

// Draw a graphical dice face (1-6) at screen position (x,y)
void drawDice(sf::RenderWindow &w, int face, float x, float y)
{
    const float S = 60.f;
    sf::RectangleShape box(sf::Vector2f(S, S));
    box.setPosition(x, y);
    box.setFillColor(sf::Color::White);
    box.setOutlineThickness(2.f);
    box.setOutlineColor(sf::Color(60, 60, 60));
    w.draw(box);

    switch (face) {
    case 1:
        drawDot(w, x, y, S, 0.50f, 0.50f);
        break;
    case 2:
        drawDot(w, x, y, S, 0.27f, 0.27f);
        drawDot(w, x, y, S, 0.73f, 0.73f);
        break;
    case 3:
        drawDot(w, x, y, S, 0.27f, 0.27f);
        drawDot(w, x, y, S, 0.50f, 0.50f);
        drawDot(w, x, y, S, 0.73f, 0.73f);
        break;
    case 4:
        drawDot(w, x, y, S, 0.27f, 0.27f); drawDot(w, x, y, S, 0.73f, 0.27f);
        drawDot(w, x, y, S, 0.27f, 0.73f); drawDot(w, x, y, S, 0.73f, 0.73f);
        break;
    case 5:
        drawDot(w, x, y, S, 0.27f, 0.27f); drawDot(w, x, y, S, 0.73f, 0.27f);
        drawDot(w, x, y, S, 0.50f, 0.50f);
        drawDot(w, x, y, S, 0.27f, 0.73f); drawDot(w, x, y, S, 0.73f, 0.73f);
        break;
    case 6:
        drawDot(w, x, y, S, 0.27f, 0.22f); drawDot(w, x, y, S, 0.73f, 0.22f);
        drawDot(w, x, y, S, 0.27f, 0.50f); drawDot(w, x, y, S, 0.73f, 0.50f);
        drawDot(w, x, y, S, 0.27f, 0.78f); drawDot(w, x, y, S, 0.73f, 0.78f);
        break;
    }
}

// =============================================================
// DRAW BOARD (15×15 grid)
// =============================================================
void drawBoard(sf::RenderWindow &w)
{
    // ── Step 1: Draw all cells with base colours ──────────────
    for (int r = 0; r < 15; r++) {
        for (int c = 0; c < 15; c++) {
            sf::Color col(248, 248, 248);  // default: near-white track

            // Outer yard rectangles (6×6 corners)
            if (r <= 5 && c <= 5)  col = playerLight[1];  // Green yard
            if (r <= 5 && c >= 9)  col = playerLight[2];  // Yellow yard
            if (r >= 9 && c <= 5)  col = playerLight[0];  // Red yard
            if (r >= 9 && c >= 9)  col = playerLight[3];  // Blue yard

            // Inner yard white areas (where pieces sit)
            if (r >= 1 && r <= 4 && c >= 1 && c <= 4)   col = sf::Color(255, 255, 255);
            if (r >= 1 && r <= 4 && c >= 10 && c <= 13) col = sf::Color(255, 255, 255);
            if (r >= 10 && r <= 13 && c >= 1 && c <= 4) col = sf::Color(255, 255, 255);
            if (r >= 10 && r <= 13 && c >= 10 && c <= 13) col = sf::Color(255, 255, 255);

            // Center 3×3 area — will be overdrawn with triangles
            if (r >= 6 && r <= 8 && c >= 6 && c <= 8)   col = sf::Color(230, 230, 230);

            drawCell(w, r, c, col);
        }
    }

    // ── Step 2: Home columns ─────────────────────────────────
    for (int p = 0; p < PLAYERS; p++) {
        for (int i = 0; i < HOME_SIZE - 1; i++)
            drawCell(w, homeRow[p][i], homeCol[p][i],
                     sf::Color(playerColor[p].r, playerColor[p].g, playerColor[p].b, 160));
        // Finish cell (index 5) — slightly darker
        drawCell(w, homeRow[p][5], homeCol[p][5], playerDark[p]);
    }

    // ── Step 3: Spawn cells ───────────────────────────────────
    for (int p = 0; p < PLAYERS; p++)
        drawCell(w, trackRow[spawnIndex[p]], trackCol[spawnIndex[p]], playerColor[p]);

    // ── Step 4: Star / safe-spot cells (non-spawn) ───────────
    int stars[4] = { 8, 21, 34, 47 };
    for (int i = 0; i < 4; i++)
        drawCell(w, trackRow[stars[i]], trackCol[stars[i]], sf::Color(255, 220, 80));

    // ── Step 5: Centre 3×3 triangles ─────────────────────────
    float cx = GRID_OFFSET_X + 6 * CELL_SIZE;
    float cy = GRID_OFFSET_Y + 6 * CELL_SIZE;
    float cw = 3 * CELL_SIZE;
    float ch = 3 * CELL_SIZE;
    float mx = cx + cw * 0.5f, my = cy + ch * 0.5f;

    // Bottom triangle → Red home column colour
    sf::ConvexShape tri;
    tri.setPointCount(3);
    tri.setFillColor(sf::Color(playerColor[0].r, playerColor[0].g, playerColor[0].b, 200));
    tri.setPoint(0, sf::Vector2f(cx,      cy + ch));
    tri.setPoint(1, sf::Vector2f(cx + cw, cy + ch));
    tri.setPoint(2, sf::Vector2f(mx,      my));
    w.draw(tri);

    // Left triangle → Green home column colour
    tri.setFillColor(sf::Color(playerColor[1].r, playerColor[1].g, playerColor[1].b, 200));
    tri.setPoint(0, sf::Vector2f(cx, cy));
    tri.setPoint(1, sf::Vector2f(cx, cy + ch));
    tri.setPoint(2, sf::Vector2f(mx, my));
    w.draw(tri);

    // Top triangle → Yellow home column colour
    tri.setFillColor(sf::Color(playerColor[2].r, playerColor[2].g, playerColor[2].b, 200));
    tri.setPoint(0, sf::Vector2f(cx,      cy));
    tri.setPoint(1, sf::Vector2f(cx + cw, cy));
    tri.setPoint(2, sf::Vector2f(mx,      my));
    w.draw(tri);

    // Right triangle → Blue home column colour
    tri.setFillColor(sf::Color(playerColor[3].r, playerColor[3].g, playerColor[3].b, 200));
    tri.setPoint(0, sf::Vector2f(cx + cw, cy));
    tri.setPoint(1, sf::Vector2f(cx + cw, cy + ch));
    tri.setPoint(2, sf::Vector2f(mx,      my));
    w.draw(tri);

    // Centre white circle labelled "HOME"
    sf::CircleShape homeCircle(CELL_SIZE * 0.9f);
    homeCircle.setFillColor(sf::Color(255, 255, 255, 220));
    homeCircle.setOrigin(CELL_SIZE * 0.9f, CELL_SIZE * 0.9f);
    homeCircle.setPosition(mx, my);
    w.draw(homeCircle);

    // ── Step 6: Star symbols on star cells ───────────────────
    // Draw a small ★ using a convex polygon (8-point star approximation)
    for (int i = 0; i < 4; i++) {
        sf::CircleShape star(CELL_SIZE * 0.22f, 5);
        star.setFillColor(sf::Color(200, 120, 0));
        star.setOrigin(CELL_SIZE * 0.22f, CELL_SIZE * 0.22f);
        sf::Vector2f sc = trackCentre(stars[i]);
        star.setPosition(sc.x, sc.y);
        w.draw(star);
    }
}

// =============================================================
// DRAW PIECES
// =============================================================
void drawPieces(sf::RenderWindow &w)
{
    float r = CELL_SIZE * 0.32f;

    for (int p = 0; p < PLAYERS; p++) {
        for (int pc = 0; pc < PIECES; pc++) {
            int st = pieceState[p][pc];
            if (st == 3) continue;   // finished — don't draw on board

            sf::Vector2f centre;

            if (st == 0) {
                centre = baseCentre(p, pc);
            } else if (st == 1) {
                centre = trackCentre(piecePos[p][pc]);
                // Offset stacked pieces so they are all visible
                int cnt   = countAtTrack(piecePos[p][pc]);
                int myIdx = 0;
                if (cnt > 1) {
                    // Count how many pieces come before this one at the same cell
                    bool found = false;
                    for (int q = 0; q < PLAYERS && !found; q++)
                        for (int qc = 0; qc < PIECES && !found; qc++) {
                            if (q == p && qc == pc) { found = true; break; }
                            if (pieceState[q][qc] == 1 && piecePos[q][qc] == piecePos[p][pc])
                                myIdx++;
                        }
                    // Four possible offsets (small 2×2 grid around centre)
                    float ox[4] = { -r * 0.55f,  r * 0.55f, -r * 0.55f,  r * 0.55f };
                    float oy[4] = { -r * 0.55f, -r * 0.55f,  r * 0.55f,  r * 0.55f };
                    if (myIdx < 4) { centre.x += ox[myIdx]; centre.y += oy[myIdx]; }
                }
            } else {  // st == 2
                centre = homeCentre(p, piecePos[p][pc]);
            }

            // Determine highlight state
            bool highlighted = false;
            if (waitingForMove && currentPlayer == p) {
                for (int i = 0; i < highlightedCount; i++)
                    if (highlightedPieces[i] == pc) { highlighted = true; break; }
            }

            // Shadow
            sf::CircleShape shadow(r);
            shadow.setFillColor(sf::Color(0, 0, 0, 55));
            shadow.setPosition(centre.x - r + 2.f, centre.y - r + 2.f);
            w.draw(shadow);

            // Piece body
            sf::CircleShape circle(r);
            circle.setFillColor(playerColor[p]);
            circle.setOutlineThickness(highlighted ? 3.f : 2.f);
            circle.setOutlineColor(highlighted
                ? sf::Color::White
                : playerDark[p]);
            circle.setPosition(centre.x - r, centre.y - r);
            w.draw(circle);

            // Pulsing ring around highlighted pieces
            if (highlighted) {
                sf::CircleShape ring(r + 5.f);
                ring.setFillColor(sf::Color::Transparent);
                ring.setOutlineThickness(2.f);
                ring.setOutlineColor(sf::Color(255, 255, 100, 200));
                ring.setPosition(centre.x - r - 5.f, centre.y - r - 5.f);
                w.draw(ring);
            }
        }
    }
}

// =============================================================
// BUTTON DRAWING HELPER
// =============================================================
void drawButton(sf::RenderWindow &w,
                float x, float y, float bw, float bh,
                const char *label,
                sf::Font &font,
                sf::Color bg    = sf::Color(70, 70, 200),
                unsigned int sz = 20)
{
    sf::RectangleShape btn(sf::Vector2f(bw, bh));
    btn.setPosition(x, y);
    btn.setFillColor(bg);
    btn.setOutlineThickness(2.f);
    btn.setOutlineColor(sf::Color::White);
    w.draw(btn);

    sf::Text txt;
    txt.setFont(font);
    txt.setString(label);
    txt.setCharacterSize(sz);
    txt.setFillColor(sf::Color::White);
    sf::FloatRect b = txt.getLocalBounds();
    txt.setPosition(x + (bw - b.width) * 0.5f - b.left,
                    y + (bh - b.height) * 0.5f - b.top);
    w.draw(txt);
}

// =============================================================
// SCREEN: MAIN MENU (screen 0)
// =============================================================
void drawMenu(sf::RenderWindow &w, sf::Font &font)
{
    w.clear(sf::Color(25, 25, 55));

    // Title
    sf::Text title;
    title.setFont(font);
    title.setString("L U D O");
    title.setCharacterSize(80);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color::White);
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition((WIN_WIDTH - tb.width) * 0.5f - tb.left, 70.f);
    w.draw(title);

    // Decorative coloured underline
    float lineY = 165.f;
    float lineW = 400.f, lineX = (WIN_WIDTH - lineW) * 0.5f;
    for (int i = 0; i < 4; i++) {
        sf::RectangleShape seg(sf::Vector2f(lineW * 0.25f - 4.f, 6.f));
        seg.setPosition(lineX + i * lineW * 0.25f + 2.f, lineY);
        seg.setFillColor(playerColor[i]);
        w.draw(seg);
    }

    float bw = 300.f, bh = 55.f, bx = (WIN_WIDTH - bw) * 0.5f;
    drawButton(w, bx, 230.f, bw, bh, "Player vs Player",    font, sf::Color(60, 140, 60));
    drawButton(w, bx, 310.f, bw, bh, "Player vs Computer",  font, sf::Color(60,  90, 180));
    drawButton(w, bx, 390.f, bw, bh, "Load Game",           font, sf::Color(90,  90,  90));
    drawButton(w, bx, 470.f, bw, bh, "Settings",            font, sf::Color(110, 70, 130));
}

// =============================================================
// SCREEN: COLOUR SELECT (screen 1)
// =============================================================
void drawColorSelect(sf::RenderWindow &w, sf::Font &font)
{
    w.clear(sf::Color(25, 25, 55));

    sf::Text title;
    title.setFont(font);
    title.setString("Choose Your Color");
    title.setCharacterSize(42);
    title.setFillColor(sf::Color::White);
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition((WIN_WIDTH - tb.width) * 0.5f - tb.left, 80.f);
    w.draw(title);

    const char *names[4] = { "Red", "Green", "Yellow", "Blue" };
    for (int i = 0; i < 4; i++) {
        float bx = 80.f + i * 185.f, by = 220.f;
        sf::RectangleShape box(sf::Vector2f(140.f, 140.f));
        box.setPosition(bx, by);
        box.setFillColor(playerColor[i]);
        box.setOutlineThickness(4.f);
        box.setOutlineColor(sf::Color::White);
        w.draw(box);

        sf::Text lbl;
        lbl.setFont(font);
        lbl.setString(names[i]);
        lbl.setCharacterSize(22);
        lbl.setFillColor(sf::Color::White);
        sf::FloatRect lb = lbl.getLocalBounds();
        lbl.setPosition(bx + (140.f - lb.width) * 0.5f - lb.left, by + 150.f);
        w.draw(lbl);
    }

    drawButton(w, (WIN_WIDTH - 200.f) * 0.5f, 430.f, 200.f, 50.f,
               "Back", font, sf::Color(110, 50, 50));
}

// =============================================================
// SCREEN: GAME (screen 2)
// =============================================================
// Y coordinate of the bottom control panel
static float panelY()  { return GRID_OFFSET_Y + 15.f * CELL_SIZE + 8.f; }
static float ctrlY()   { return panelY() + 50.f; }

void drawGameScreen(sf::RenderWindow &w, sf::Font &font)
{
    w.clear(sf::Color(200, 200, 200));

    drawBoard(w);
    drawPieces(w);

    float py = panelY(), cy = ctrlY();

    // ── Current-player colour bar ─────────────────────────────
    sf::RectangleShape bar(sf::Vector2f(WIN_WIDTH - 20.f, 42.f));
    bar.setPosition(10.f, py);
    bar.setFillColor(playerColor[currentPlayer]);
    bar.setOutlineThickness(2.f);
    bar.setOutlineColor(sf::Color(80, 80, 80));
    w.draw(bar);

    char turnStr[64];
    snprintf(turnStr, sizeof(turnStr), "%s's Turn%s",
             playerName(currentPlayer),
             isAI(currentPlayer) ? "  [AI]" : "");
    sf::Text turnTxt;
    turnTxt.setFont(font);
    turnTxt.setString(turnStr);
    turnTxt.setCharacterSize(22);
    turnTxt.setFillColor(sf::Color::White);
    turnTxt.setPosition(16.f, py + 10.f);
    w.draw(turnTxt);

    // Finished-pieces badge per player
    for (int p = 0; p < PLAYERS; p++) {
        if (finishedCount[p] > 0) {
            sf::RectangleShape badge(sf::Vector2f(70.f, 18.f));
            float bx = WIN_WIDTH - 80.f - (PLAYERS - 1 - p) * 76.f;
            badge.setPosition(bx, py + 12.f);
            badge.setFillColor(playerColor[p]);
            w.draw(badge);
            char fc[16];
            snprintf(fc, sizeof(fc), "%s %d/4", playerName(p), finishedCount[p]);
            sf::Text ft;
            ft.setFont(font);
            ft.setString(fc);
            ft.setCharacterSize(12);
            ft.setFillColor(sf::Color::White);
            ft.setPosition(bx + 3.f, py + 14.f);
            w.draw(ft);
        }
    }

    // ── Dice ─────────────────────────────────────────────────
    int face = diceAnimating ? diceAnimFace : currentDice;
    drawDice(w, face, 16.f, cy);

    // ── Roll button / waiting hint ────────────────────────────
    if (waitingForRoll && !isAI(currentPlayer) && !diceAnimating) {
        drawButton(w, 90.f, cy, 160.f, 60.f, "Roll Dice", font,
                   sf::Color(60, 140, 60));
    } else if (waitingForMove && !isAI(currentPlayer)) {
        sf::Text hint;
        hint.setFont(font);
        hint.setString("Click a piece to move");
        hint.setCharacterSize(17);
        hint.setFillColor(sf::Color(40, 40, 130));
        hint.setPosition(90.f, cy + 20.f);
        w.draw(hint);
    } else if (diceAnimating || (isAI(currentPlayer))) {
        sf::Text thinking;
        thinking.setFont(font);
        thinking.setString("Thinking...");
        thinking.setCharacterSize(17);
        thinking.setFillColor(sf::Color(100, 100, 100));
        thinking.setPosition(90.f, cy + 20.f);
        w.draw(thinking);
    }

    // ── Consecutive-sixes indicator ───────────────────────────
    if (consecutiveSixes > 0) {
        char sxt[24];
        snprintf(sxt, sizeof(sxt), "Sixes: %d / 3", consecutiveSixes);
        sf::Text sx;
        sx.setFont(font);
        sx.setString(sxt);
        sx.setCharacterSize(15);
        sx.setFillColor(sf::Color(200, 50, 50));
        sx.setPosition(16.f, cy + 64.f);
        w.draw(sx);
    }

    // ── Pause button ─────────────────────────────────────────
    drawButton(w, WIN_WIDTH - 115.f, cy, 105.f, 40.f,
               "Pause", font, sf::Color(90, 90, 90), 18);

    // ── Game log ─────────────────────────────────────────────
    float logX = 265.f, logW = WIN_WIDTH - logX - 125.f;
    sf::RectangleShape logBg(sf::Vector2f(logW, 110.f));
    logBg.setPosition(logX, cy);
    logBg.setFillColor(sf::Color(215, 215, 215));
    logBg.setOutlineThickness(1.f);
    logBg.setOutlineColor(sf::Color(160, 160, 160));
    w.draw(logBg);

    sf::Text logHdr;
    logHdr.setFont(font);
    logHdr.setString("Game Log:");
    logHdr.setCharacterSize(13);
    logHdr.setFillColor(sf::Color(70, 70, 70));
    logHdr.setStyle(sf::Text::Bold);
    logHdr.setPosition(logX + 4.f, cy + 2.f);
    w.draw(logHdr);

    int startIdx = logCount - 5;
    if (startIdx < 0) startIdx = 0;
    for (int i = startIdx; i < logCount; i++) {
        sf::Text lt;
        lt.setFont(font);
        lt.setString(gameLog[i]);
        lt.setCharacterSize(12);
        lt.setFillColor(sf::Color(40, 40, 40));
        lt.setPosition(logX + 4.f, cy + 18.f + (float)(i - startIdx) * 18.f);
        w.draw(lt);
    }
}

// =============================================================
// SCREEN: WIN (screen 3)
// =============================================================
void drawWinScreen(sf::RenderWindow &w, sf::Font &font)
{
    w.clear(sf::Color(15, 15, 35));

    sf::Text title;
    title.setFont(font);
    title.setString("WINNER!");
    title.setCharacterSize(80);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color::Yellow);
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition((WIN_WIDTH - tb.width) * 0.5f - tb.left, 130.f);
    w.draw(title);

    sf::Text nameT;
    nameT.setFont(font);
    nameT.setString(playerName(winnerID));
    nameT.setCharacterSize(60);
    nameT.setFillColor(winnerID >= 0 ? playerColor[winnerID] : sf::Color::White);
    sf::FloatRect nb = nameT.getLocalBounds();
    nameT.setPosition((WIN_WIDTH - nb.width) * 0.5f - nb.left, 240.f);
    w.draw(nameT);

    sf::Text sub;
    sub.setFont(font);
    sub.setString("wins the game!");
    sub.setCharacterSize(36);
    sub.setFillColor(sf::Color(210, 210, 210));
    sf::FloatRect sb = sub.getLocalBounds();
    sub.setPosition((WIN_WIDTH - sb.width) * 0.5f - sb.left, 320.f);
    w.draw(sub);

    drawButton(w, (WIN_WIDTH - 260.f) * 0.5f, 440.f, 260.f, 56.f,
               "Main Menu", font, sf::Color(70, 70, 200));
}

// =============================================================
// SCREEN: PAUSE OVERLAY (screen 4)
// =============================================================
void drawPauseScreen(sf::RenderWindow &w, sf::Font &font)
{
    // Dim overlay
    sf::RectangleShape dim(sf::Vector2f((float)WIN_WIDTH, (float)WIN_HEIGHT));
    dim.setFillColor(sf::Color(0, 0, 0, 155));
    w.draw(dim);

    sf::RectangleShape panel(sf::Vector2f(320.f, 290.f));
    float px = (WIN_WIDTH - 320.f) * 0.5f;
    float py2 = (WIN_HEIGHT - 290.f) * 0.5f;
    panel.setPosition(px, py2);
    panel.setFillColor(sf::Color(45, 45, 75));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color::White);
    w.draw(panel);

    sf::Text pt;
    pt.setFont(font);
    pt.setString("PAUSED");
    pt.setCharacterSize(38);
    pt.setStyle(sf::Text::Bold);
    pt.setFillColor(sf::Color::White);
    sf::FloatRect ptb = pt.getLocalBounds();
    pt.setPosition((WIN_WIDTH - ptb.width) * 0.5f - ptb.left, py2 + 18.f);
    w.draw(pt);

    float bx = (WIN_WIDTH - 210.f) * 0.5f, by = py2 + 80.f;
    drawButton(w, bx, by,        210.f, 48.f, "Resume",    font, sf::Color(60, 140, 60));
    drawButton(w, bx, by + 65.f, 210.f, 48.f, "Save Game", font, sf::Color(60,  90, 180));
    drawButton(w, bx, by + 130.f,210.f, 48.f, "Main Menu", font, sf::Color(150, 60, 60));
}

// =============================================================
// SCREEN: SETTINGS (screen 5)
// =============================================================
void drawSettingsScreen(sf::RenderWindow &w, sf::Font &font)
{
    w.clear(sf::Color(25, 25, 55));

    sf::Text title;
    title.setFont(font);
    title.setString("Settings");
    title.setCharacterSize(50);
    title.setFillColor(sf::Color::White);
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition((WIN_WIDTH - tb.width) * 0.5f - tb.left, 100.f);
    w.draw(title);

    char soundLabel[24];
    snprintf(soundLabel, sizeof(soundLabel), "Sound: %s", soundEnabled ? "ON" : "OFF");
    drawButton(w, (WIN_WIDTH - 220.f) * 0.5f, 240.f, 220.f, 56.f,
               soundLabel, font,
               soundEnabled ? sf::Color(60, 140, 60) : sf::Color(140, 60, 60));

    drawButton(w, (WIN_WIDTH - 220.f) * 0.5f, 380.f, 220.f, 56.f,
               "Back", font, sf::Color(110, 50, 50));
}

// =============================================================
// HIT-TEST HELPER
// =============================================================
bool inRect(float mx, float my, float rx, float ry, float rw, float rh)
{
    return mx >= rx && mx <= rx + rw && my >= ry && my <= ry + rh;
}

// =============================================================
// PIECE CLICK-DETECTION
// =============================================================
// Returns piece index (0-3) of the current player's piece at screen pos, or -1
int pieceAtPos(float mx, float my)
{
    float r = CELL_SIZE * 0.38f;
    for (int pc = 0; pc < PIECES; pc++) {
        int st = pieceState[currentPlayer][pc];
        if (st == 3) continue;

        sf::Vector2f c;
        if      (st == 0) c = baseCentre(currentPlayer, pc);
        else if (st == 1) c = trackCentre(piecePos[currentPlayer][pc]);
        else               c = homeCentre(currentPlayer, piecePos[currentPlayer][pc]);

        float dx = mx - c.x, dy = my - c.y;
        if (dx * dx + dy * dy <= r * r * 1.5f) return pc;
    }
    return -1;
}

// =============================================================
// EVENT HANDLERS PER SCREEN
// =============================================================
void handleMenuClick(float mx, float my)
{
    float bw = 300.f, bh = 55.f, bx = (WIN_WIDTH - bw) * 0.5f;

    if (inRect(mx, my, bx, 230.f, bw, bh)) {   // Player vs Player
        gameMode = 0;
        initializeGame();
        currentScreen = 2;
        logMsg("--- Red's turn ---");
    }
    if (inRect(mx, my, bx, 310.f, bw, bh)) {   // Player vs Computer
        gameMode = 1;
        currentScreen = 1;
    }
    if (inRect(mx, my, bx, 390.f, bw, bh)) {   // Load Game
        if (!loadGame()) logMsg("No save file found.");
    }
    if (inRect(mx, my, bx, 470.f, bw, bh)) {   // Settings
        currentScreen = 5;
    }
}

void handleColorSelectClick(float mx, float my)
{
    for (int i = 0; i < 4; i++) {
        float bx = 80.f + i * 185.f;
        if (inRect(mx, my, bx, 220.f, 140.f, 140.f)) {
            humanPlayerID = i;
            initializeGame();
            currentScreen = 2;
            logMsg("--- Red's turn ---");
            return;
        }
    }
    if (inRect(mx, my, (WIN_WIDTH - 200.f) * 0.5f, 430.f, 200.f, 50.f))
        currentScreen = 0;
}

void handleGameClick(float mx, float my)
{
    float cy = ctrlY();

    // Pause button
    if (inRect(mx, my, WIN_WIDTH - 115.f, cy, 105.f, 40.f)) {
        currentScreen = 4;
        return;
    }

    // Roll dice button
    if (waitingForRoll && !isAI(currentPlayer) && !diceAnimating) {
        if (inRect(mx, my, 90.f, cy, 160.f, 60.f)) {
            diceAnimating = true;
            diceAnimTime  = 0.f;
            diceFrameAcc  = 0.f;
            diceAnimClock.restart();
            return;
        }
    }

    // Piece selection
    if (waitingForMove && !isAI(currentPlayer)) {
        int pc = pieceAtPos(mx, my);
        if (pc >= 0) handlePieceSelected(pc);
    }
}

void handlePauseClick(float mx, float my)
{
    float bx = (WIN_WIDTH - 210.f) * 0.5f;
    float by = ((WIN_HEIGHT - 290.f) * 0.5f) + 80.f;

    if (inRect(mx, my, bx, by,         210.f, 48.f)) currentScreen = 2;
    if (inRect(mx, my, bx, by + 65.f,  210.f, 48.f)) saveGame();
    if (inRect(mx, my, bx, by + 130.f, 210.f, 48.f)) currentScreen = 0;
}

void handleSettingsClick(float mx, float my)
{
    if (inRect(mx, my, (WIN_WIDTH - 220.f) * 0.5f, 240.f, 220.f, 56.f))
        soundEnabled = !soundEnabled;
    if (inRect(mx, my, (WIN_WIDTH - 220.f) * 0.5f, 380.f, 220.f, 56.f))
        currentScreen = 0;
}

void handleWinClick(float mx, float my)
{
    if (inRect(mx, my, (WIN_WIDTH - 260.f) * 0.5f, 440.f, 260.f, 56.f))
        currentScreen = 0;
}

// =============================================================
// UPDATE — dice animation + AI turns
// =============================================================
void update(float dt)
{
    // ── Dice animation ────────────────────────────────────────
    if (diceAnimating) {
        diceAnimTime += dt;
        diceFrameAcc  += dt;
        if (diceFrameAcc >= ANIM_FRAME_SPEED) {
            diceFrameAcc -= ANIM_FRAME_SPEED;
            diceAnimFace = (diceAnimFace % 6) + 1;
        }
        if (diceAnimTime >= ANIM_DURATION) {
            // Settle on the final result
            diceAnimating = false;
            diceAnimTime  = 0.f;
            int rolled = rand() % 6 + 1;
            char msg[64];
            snprintf(msg, sizeof(msg), "%s rolled %d",
                     playerName(currentPlayer), rolled);
            logMsg(msg);
            afterRoll(rolled);
        }
        return;
    }

    if (currentScreen != 2) return;
    if (!isAI(currentPlayer))  return;

    // ── AI: roll dice ────────────────────────────────────────
    if (waitingForRoll && !aiThinking) {
        aiThinking = true;
        aiClock.restart();
    }
    if (aiThinking && waitingForRoll &&
        aiClock.getElapsedTime().asSeconds() >= AI_ROLL_DELAY)
    {
        aiThinking = false;
        // Trigger the same dice animation as a human would see
        diceAnimating = true;
        diceAnimTime  = 0.f;
        diceFrameAcc  = 0.f;
        diceAnimClock.restart();
    }

    // ── AI: move piece ────────────────────────────────────────
    if (waitingForMove && !aiMoving) {
        aiMoving = true;
        aiClock.restart();
    }
    if (aiMoving && waitingForMove &&
        aiClock.getElapsedTime().asSeconds() >= AI_MOVE_DELAY)
    {
        aiDoMove();
    }
}

// =============================================================
// MAIN
// =============================================================
int main()
{
    srand(static_cast<unsigned>(time(nullptr)));

    sf::RenderWindow window(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "Ludo Board Game",
                            sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);

    // ── Font loading (try common system paths; fail gracefully) ──
    sf::Font font;
    bool fontLoaded = false;
    const char *fontPaths[] = {
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        nullptr
    };
    for (int i = 0; fontPaths[i] != nullptr; i++) {
        if (font.loadFromFile(fontPaths[i])) { fontLoaded = true; break; }
    }
    if (!fontLoaded)
        cout << "Warning: could not load a system font. Text will not display.\n";

    initializeGame();

    sf::Clock frameClock;

    while (window.isOpen()) {
        float dt = frameClock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;  // cap delta-time to avoid spiral of doom

        // ── Event handling ────────────────────────────────────
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                float mx = static_cast<float>(event.mouseButton.x);
                float my = static_cast<float>(event.mouseButton.y);

                switch (currentScreen) {
                case 0: handleMenuClick(mx, my);         break;
                case 1: handleColorSelectClick(mx, my);  break;
                case 2: handleGameClick(mx, my);         break;
                case 3: handleWinClick(mx, my);          break;
                case 4: handlePauseClick(mx, my);        break;
                case 5: handleSettingsClick(mx, my);     break;
                }
            }
        }

        // ── Update ────────────────────────────────────────────
        if (currentScreen == 2 || diceAnimating)
            update(dt);

        // ── Draw ─────────────────────────────────────────────
        switch (currentScreen) {
        case 0: drawMenu(window, font);         break;
        case 1: drawColorSelect(window, font);  break;
        case 2: drawGameScreen(window, font);   break;
        case 3: drawWinScreen(window, font);    break;
        case 4:
            drawGameScreen(window, font);
            drawPauseScreen(window, font);
            break;
        case 5: drawSettingsScreen(window, font); break;
        }

        window.display();
    }

    return 0;
}
