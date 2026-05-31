#ifndef SNAKE_PRIVATE_H
#define SNAKE_PRIVATE_H

extern const GameInfo GameInfo_Snake;

namespace SnakeGame {

  // --- Siatka graficzna ---
  // Waz rysowany kwadratami 4x4 px (fillRect), bez czcionki.
  // Ekran 128x64 px -> 32 kolumny x 16 wierszy (kwadratowe komorki).
  #define CELL    4
  #define SNAKE_COLS  32   // 32 * 4 = 128 px
  #define SNAKE_ROWS  16   // 16 * 4 = 64 px

  // Macierz planszy: true = pole zajete przez weza
  extern bool board[SNAKE_ROWS][SNAKE_COLS];

  void Game_Snake();
  void DrawWelcomeScreen();
  void ClearBoard();
  void FillBoardTest();
  void DrawBoard();
  void DrawCell(uint8_t row, uint8_t col, bool head);
}

#endif
