#ifndef SNAKE_PRIVATE_H
#define SNAKE_PRIVATE_H

extern const GameInfo GameInfo_Snake;

namespace SnakeGame {

  // --- Siatka graficzna ---
  // Waz rysowany w komorkach 3x3 px (rysunek pikselami), bez czcionki.
  // Ekran 128x64 px -> 42 kolumny x 21 wierszy (kwadratowe komorki 3x3).
  #define CELL    3
  #define SNAKE_COLS  42   // 42 * 3 = 126 px
  #define SNAKE_ROWS  21   // 21 * 3 = 63 px

  // Macierz planszy: true = pole zajete przez weza (do kolizji z cialem)
  extern bool board[SNAKE_ROWS][SNAKE_COLS];

  // --- Parametry rozgrywki ---
  // Waz ma stala dlugosc (paliwo, nie wzrost). Mechanika "paliwa ruchu":
  // start FUEL_START kratek, kazdy krok -1, zjedzenie x daje +FUEL_PER_FOOD.
  // Koniec gry: paliwo == 0 lub kolizja (sciana / cialo).
  #define SNAKE_LEN       8     // stala dlugosc weza
  #define FUEL_START      50    // poczatkowe paliwo (kratki ruchu)
  #define FUEL_PER_FOOD   40    // ile paliwa daje zjedzone "x"

  // Plansza ograniczona sciana po obwodzie - wewnetrzne pola: 1..COLS-2 / 1..ROWS-2.
  #define WALL  1

  // Tempo: start STEP_MS_START, co 3 zjedzone "x" krok skraca sie o STEP_MS_DEC,
  // nie schodzac ponizej STEP_MS_MIN.
  #define STEP_MS_START   350
  #define STEP_MS_DEC     30
  #define STEP_MS_MIN     120

  // Kierunek jazdy (zgodnie z ruchem wskazowek zegara: gora->prawo->dol->lewo).
  enum Dir { DIR_UP = 0, DIR_RIGHT = 1, DIR_DOWN = 2, DIR_LEFT = 3 };

  struct Cell { int8_t col; int8_t row; };

  // Waz jako bufor pierscieniowy segmentow; head wskazuje glowe.
  struct SnakeState {
    Cell seg[SNAKE_LEN];
    int  head;                 // indeks glowy w seg[]
    Dir  dir;
    Cell food;                 // pozycja jedzenia
    int  fuel;                 // pozostale kratki ruchu
    int  eaten;                // zjedzone "x" (do przyspieszania)
    unsigned long stepMs;      // aktualny odstep miedzy krokami
    unsigned long lastStep;
  };

  void Game_Snake();
  void DrawWelcomeScreen();
  void ClearBoard();
  void DrawWall();
  void DrawSnakeCell(uint8_t row, uint8_t col, bool head, bool vertical);
  void DrawFood(const Cell& f);
  void DrawHud(SnakeState& s);

  void SnakeInit(SnakeState& s);
  void PlaceFood(SnakeState& s);
  void DrawSnake(SnakeState& s);
  void HandleTurn(SnakeState& s);
  bool StepSnake(SnakeState& s);   // false = koniec gry (kolizja lub brak paliwa)
  void DisplayStarved();
  void DisplayGameOver(SnakeState& s);
}

#endif
