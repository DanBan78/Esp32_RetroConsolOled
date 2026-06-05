#include "Snake.h"
#include "Snake_private.h"

namespace SnakeGame {

  bool board[SNAKE_ROWS][SNAKE_COLS];

  // ----------------------------------------------------------------------
  // Rysuje komorke weza (3x3 px). Wzor zalezy od orientacji segmentu, dzieki
  // czemu waz "animuje" sie zarowno w poziomie jak i w pionie.
  // head    -> pelny kwadrat 3x3 z czarnym okiem w srodku (XXX/X.X/XXX).
  // poziomy (vertical=false) -> kreska pozioma 2px, naprzemiennie u gory/dolu:
  //   parzysta: XX. / .. / ..      nieparzysta: .. / .. / .XX
  // pionowy (vertical=true) -> kreska pionowa 2px, naprzemiennie z lewej/prawej:
  //   parzysta: X.. / X.. / ..     nieparzysta: ..X / ..X / ..
  // ----------------------------------------------------------------------
  void DrawSnakeCell(uint8_t row, uint8_t col, bool head, bool vertical) {
    int16_t x = col * CELL;
    int16_t y = row * CELL;
    if (head) {
      myOLED.fillRect(x, y, CELL, CELL, SH110X_WHITE);
      myOLED.drawPixel(x + 1, y + 1, SH110X_BLACK); // oko
      return;
    }
    bool even = (((row + col) & 1) == 0);
    if (vertical) {
      int px = even ? x : x + 2;        // lewa lub prawa kolumna komorki
      myOLED.drawPixel(px, y,     SH110X_WHITE);
      myOLED.drawPixel(px, y + 1, SH110X_WHITE);
    } else {
      int py = even ? y : y + 2;        // gorny lub dolny rzad komorki
      myOLED.drawPixel(x,     py, SH110X_WHITE);
      myOLED.drawPixel(x + 1, py, SH110X_WHITE);
    }
  }

  // Jedzenie: znak "x" z kropek w kratce 3x3 (rogi + srodek): X.X / .X. / X.X
  void DrawFood(const Cell& f) {
    int16_t x = f.col * CELL;
    int16_t y = f.row * CELL;
    myOLED.drawPixel(x,     y,     SH110X_WHITE);
    myOLED.drawPixel(x + 2, y,     SH110X_WHITE);
    myOLED.drawPixel(x + 1, y + 1, SH110X_WHITE);
    myOLED.drawPixel(x,     y + 2, SH110X_WHITE);
    myOLED.drawPixel(x + 2, y + 2, SH110X_WHITE);
  }

  void ClearBoard() {
    for (uint8_t r = 0; r < SNAKE_ROWS; r++) {
      for (uint8_t c = 0; c < SNAKE_COLS; c++) {
        board[r][c] = false;
      }
    }
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

  // Indeks segmentu w buforze pierscieniowym, liczony wstecz od glowy
  // (0 = glowa, SNAKE_LEN-1 = ogon).
  static inline int SegIndex(SnakeState& s, int back) {
    return (s.head - back + SNAKE_LEN) % SNAKE_LEN;
  }

  // Ustawia weza poziomo na srodku, glowa po prawej, jadacy w prawo.
  void SnakeInit(SnakeState& s) {
    ClearBoard();
    s.head = 0;
    s.dir = DIR_RIGHT;
    s.fuel = FUEL_START;
    s.eaten = 0;
    s.stepMs = STEP_MS_START;
    s.lastStep = millis();

    int row = SNAKE_ROWS / 2;
    int headCol = SNAKE_COLS / 2;
    for (int i = 0; i < SNAKE_LEN; i++) {
      int idx = (s.head - i + SNAKE_LEN) % SNAKE_LEN;
      s.seg[idx].col = headCol - i;   // ogon po lewej, glowa po prawej
      s.seg[idx].row = row;
      board[row][headCol - i] = true;
    }
    PlaceFood(s);
  }

  // Losuje jedzenie na wolnym polu wewnatrz scian (niezajetym przez weza).
  void PlaceFood(SnakeState& s) {
    do {
      s.food.col = random(WALL, SNAKE_COLS - WALL);
      s.food.row = random(WALL, SNAKE_ROWS - WALL);
    } while (board[s.food.row][s.food.col]);
  }

  // Ramka sciany po obwodzie planszy (kolizyjna granica).
  void DrawWall() {
    myOLED.drawRect(0, 0, SNAKE_COLS * CELL, SNAKE_ROWS * CELL, SH110X_WHITE);
  }

  // Pasek paliwa wzdluz gornej krawedzi: dlugosc proporcjonalna do fuel.
  // Skala: pelna szerokosc ekranu odpowiada 2*FUEL_START (z zapasem na +20).
  void DrawHud(SnakeState& s) {
    int maxW = SNAKE_COLS * CELL - 2;          // szerokosc wewnatrz ramki
    int w = (s.fuel * maxW) / (2 * FUEL_START);
    if (w > maxW) w = maxW;
    if (w < 0) w = 0;
    myOLED.fillRect(1, 1, w, 1, SH110X_WHITE);  // pasek tuz pod gorna ramka
  }

  void DrawSnake(SnakeState& s) {
    myOLED.clearDisplay();
    DrawWall();
    DrawHud(s);
    DrawFood(s.food);
    for (int i = 0; i < SNAKE_LEN; i++) {
      Cell& c = s.seg[SegIndex(s, i)];
      bool vertical;
      if (i == 0) {
        // glowa: orientacja wg kierunku jazdy
        vertical = (s.dir == DIR_UP || s.dir == DIR_DOWN);
      } else {
        // segment: orientacja wg roznicy z sasiadem blizszym glowy
        Cell& prev = s.seg[SegIndex(s, i - 1)];
        vertical = (c.col == prev.col);
      }
      DrawSnakeCell(c.row, c.col, i == 0, vertical);
    }
    myOLED.display();
  }

  // Sterowanie bezwzgledne zalezne od osi jazdy (zawsze skret prostopadly):
  //  - waz jedzie poziomo: UpLeft = w gore, DownLeft = w dol,
  //  - waz jedzie pionowo: UpLeft = w lewo, UpRight = w prawo.
  void HandleTurn(SnakeState& s) {
    bool horizontal = (s.dir == DIR_LEFT || s.dir == DIR_RIGHT);
    if (horizontal) {
      if (IsPressed(UpLeft)) {
        s.dir = DIR_UP;
        while (IsPressed(UpLeft));
      } else if (IsPressed(DownLeft)) {
        s.dir = DIR_DOWN;
        while (IsPressed(DownLeft));
      }
    } else {
      if (IsPressed(UpLeft)) {
        s.dir = DIR_LEFT;
        while (IsPressed(UpLeft));
      } else if (IsPressed(UpRight)) {
        s.dir = DIR_RIGHT;
        while (IsPressed(UpRight));
      }
    }
  }

  // Wykonuje jeden krok weza. Zwraca false = koniec gry (sciana, cialo, paliwo).
  bool StepSnake(SnakeState& s) {
    Cell head = s.seg[s.head];
    int nc = head.col, nr = head.row;
    switch (s.dir) {
      case DIR_UP:    nr--; break;
      case DIR_DOWN:  nr++; break;
      case DIR_LEFT:  nc--; break;
      case DIR_RIGHT: nc++; break;
    }

    // kolizja ze sciana (brzeg planszy o szerokosci WALL)
    if (nc < WALL || nc >= SNAKE_COLS - WALL || nr < WALL || nr >= SNAKE_ROWS - WALL) {
      return false;
    }

    // Waz ma stala dlugosc - ogon zawsze zwalnia swoje pole, wiec nie liczymy
    // go jako kolizji (glowa moze wejsc tam, gdzie ogon wlasnie znika).
    Cell tail = s.seg[SegIndex(s, SNAKE_LEN - 1)];
    if (board[nr][nc] && !(nr == tail.row && nc == tail.col)) return false;

    // koszt ruchu: 1 kratka paliwa
    s.fuel--;
    Score++;                                  // wynik = liczba wykonanych krokow
    if (s.fuel <= 0) return false;            // brak paliwa = koniec gry

    bool eating = (nr == s.food.row && nc == s.food.col);
    if (eating) {
      s.fuel += FUEL_PER_FOOD;
      s.eaten++;
      // co 3 zjedzone przyspiesz weza
      if (s.eaten % 3 == 0 && s.stepMs > STEP_MS_MIN) {
        s.stepMs -= STEP_MS_DEC;
        if (s.stepMs < STEP_MS_MIN) s.stepMs = STEP_MS_MIN;
      }
    }

    // przesun weza: zwolnij ogon, dodaj nowa glowe (stala dlugosc)
    board[tail.row][tail.col] = false;
    s.head = (s.head + 1) % SNAKE_LEN;
    s.seg[s.head].col = nc;
    s.seg[s.head].row = nr;
    board[nr][nc] = true;

    if (eating) PlaceFood(s);
    return true;
  }

  // Komunikat o smierci z glodu (paliwo spadlo do zera).
  void DisplayStarved() {
    myOLED.clearDisplay();
    myOLED.setTextColor(SH110X_WHITE);
    myOLED.setTextSize(1);
    myOLED.setCursor(10, 22);
    myOLED.println("Koniec jedzenia");
    myOLED.setCursor(30, 36);
    myOLED.println("umierasz");
    myOLED.display();
    delay(DELAY1500MS);
  }

  void DisplayGameOver(SnakeState& s) {
    int highscore;
    EEPROM.get(Game_SnakeRecord, highscore);
    if (highscore < 0 || highscore > 9999) highscore = 0; // EEPROM niezainicjalizowane
    if (Score > highscore) {
      EEPROM.put(Game_SnakeRecord, Score);
      EEPROM.commit();
      highscore = Score;
    }

    myOLED.clearDisplay();
    myOLED.setTextColor(SH110X_WHITE);
    myOLED.setTextSize(2);
    myOLED.setCursor(10, 5);
    myOLED.println("Game Over");
    myOLED.setTextSize(1);
    myOLED.setCursor(5, 30);
    myOLED.print("Score: ");
    myOLED.print(Score);
    myOLED.setCursor(5, 42);
    myOLED.print("Best:  ");
    myOLED.print(highscore);
    myOLED.display();
    delay(DELAY1000MS);
    WaitForAnyButtonToContinue();
  }

  void Game_Snake() {
    DrawWelcomeScreen();

    while (true) {
      Score = 0;
      SnakeState snake;
      SnakeInit(snake);
      DrawSnake(snake);

      bool alive = true;
      while (alive) {
        HandleTurn(snake);                 // reaguj na przyciski na biezaco
        if (TimerElapsed(snake.lastStep, snake.stepMs)) {
          alive = StepSnake(snake);
          if (alive) DrawSnake(snake);
        }
        delay(10);                         // odciaz petle / debounce
      }

      if (snake.fuel <= 0) DisplayStarved();  // smierc z glodu (paliwo == 0)
      DisplayGameOver(snake);
      // petla while(true) restartuje gre po nacisnieciu przycisku,
      // jak w pozostalych grach; wszystkie 4 przyciski = restart konsoli.
    }
  }

} // namespace SnakeGame

const GameInfo GameInfo_Snake = {
  "Snake",
  "klasyczny waz",
  SnakeGame::Game_Snake,
  Game_SnakeRecord
};
