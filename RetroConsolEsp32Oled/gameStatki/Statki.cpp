#include "Statki.h"
#include "Statki_private.h"

namespace StatkiGame {

  // ---- Rozstawienie floty -------------------------------------------------

  bool IsShip(const Board& b, int8_t r, int8_t c) {
    return b.shipAt[r][c] >= 0;
  }

  // Probuje wstawic statek shipIdx (dlugosci len) w losowym miejscu/orientacji.
  // Statki nie moga sie stykac (margines 1 pola). Zwraca true gdy sie udalo.
  bool PlaceShip(Board& b, uint8_t shipIdx, uint8_t len) {
    for (int tries = 0; tries < 200; tries++) {
      bool horiz = random(0, 2);
      int r, c;
      if (horiz) { r = random(0, GRID);           c = random(0, GRID - len + 1); }
      else       { r = random(0, GRID - len + 1); c = random(0, GRID); }

      // sprawdz pola statku + otoczke (sasiedztwo) - nie moga stykac sie z innym
      bool ok = true;
      for (int i = -1; i <= len && ok; i++) {
        for (int j = -1; j <= 1 && ok; j++) {
          int rr = r + (horiz ? j : i);
          int cc = c + (horiz ? i : j);
          if (rr < 0 || rr >= GRID || cc < 0 || cc >= GRID) continue;
          if (b.shipAt[rr][cc] >= 0) ok = false;
        }
      }
      if (!ok) continue;

      Ship& sh = b.ship[shipIdx];
      sh.len = len;
      sh.sunk = false;
      for (int i = 0; i < len; i++) {
        int rr = r + (horiz ? 0 : i);
        int cc = c + (horiz ? i : 0);
        sh.r[i] = rr;
        sh.c[i] = cc;
        b.shipAt[rr][cc] = shipIdx;
      }
      return true;
    }
    return false;
  }

  void RandomFleet(Board& b) {
    for (int r = 0; r < GRID; r++)
      for (int c = 0; c < GRID; c++) {
        b.shot[r][c] = false;
        b.shipAt[r][c] = -1;
      }
    b.hits = 0;
    for (int i = 0; i < FLEET_N; i++) {
      if (!PlaceShip(b, i, FLEET[i])) i--;   // ponow gdy nie zmiescil sie
    }
  }

  // ---- Strzal -------------------------------------------------------------

  // Oznacza pole otaczajace zatopiony statek jako "strzelano" (puste pudlo).
  // Statki sie nie stykaja, wiec te pola na pewno sa woda.
  static void MarkSurround(Board& b, const Ship& sh) {
    for (int i = 0; i < sh.len; i++) {
      for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
          int rr = sh.r[i] + dr;
          int cc = sh.c[i] + dc;
          if (rr < 0 || rr >= GRID || cc < 0 || cc >= GRID) continue;
          if (b.shipAt[rr][cc] < 0) b.shot[rr][cc] = true; // tylko woda
        }
      }
    }
  }

  // Strzal w (r,c): zaznacza strzal, zwraca MISS / HIT / SUNK. Przy zatopieniu
  // oznacza statek jako sunk i otacza go kropkami (pola dookola = woda).
  ShotResult FireAt(Board& b, int8_t r, int8_t c) {
    b.shot[r][c] = true;
    int8_t idx = b.shipAt[r][c];
    if (idx < 0) return SHOT_MISS;

    b.hits++;
    Ship& sh = b.ship[idx];
    // czy wszystkie pola tego statku sa juz ostrzelane?
    for (int i = 0; i < sh.len; i++) {
      if (!b.shot[sh.r[i]][sh.c[i]]) return SHOT_HIT;
    }
    sh.sunk = true;
    MarkSurround(b, sh);
    return SHOT_SUNK;
  }

  // ---- Rysowanie ----------------------------------------------------------

  // Symbole w polu 8x8 (x,y = lewy gorny rog pola).
  static void DrawMissDot(int x, int y) {            // pudlo: kropka w srodku
    myOLED.drawPixel(x + BCELL / 2, y + BCELL / 2, SH110X_WHITE);
  }
  static void DrawHitO(int x, int y) {               // trafiony: 'o'
    myOLED.drawCircle(x + BCELL / 2, y + BCELL / 2, 2, SH110X_WHITE);
  }
  static void DrawSunkX(int x, int y) {              // zatopiony: 'x'
    myOLED.drawLine(x + 2, y + 2, x + BCELL - 2, y + BCELL - 2, SH110X_WHITE);
    myOLED.drawLine(x + BCELL - 2, y + 2, x + 2, y + BCELL - 2, SH110X_WHITE);
  }

  // Plansza strzalow w CPU: pudlo=kropka, trafiony='o', zatopiony='x'.
  // Statki CPU sa ukryte - widac tylko skutki strzalow. Kursor = ramka.
  void DrawBoardShots(const Board& b, int8_t curR, int8_t curC) {
    myOLED.drawRect(GRID_X, GRID_Y, GRID * BCELL, GRID * BCELL, SH110X_WHITE);
    for (int r = 0; r < GRID; r++) {
      for (int c = 0; c < GRID; c++) {
        if (!b.shot[r][c]) continue;
        int x = GRID_X + c * BCELL;
        int y = GRID_Y + r * BCELL;
        int8_t idx = b.shipAt[r][c];
        if (idx < 0)              DrawMissDot(x, y);
        else if (b.ship[idx].sunk) DrawSunkX(x, y);
        else                       DrawHitO(x, y);
      }
    }
    int cx = GRID_X + curC * BCELL;
    int cy = GRID_Y + curR * BCELL;
    myOLED.drawRect(cx, cy, BCELL, BCELL, SH110X_WHITE);
  }

  // Podglad wlasnej floty: statki jako ramki pol; trafiony='o', zatopiony='x',
  // pudlo CPU = kropka.
  void DrawOwnFleet(const Board& b) {
    myOLED.drawRect(GRID_X, GRID_Y, GRID * BCELL, GRID * BCELL, SH110X_WHITE);
    for (int r = 0; r < GRID; r++) {
      for (int c = 0; c < GRID; c++) {
        int x = GRID_X + c * BCELL;
        int y = GRID_Y + r * BCELL;
        int8_t idx = b.shipAt[r][c];
        if (idx >= 0) myOLED.drawRect(x + 1, y + 1, BCELL - 2, BCELL - 2, SH110X_WHITE);
        if (b.shot[r][c]) {
          if (idx < 0)                DrawMissDot(x, y);
          else if (b.ship[idx].sunk)  DrawSunkX(x, y);
          else                        DrawHitO(x, y);
        }
      }
    }
  }

  // Rysuje jeden kwadracik 4x4 w legendzie ze statusem pola statku.
  static void DrawShipCell(int x, int y, bool shot, bool sunk) {
    myOLED.drawRect(x, y, 4, 4, SH110X_WHITE);  // ramka = kwadracik
    if (sunk) {
      // zatopiony: x
      myOLED.drawLine(x, y, x + 3, y + 3, SH110X_WHITE);
      myOLED.drawLine(x + 3, y, x, y + 3, SH110X_WHITE);
    } else if (shot) {
      // trafiony: o (dwa piksele w srodku)
      myOLED.drawPixel(x + 1, y + 1, SH110X_WHITE);
      myOLED.drawPixel(x + 2, y + 2, SH110X_WHITE);
      myOLED.drawPixel(x + 2, y + 1, SH110X_WHITE);
      myOLED.drawPixel(x + 1, y + 2, SH110X_WHITE);
    }
  }

  // Rysuje legende floty dla planszy b.
  // Kazdy statek jako rzad kwadracikow 4x4 (1px przerwa miedzy nimi).
  // Statki o tej samej dlugosci (2x2 i 2x1) sa na jednej linii obok siebie.
  // px = x startowy panelu, py = y startowy.
  void DrawFleetLegend(const Board& b, int px, int py) {
    // FLEET = {4,3,2,2,1,1} - rysujemy w kolejnosci indeksow ship[]
    // Statki 0=4masz, 1=3masz, 2=2masz, 3=2masz, 4=1masz, 5=1masz
    const int CELL_SZ = 4;
    const int GAP = 1;      // przerwa miedzy kwadracikami jednego statku
    const int SHIP_GAP = 4; // przerwa miedzy dwoma statkami na jednej linii

    for (int i = 0; i < FLEET_N; ) {
      // sprawdz czy dwa kolejne statki sa tej samej dlugosci -> na jednej linii
      bool pair = (i + 1 < FLEET_N && FLEET[i] == FLEET[i + 1]);
      int lx = px;

      for (int rep = 0; rep < (pair ? 2 : 1); rep++) {
        int si = i + rep;
        const Ship& sh = b.ship[si];
        for (int m = 0; m < sh.len; m++) {
          // czy to pole statku bylo ostrzelane?
          bool shot = b.shot[sh.r[m]][sh.c[m]];
          DrawShipCell(lx, py, shot, sh.sunk);
          lx += CELL_SZ + GAP;
        }
        if (pair && rep == 0) lx += SHIP_GAP;
      }

      py += CELL_SZ + GAP + 1;   // nastepna linia
      i += pair ? 2 : 1;
    }
  }

  // Panel po prawej: legenda floty CPU + licznik strzalow i strat gracza.
  void DrawSidePanel(StatkiState& s) {
    int px = GRID * BCELL + 4;   // x = 68
    myOLED.setTextSize(1);
    myOLED.setTextColor(SH110X_WHITE);
    myOLED.setCursor(px, 0);
    myOLED.print("CPU:");
    DrawFleetLegend(s.cpu, px, 8);
    // separator
    myOLED.drawLine(px, 42, 127, 42, SH110X_WHITE);
    // strzaly gracza
    myOLED.setCursor(px, 46);
    myOLED.print("Strz:");
    myOLED.print(s.shots);
    // stracone statki gracza
    myOLED.setCursor(px, 56);
    myOLED.print("Str:");
    myOLED.print(s.playerSunk);
    myOLED.print("/6");
  }

  // ---- Tura gracza --------------------------------------------------------

  // Rysuje aktualny ekran (plansza strzalow lub podglad floty) + panel.
  static void Render(StatkiState& s) {
    myOLED.clearDisplay();
    if (s.showFleet) DrawOwnFleet(s.player);
    else             DrawBoardShots(s.cpu, s.curR, s.curC);
    DrawSidePanel(s);
    myOLED.display();
  }

  // Obsluga jednej tury gracza. Sterowanie:
  //  UpLeft  = kursor w prawo (kolumna++ cyklicznie),
  //  DownLeft= kursor w dol  (wiersz++ cyklicznie),
  //  UpRight = podglad wlasnej floty (przelacznik),
  //  DownRight = strzal w pole pod kursorem.
  // Zwraca true gdy gracz zatopil cala flote CPU. Liczy kazdy oddany strzal
  // w shots. Trafienie pozwala strzelac dalej, pudlo konczy ture.
  bool PlayerTurn(StatkiState& s) {
    while (true) {
      Render(s);

      if (IsPressed(UpLeft)) {
        s.curR = (s.curR - 1 + GRID) % GRID;   // gora
        while (IsPressed(UpLeft));
      } else if (IsPressed(DownLeft)) {
        s.curR = (s.curR + 1) % GRID;           // dol
        while (IsPressed(DownLeft));
      } else if (IsPressed(UpRight)) {
        s.curC = (s.curC + 1) % GRID;           // prawo
        while (IsPressed(UpRight));
      } else if (IsPressed(DownRight)) {
        while (IsPressed(DownRight));
        if (s.cpu.shot[s.curR][s.curC]) continue;           // juz strzelane - ignoruj
        s.shots++;
        ShotResult res = FireAt(s.cpu, s.curR, s.curC);
        if (SoundEnabled) MyTune(res == SHOT_MISS ? TON_RAMKA_FREQ : TON_PUNKT_FREQ, 60);
        if (res != SHOT_MISS) {
          if (s.cpu.hits >= FLEET_CELLS) return true;       // wygrana
          continue;                                         // trafienie/zatopienie = gra dalej
        }
        return false;                                       // pudlo = koniec tury
      }
      delay(20);
    }
  }

  // ---- AI CPU -------------------------------------------------------------

  // Wybiera pole do strzalu: w trybie polowania dobija obok trafienia,
  // inaczej strzela losowo w nieostrzelane pole.
  void AiShoot(StatkiState& s, int8_t& outR, int8_t& outC) {
    Board& b = s.player;

    if (s.ai.hunting) {
      // sprawdz 4 sasiadow ostatniego trafienia, wybierz pierwsze nieostrzelane
      const int8_t dr[4] = { -1, 1, 0, 0 };
      const int8_t dc[4] = { 0, 0, -1, 1 };
      int order[4] = { 0, 1, 2, 3 };
      for (int i = 3; i > 0; i--) { int j = random(0, i + 1); int t = order[i]; order[i] = order[j]; order[j] = t; }
      for (int k = 0; k < 4; k++) {
        int8_t r = s.ai.lastHitR + dr[order[k]];
        int8_t c = s.ai.lastHitC + dc[order[k]];
        if (r < 0 || r >= GRID || c < 0 || c >= GRID) continue;
        if (b.shot[r][c]) continue;
        outR = r; outC = c; return;
      }
    }
    // losowy strzal w nieostrzelane pole
    do {
      outR = random(0, GRID);
      outC = random(0, GRID);
    } while (b.shot[outR][outC]);
  }

  // Tura CPU: strzela az spudluje. Zwraca true gdy CPU zatopilo flote gracza.
  bool CpuTurn(StatkiState& s) {
    Board& b = s.player;
    while (true) {
      int8_t r, c;
      AiShoot(s, r, c);
      ShotResult res = FireAt(b, r, c);

      // pokaz strzal CPU na podgladzie floty
      s.showFleet = true;
      Render(s);
      if (SoundEnabled) MyTune(res == SHOT_MISS ? TON_RAMKA_FREQ : TON_ERROR_FREQ, 60);
      delay(DELAY500MS);

      if (res == SHOT_SUNK) {
        s.playerSunk++;
        if (b.hits >= FLEET_CELLS) return true;   // CPU wygralo
        s.ai.hunting = false;                     // statek zatopiony - koniec polowania
        continue;                                 // trafienie = strzela dalej
      } else if (res == SHOT_HIT) {
        // wejdz/kontynuuj polowanie wokol trafienia
        if (!s.ai.hunting) { s.ai.hunting = true; s.ai.firstHitR = r; s.ai.firstHitC = c; }
        s.ai.lastHitR = r; s.ai.lastHitC = c;
        continue;                                 // trafienie = strzela dalej
      } else {
        // pudlo: jesli polowal, wroc do pierwszego trafienia jako bazy
        if (s.ai.hunting) { s.ai.lastHitR = s.ai.firstHitR; s.ai.lastHitC = s.ai.firstHitC; }
        return false;                             // koniec tury CPU
      }
    }
  }

  // ---- Ekran konca --------------------------------------------------------

  void DisplayResult(bool playerWon, StatkiState& s) {
    int highscore;
    EEPROM.get(Game_StatkiRecord, highscore);
    if (highscore <= 0 || highscore > 9999) highscore = 0;
    // rekord = najmniejsza liczba strzalow do wygranej (mniej = lepiej)
    bool newRecord = playerWon && (highscore == 0 || s.shots < highscore);
    if (newRecord) { EEPROM.put(Game_StatkiRecord, s.shots); EEPROM.commit(); highscore = s.shots; }

    myOLED.clearDisplay();
    myOLED.setTextColor(SH110X_WHITE);
    myOLED.setTextSize(2);
    myOLED.setCursor(5, 2);
    myOLED.print(playerWon ? "WYGRANA!" : "PORAZKA");
    myOLED.setTextSize(1);
    myOLED.setCursor(0, 22);
    myOLED.print("Twoje strzaly: ");
    myOLED.print(s.shots);
    myOLED.setCursor(0, 32);
    myOLED.print("Stracone: ");
    myOLED.print(s.playerSunk);
    myOLED.print("/6");
    if (playerWon) {
      myOLED.setCursor(0, 42);
      if (newRecord) {
        myOLED.print("Nowy rekord!");
      } else {
        myOLED.print("Rekord: ");
        myOLED.print(highscore);
        myOLED.print(" strz.");
      }
    }
    myOLED.display();
    delay(DELAY1000MS);
    WaitForAnyButtonToContinue();
  }

  // ---- Petla gry ----------------------------------------------------------

  void Game_Statki() {
    while (true) {
      StatkiState s;
      RandomFleet(s.player);
      RandomFleet(s.cpu);
      s.curR = 0; s.curC = 0;
      s.showFleet = false;
      s.ai.hunting = false;
      s.shots = 0;
      s.playerSunk = 0;

      while (true) {
        // tura gracza: strzela az spudluje lub wygra (s.shots liczone w srodku)
        if (PlayerTurn(s)) { DisplayResult(true, s); break; }
        // tura CPU: strzela az spudluje lub wygra
        if (CpuTurn(s))    { DisplayResult(false, s); break; }
        s.showFleet = false;
      }
    }
  }

} // namespace StatkiGame

const GameInfo GameInfo_Statki = {
  "Statki",
  "Zatop flote CPU",
  StatkiGame::Game_Statki,
  Game_StatkiRecord
};
