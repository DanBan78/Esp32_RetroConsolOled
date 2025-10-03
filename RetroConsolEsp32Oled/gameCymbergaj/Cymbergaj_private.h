#ifndef CYMBERGAJ_PRIVATE_H
#define CYMBERGAJ_PRIVATE_H

namespace CymbergajGame {

    #define HalfFieldY 31 
    #define HalfFieldX 63
    #define FieldXmax 126
    #define FieldYmax 62
    #define DELTA_X 4
    #define CYM_MENU_ROW_MAX 3
    #define CYM_MENU_OPTION_IN_ROW_MAX 4

    enum direction {toRight=0, toRightDown, toRightUp, toLeft, toLeftDown, toLeftUp};
    enum collision {noCollision=0, collisionLeft, collisionRight};
    enum speed {Wolno=0, Srednio, Szybko};
    enum level {Lekko=0, Trudniej, Master};
    enum aiLevel {Amator=1, Normalny, Geniusz};

    typedef struct cymBallStruct {
      int x;
      int y;
      int prevX;
      int prevY;
      unsigned long prevUpdateTime = 0;
      unsigned long updateRate;
      unsigned long updateRateInit;
      direction isHeading;
      int dY;
    };

    typedef struct cymPlayerStruct {
      int y;          //srodek paletki
      int score = 0;
    };

    typedef struct gameStruct {
      bool updateScreen;
      collision isCollision;
      int playersNo;
      int aiLevel;
      unsigned long aiPadlePosPrevUpdateTime = 0;
      unsigned long aiPadlePosUpdateRate = 150;
      unsigned long playerPadlePosPrevUpdateTime = 0;
      unsigned long playerPadlePosUpdateRate = 50;
      int maxPoints = 5;
      int padleDeltaMoveY;
      int padleHalfWidth;
      level difficulty;
      speed ballSpeed;
    };

    typedef struct  cymMenuStruct {
        const int initX = 7;
        const int initY = 18;
        const int deltaY = 11;
        const int rowMax = CYM_MENU_ROW_MAX;
        const int optionsInRow =  CYM_MENU_OPTION_IN_ROW_MAX;
        const String optionString[CYM_MENU_ROW_MAX][CYM_MENU_OPTION_IN_ROW_MAX] =  {
            { "Tryb:     ", "1 gracz", "2 graczy", "demo", }, 
            { "Trudnosc: ", "lekko", "trudniej", "master" }, 
            { "AI:       ", "amator", "normalny", "geniusz" }
        };
        const int menuOptionValue[CYM_MENU_ROW_MAX][3] =  {
            { 1, 2, 0 }, 
            { 0, 1, 2 }, 
            { 1, 2, 3 } 
        };
        int rowOptionSelect[CYM_MENU_ROW_MAX] = { 1, 2,  2};
    };
 
  bool ItsGameOver(gameStruct& Game, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  bool ItsTimeForBallPosUpdate(cymBallStruct& Ball);
  bool ItsTimeForPadlePosUpdate(gameStruct& Game);
  int  UpdateBallDyAfterPadleHit(int BallX, int BallY, int LeftPadleY, int RightPadleY);
  void UpdateBallPos(gameStruct& Game, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void UpdateBallXpos(cymBallStruct& Ball);
  void UpdateBallYpos(cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void CheckIfBallHitBand(cymBallStruct& Ball);
  void CheckIfLeftPlayerLostGoal(gameStruct& Game, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void CheckIfRightPlayerLostGoal(gameStruct& Game, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void InitParams(gameStruct& Game, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void ParamsUpdateFromUserMenu(cymMenuStruct& Menu, gameStruct& Game);
  void DisplayGameSummary(gameStruct& Game, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void DisplayGoal(gameStruct& Game, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void DisplayScore(gameStruct& Game, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void DrawBall(cymBallStruct& Ball);
  void DrawField();
  void DrawGameScreen(gameStruct& Game, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void DrawPadle(gameStruct& Game, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void Game_Cymbergaj();
  void HandleLostGoal(gameStruct& Game, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void PadlePosAiUpdate(gameStruct& Game, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void PadlePosPlayerUpdate(gameStruct& Game, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void ShowUserMenu(cymMenuStruct& Menu, gameStruct& Game);
  void UpdateBallPosAndCheckBandOrGoal(gameStruct& CymGame, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer);
  void DisplayMenuStrings(cymMenuStruct& Menu, int SelectedRow);
}

#endif
