#include "Snake.h"
#include "Snake_private.h"

namespace SnakeGame {

  bool board[SNAKE_ROWS][SNAKE_COLS];

  // ----------------------------------------------------------------------
  // Rysuje pojedyncza komorke siatki (4x4 px) jako bloki 2x2 px.
  // head == true -> glowa: blok 2x2 w lewym gornym rogu:
  //     XX..
  //     XX..
  //     ....
  //     ....
  // head == false -> cialo: gruba przekatna, naprzemiennie '\' i '/'
  // zaleznie od parzystosci (row+col), co daje zygzak wzdluz weza:
  //   '\' : XX..      '/' : ..XX
  //         XX..            ..XX
  //         ..XX            XX..
  //         ..XX            XX..
  // ----------------------------------------------------------------------
  void DrawCell(uint8_t row, uint8_t col, bool head) {
    int16_t x = col * CELL;
    int16_t y = row * CELL;
    if (head) {
      myOLED.fillRect(x, y, 2, 2, SH110X_WHITE);
      return;
    }
    if (((row + col) & 1) == 0) {
      // segment '\' - bloki na przekatnej z lewego-gornego do prawego-dolnego
      myOLED.fillRect(x,     y,     2, 2, SH110X_WHITE);
      myOLED.fillRect(x + 2, y + 2, 2, 2, SH110X_WHITE);
    } else {
      // segment '/' - bloki na przekatnej z prawego-gornego do lewego-dolnego
      myOLED.fillRect(x + 2, y,     2, 2, SH110X_WHITE);
      myOLED.fillRect(x,     y + 2, 2, 2, SH110X_WHITE);
    }
  }

  void ClearBoard() {
    for (uint8_t r = 0; r < SNAKE_ROWS; r++) {
      for (uint8_t c = 0; c < SNAKE_COLS; c++) {
        board[r][c] = false;
      }
    }
  }

  // Test: wypelnij cala plansze (sprawdzenie rozmiaru siatki na ekranie)
  void FillBoardTest() {
    for (uint8_t r = 0; r < SNAKE_ROWS; r++) {
      for (uint8_t c = 0; c < SNAKE_COLS; c++) {
        board[r][c] = true;
      }
    }
  }

  // Rysuje plansze na podstawie macierzy bool - kazde true to kwadrat 4x4
  void DrawBoard() {
    myOLED.clearDisplay();
    for (uint8_t r = 0; r < SNAKE_ROWS; r++) {
      for (uint8_t c = 0; c < SNAKE_COLS; c++) {
        if (board[r][c]) {
          DrawCell(r, c, false);
        }
      }
    }
    myOLED.display();
  }

  void DrawWelcomeScreen() {
    myOLED.clearDisplay();
    myOLED.setTextColor(SH110X_WHITE);
    myOLED.setTextSize(2);
    myOLED.setCursor(20, 1);
    myOLED.println("Snake");
    myOLED.setTextSize(1);
    myOLED.setCursor(0, 30);
    myOLED.println("dowolny przycisk");
    myOLED.setCursor(0, 42);
    myOLED.println("by zaczac");
    myOLED.display();
    WaitForAnyButtonToContinue();
  }

  // ----------------------------------------------------------------------
  // Glowny przebieg gry (na razie etap szkieletu):
  //  1. ekran powitalny,
  //  2. test - cala plansza wypelniona kwadratami,
  //  3. waz wysrodkowany (tulow + glowa),
  //  4. powrot do menu po przycisku.
  // ----------------------------------------------------------------------
  void Game_Snake() {
    Score = 0;
    DrawWelcomeScreen();

    // 2. Test wypelnienia ekranu
    FillBoardTest();
    DrawBoard();
    WaitForAnyButtonToContinue();

    // 3. Waz na srodku: tulow (2 segmenty) + glowa
    ClearBoard();
    uint8_t midRow = SNAKE_ROWS / 2;          // wiersz 8
    uint8_t midCol = SNAKE_COLS / 2;          // kolumna 16
    board[midRow][midCol - 2] = true;         // ogon
    board[midRow][midCol - 1] = true;         // tulow
    board[midRow][midCol]     = true;         // glowa

    // Rysujemy tulow z macierzy, a glowe nadpisujemy blokiem 2x2
    DrawBoard();
    myOLED.fillRect(midCol * CELL, midRow * CELL, CELL, CELL, SH110X_BLACK); // wyczysc komorke glowy
    DrawCell(midRow, midCol, true);
    myOLED.display();

    WaitForAnyButtonToContinue();
  }

} // namespace SnakeGame

const GameInfo GameInfo_Snake = {
  "Snake",
  "klasyczny waz",
  SnakeGame::Game_Snake,
  Game_SnakeRecord
};
