#ifndef STATKI_PRIVATE_H
#define STATKI_PRIVATE_H

extern const GameInfo GameInfo_Statki;

namespace StatkiGame {

  // --- Plansza ---
  #define GRID    8          // 8x8 pol
  #define BCELL   8          // 8 px na pole -> plansza 64x64 px
  #define GRID_X  0          // lewy gorny rog planszy na ekranie
  #define GRID_Y  0

  // Flota: dlugosci statkow (suma pol = 4+3+2+2+1+1 = 13).
  #define FLEET_N 6
  const uint8_t FLEET[FLEET_N] = { 4, 3, 2, 2, 1, 1 };
  #define FLEET_CELLS 13     // laczna liczba pol zajetych przez statki

  #define SHIP_MAX_LEN 4       // najdluzszy statek

  // Pojedynczy statek: lista swoich pol (wspolrzedne) i dlugosc.
  struct Ship {
    int8_t r[SHIP_MAX_LEN];
    int8_t c[SHIP_MAX_LEN];
    uint8_t len;
    bool sunk;                 // czy zatopiony
  };

  // Plansza:
  //  shot[r][c]   - czy w pole strzelano (0/1),
  //  shipAt[r][c] - indeks statku na polu (-1 = woda), do szybkiego trafienia,
  //  ship[]       - krotka lista statkow (pola), wg ktorej liczymy zatopienia.
  struct Board {
    bool   shot[GRID][GRID];
    int8_t shipAt[GRID][GRID];
    Ship   ship[FLEET_N];
    uint8_t hits;              // trafione pola (do wykrycia konca gry)
  };

  // Stan AI: tryb dobijania po trafieniu (cel + kierunki do sprawdzenia).
  struct AiState {
    bool hunting;              // true = dobija trafiony statek
    int8_t firstHitR, firstHitC;
    int8_t lastHitR, lastHitC;
  };

  struct StatkiState {
    Board player;              // flota gracza (CPU w nia strzela)
    Board cpu;                 // flota CPU (gracz w nia strzela)
    int8_t curR, curC;         // pozycja kursora na planszy CPU
    bool showFleet;            // czy podgladamy wlasna flote
    AiState ai;
    uint8_t playerSunk;        // ile statkow gracza CPU zatopilo
    int shots;                 // liczba strzalow gracza (do panelu)
  };

  // Wynik strzalu w pole.
  enum ShotResult { SHOT_MISS, SHOT_HIT, SHOT_SUNK };

  void Game_Statki();
  void RandomFleet(Board& b);
  bool PlaceShip(Board& b, uint8_t shipIdx, uint8_t len);
  ShotResult FireAt(Board& b, int8_t r, int8_t c);   // oznacza strzal, zwraca wynik
  bool IsShip(const Board& b, int8_t r, int8_t c);    // czy pole nalezy do statku
  void DrawBoardShots(const Board& b, int8_t curR, int8_t curC);
  void DrawOwnFleet(const Board& b);
  void DrawFleetLegend(const Board& b, int px, int py);
  void DrawSidePanel(StatkiState& s);
  bool PlayerTurn(StatkiState& s);   // true gdy gracz wygral
  bool CpuTurn(StatkiState& s);      // true gdy CPU wygralo
  void AiShoot(StatkiState& s, int8_t& r, int8_t& c);
  void DisplayResult(bool playerWon, StatkiState& s);
}

#endif
