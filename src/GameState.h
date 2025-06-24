#pragma once

/// full sequence: UI → MAP → L1… → L2… → L3… → EXIT
enum class GameState {
    INTERFACE,   // your splash or main menu
    MAP,         // world map
    // ─── Level 1 ─────────────────────────────────
    PUZZLE,      // PuzzleGame::run()
    RSA,         // RSADecryptor::run()
    TETRIS,      // Tetris::run()
    // ─── Level 2 ─────────────────────────────────
    CIRCUIT,     // CircuitGame::run()
    PROJECTION,  // ProjectionGame::run()
    // ─── Level 3 ─────────────────────────────────
    SPACESHOOTER,// SpaceShooter::run()
    MONSTER,     // MonsterGame::run()
    // ─── end ───────────────────────────────────────
    EXIT
};
