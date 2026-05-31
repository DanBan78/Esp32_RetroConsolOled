#include "Cymbergaj.h"
#include "Cymbergaj_private.h"

namespace CymbergajGame {

  void Game_Cymbergaj(){
    gameStruct   CymGame;
    cymMenuStruct   Menu;
    cymBallStruct   Ball;
    cymPlayerStruct LeftPlayer;
    cymPlayerStruct RightPlayer;

    while(true) {
      ShowUserMenu(Menu, CymGame);
      InitParams(CymGame, Ball, LeftPlayer, RightPlayer);

      while(true) {
        if (CymGame.updateScreen) DrawGameScreen(CymGame, Ball, LeftPlayer, RightPlayer);
        if (ItsTimeForBallPosUpdate(Ball)) UpdateBallPos(CymGame, Ball, LeftPlayer, RightPlayer);
        if (ItsTimeForPadlePosUpdate(CymGame)) {
          PadlePosPlayerUpdate(CymGame, LeftPlayer, RightPlayer);
          PadlePosAiUpdate(CymGame, Ball, LeftPlayer, RightPlayer);
        }
        if (ItsGameOver(CymGame, LeftPlayer, RightPlayer)) {
          DisplayGameSummary(CymGame, LeftPlayer, RightPlayer);
          break;
        }
      }
    }
  }

  void InitParams(gameStruct& CymGame, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer){
    Ball.x         = HalfFieldX;
    Ball.y         = HalfFieldY;
    Ball.isHeading = toRight;
    Ball.dY        = 0;

    LeftPlayer.y   = HalfFieldY;
    RightPlayer.y  = HalfFieldY;
    LeftPlayer.score  = 0;
    RightPlayer.score = 0;
    CymGame.padleDeltaMoveY = 3;
    CymGame.updateScreen    = true;

    switch (CymGame.difficulty) {
      case Lekko:
        Ball.updateRateInit = 30;
        CymGame.padleDeltaMoveY = 3;
        break;
      case Trudniej:
        Ball.updateRateInit = 22;
        CymGame.padleDeltaMoveY = 2;
        break;
      case Master:
        Ball.updateRateInit = 18;
        CymGame.padleDeltaMoveY = 2;
        break;
    }
    Ball.updateRate = Ball.updateRateInit;
  }

  void PadlePosAiUpdate(gameStruct& CymGame, cymBallStruct& Ball,  cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    if (!TimerElapsed(CymGame.aiPadlePosPrevUpdateTime, CymGame.aiPadlePosUpdateRate)) return;

    uint8_t deltaYai = 3*CymGame.aiLevel;
    uint8_t LossFactor = random(0,6/CymGame.aiLevel);
    if (CymGame.playersNo != 2) {
      if (RightPlayer.y > Ball.y) RightPlayer.y = RightPlayer.y - deltaYai - LossFactor;
      if (RightPlayer.y < Ball.y) RightPlayer.y = RightPlayer.y + deltaYai + LossFactor;
    }
    if (CymGame.playersNo == 0) {
      if (LeftPlayer.y > Ball.y) LeftPlayer.y = LeftPlayer.y - deltaYai - LossFactor;
      if (LeftPlayer.y < Ball.y) LeftPlayer.y = LeftPlayer.y + deltaYai + LossFactor;
    }
    if (CymGame.playersNo > 0) CymGame.updateScreen = true;
  }

  void PadlePosPlayerUpdate(gameStruct& CymGame, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    int PadleMinY = 0;
    int PadleMaxY = FieldYmax + 4;

    switch (CymGame.playersNo) {
      case 2:
        if (IsPressed(UpRight)) {
          RightPlayer.y = RightPlayer.y-CymGame.padleDeltaMoveY;
          if (RightPlayer.y < PadleMinY) RightPlayer.y = PadleMinY;
        }
        if (IsPressed(DownRight)) {
          RightPlayer.y = RightPlayer.y + CymGame.padleDeltaMoveY;
          if (RightPlayer.y > PadleMaxY) RightPlayer.y = PadleMaxY;
        }
        // fall-through: przy 2 graczach obsluz tez lewa paletke ponizej
      case 1:
       if (IsPressed(UpLeft)) {
          LeftPlayer.y = LeftPlayer.y-CymGame.padleDeltaMoveY;
          if (LeftPlayer.y < PadleMinY) LeftPlayer.y = PadleMinY;
        }
        if (IsPressed(DownLeft)) {
          LeftPlayer.y = LeftPlayer.y+CymGame.padleDeltaMoveY;
          if (LeftPlayer.y > PadleMaxY ) LeftPlayer.y = PadleMaxY;
        }
        if (!IsPressed(NONE)) CymGame.updateScreen = true;
        // fall-through: case 0 i tak tylko wychodzi (AI sterowane gdzie indziej)
      case 0:
      default:
        return;
    }
  }

  void UpdateBallPosAndCheckBandOrGoal(gameStruct& CymGame, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    UpdateBallXpos(Ball);
    UpdateBallYpos(Ball, LeftPlayer, RightPlayer);
    CheckIfBallHitBand(Ball);
    if (Ball.x < 3) {
      CheckIfLeftPlayerLostGoal(CymGame, Ball, LeftPlayer, RightPlayer);
      if (Ball.isHeading == toLeft ) Ball.isHeading = (direction)random(1,3);
      if (Ball.isHeading == toLeftUp ) Ball.isHeading = toRightUp;
      if (Ball.isHeading == toLeftDown ) Ball.isHeading = toRightDown;
      Ball.x = 5; 
    }
    if (Ball.x >= FieldXmax-3) {
      CheckIfRightPlayerLostGoal(CymGame, Ball, LeftPlayer, RightPlayer);
      if (Ball.isHeading == toRight ) Ball.isHeading = (direction)random(4,6);
      if (Ball.isHeading == toRightUp ) Ball.isHeading = toLeftUp;
      if (Ball.isHeading == toRightDown ) Ball.isHeading = toLeftDown;
      Ball.x = FieldXmax-6;
    }
  }

  int UpdateBallDyAfterPadleHit(int BallX, int BallY, int LeftPadleY, int RightPadleY) {
    int padleHitDist;
    if (BallX < HalfFieldX) {
      padleHitDist = abs(BallY - LeftPadleY);
    } else {
      padleHitDist = abs(BallY - RightPadleY);
    }
    if (padleHitDist < 3) {return 1;
    } else if (padleHitDist < 7) {return 2;
    } else if  (padleHitDist < 10) {return 3;
    } else return 4;
  }

  void CheckIfLeftPlayerLostGoal(gameStruct& CymGame, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    if (Ball.y < LeftPlayer.y - CymGame.padleHalfWidth-2 || Ball.y > LeftPlayer.y + CymGame.padleHalfWidth+2) {
      RightPlayer.score++;
      HandleLostGoal(CymGame, Ball, LeftPlayer, RightPlayer);
      CymGame.updateScreen = true;

    } else {
      if (SoundEnabled) MyTune(TON_RAMKA_FREQ,30);
      Ball.dY = UpdateBallDyAfterPadleHit(Ball.x, Ball.y, LeftPlayer.y, RightPlayer.y);
    }
  }

  void CheckIfRightPlayerLostGoal(gameStruct& CymGame, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    if (Ball.y < RightPlayer.y - CymGame.padleHalfWidth-2 || Ball.y > RightPlayer.y + CymGame.padleHalfWidth+2) {
      LeftPlayer.score++;
      HandleLostGoal(CymGame, Ball, LeftPlayer, RightPlayer);
      CymGame.updateScreen = true;

    } else {
      if (SoundEnabled) MyTune(TON_ODLICZANIE_FREQ,30);
      Ball.dY = UpdateBallDyAfterPadleHit(Ball.x, Ball.y, LeftPlayer.y, RightPlayer.y);
    }
  }

  void HandleLostGoal(gameStruct& CymGame, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    DrawBall(Ball);
    Ball.prevX = Ball.x;
    if (Ball.x < HalfFieldX) {
      Ball.x = 5;
    } else {
      Ball.x = FieldXmax - 5;
    }
    DisplayGoal(CymGame, Ball, LeftPlayer, RightPlayer);

    Ball.y = HalfFieldY;
    Ball.dY = UpdateBallDyAfterPadleHit(Ball.x, Ball.y, LeftPlayer.y, RightPlayer.y);
    DrawBall(Ball);
    Ball.updateRate = Ball.updateRateInit;
  }

  bool ItsGameOver(gameStruct& CymGame, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    if (CymGame.playersNo == 2) {
      if (LeftPlayer.score >= CymGame.maxPoints || RightPlayer.score >= CymGame.maxPoints) {
        return true;
      }
    } else if (CymGame.playersNo == 1) {
      if (RightPlayer.score >= CymGame.maxPoints) {
        return true;
      }
    } else if (CymGame.playersNo == 0) {
      if (IsPressed(UpLeft) || 
          IsPressed(DownRight) ||
          IsPressed(UpRight) ||
          IsPressed(DownLeft)) {
        return true;
      }
    }
    return false;
  } 

  void DrawField() {
    myOLED.drawLine (4, 0, FieldXmax-5, 0, SH110X_WHITE);
    myOLED.drawLine (4, FieldYmax, FieldXmax-5, FieldYmax, SH110X_WHITE);
    myOLED.drawLine (HalfFieldX, 0, HalfFieldX, FieldYmax-1, SH110X_WHITE);
    myOLED.drawCircle (HalfFieldX, HalfFieldY, 7, SH110X_WHITE );
  }

  void UpdateBallPos(gameStruct& CymGame, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    Ball.prevX = Ball.x;
    Ball.prevY = Ball.y;
    UpdateBallPosAndCheckBandOrGoal(CymGame, Ball, LeftPlayer, RightPlayer);
    if (CymGame.ballSpeed == Szybko) {
      UpdateBallPosAndCheckBandOrGoal(CymGame, Ball, LeftPlayer, RightPlayer);
    }
    CymGame.updateScreen = true;
  }

  void DrawBall(cymBallStruct& Ball) {
    myOLED.fillCircle (Ball.prevX,Ball.prevY, 2, SH110X_BLACK);
    myOLED.fillCircle (Ball.x,Ball.y, 2, SH110X_WHITE);
  }

  void DrawPadle(gameStruct& CymGame, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    myOLED.fillRect (0,0, 3, 64, SH110X_BLACK);
    myOLED.drawRect (1,LeftPlayer.y-CymGame.padleHalfWidth, 2, 2*CymGame.padleHalfWidth-3, SH110X_WHITE);

    myOLED.fillRect (FieldXmax-3,1, 2, FieldYmax-2, SH110X_BLACK);
    myOLED.drawRect (FieldXmax-3,RightPlayer.y - CymGame.padleHalfWidth, 2, 2*CymGame.padleHalfWidth-3, SH110X_WHITE);
  }

  void ShowUserMenu(cymMenuStruct& Menu, gameStruct& CymGame) {
    btPressedCode btn;
    int SelectedRow = 0;
    int SelectedRowOption = Menu.rowOptionSelect[SelectedRow];
    while (true) {
      myOLED.fillRect(0, Menu.initY, FieldXmax, FieldYmax-Menu.initY, SH110X_BLACK);
      DisplayMenuStrings(Menu, SelectedRow);

      switch(ReadButton(nullptr, Timer1Sec)) {
        case UpLeft:
          SelectedRow--;
          if (SelectedRow < 0) SelectedRow = 0;
          SelectedRowOption = Menu.rowOptionSelect[SelectedRow];
        break;
        case DownLeft:
          SelectedRow++;
          if (SelectedRow >= Menu.rowMax) SelectedRow = Menu.rowMax - 1;
          SelectedRowOption = Menu.rowOptionSelect[SelectedRow];
        break;
        case UpRight:
          SelectedRowOption++;
          if (SelectedRowOption >= Menu.optionsInRow) {
            SelectedRowOption = 1;
          }
          Menu.rowOptionSelect[SelectedRow] = SelectedRowOption;
        break;
        case DownRight:
          UserMenuParamsUpdate(Menu, CymGame);
          WaitForButtonRelease();
        return;
      }
      WaitForButtonRelease();
      myOLED.display();
    }
  }

  void DisplayMenuStrings(cymMenuStruct& Menu, int SelectedRow) {
    myOLED.clearDisplay();
    myOLED.setCursor(1, 1);
    myOLED.setTextSize(2); 
    myOLED.print("Cymbergaj");
    myOLED.setTextSize(1);
    myOLED.setCursor(1, Menu.initY+ Menu.deltaY);
    myOLED.print("> ");

    for (int i=0; i< Menu.rowMax; i++) {
      if (Menu.rowMax > SelectedRow+i) {
        myOLED.setCursor(Menu.initX, Menu.initY + Menu.deltaY*(i+1));
        myOLED.print(Menu.optionString[SelectedRow+i][0]);
        myOLED.print(Menu.optionString[SelectedRow+i][Menu.rowOptionSelect[SelectedRow+i]]);
      }
    }
  }

  void UserMenuParamsUpdate(cymMenuStruct& Menu, gameStruct& CymGame) {
    CymGame.playersNo = Menu.menuOptionValue[0][Menu.rowOptionSelect[0]-1];
    CymGame.difficulty = (level)Menu.menuOptionValue[1][Menu.rowOptionSelect[1]-1];
    CymGame.aiLevel = Menu.menuOptionValue[2][Menu.rowOptionSelect[2]-1];
    switch(CymGame.difficulty) {
      case Lekko:
        CymGame.padleHalfWidth = 14;
        break;
      case Trudniej:
        CymGame.padleHalfWidth = 11;
        break;
      case Master:
        CymGame.padleHalfWidth = 8;
        break;
    }
  }

  bool ItsTimeForBallPosUpdate(cymBallStruct& Ball) {
    return TimerElapsed(Ball.prevUpdateTime, Ball.updateRate);
  }

  void UpdateBallXpos(cymBallStruct& Ball) {
    switch (Ball.isHeading) {
      case toLeft:
      case toLeftDown:
      case toLeftUp:
        Ball.x = Ball.x - DELTA_X;
      break;

      case toRight:
      case toRightDown:
      case toRightUp:
        Ball.x = Ball.x + DELTA_X;
      break;
    }
  }

  void UpdateBallYpos(cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    switch (Ball.isHeading) {
      case toRightUp:
      case toLeftUp:
        Ball.y = Ball.y - Ball.dY;
        break;
      case toRightDown:
      case toLeftDown:
        Ball.y = Ball.y + Ball.dY;
        break;
      case toLeft:
      case toRight:
        Ball.y = Ball.y;
        break;
    }
  }

  bool ItsTimeForPadlePosUpdate(gameStruct& CymGame) {
    unsigned long FinalUpdateRate = CymGame.playerPadlePosUpdateRate;

    if (CymGame.ballSpeed == Szybko) FinalUpdateRate = CymGame.playerPadlePosUpdateRate / 2;

    return TimerElapsed(CymGame.playerPadlePosPrevUpdateTime, FinalUpdateRate);
  }

  void CheckIfBallHitBand(cymBallStruct& Ball) {
    if (Ball.y <=2) {
      if (Ball.isHeading == toLeftUp ) Ball.isHeading = toLeftDown;
      if (Ball.isHeading == toRightUp ) Ball.isHeading = toRightDown;
      Ball.y = 3;
    }
    if (Ball.y >= FieldYmax-2) {
      if (Ball.isHeading == toLeftDown ) Ball.isHeading = toLeftUp;
      if (Ball.isHeading == toRightDown ) Ball.isHeading = toRightUp;
      Ball.y = FieldYmax-4;
    }
  };

  void DisplayGameSummary(gameStruct& CymGame, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    myOLED.clearDisplay();
    myOLED.setCursor(20, 20);
    myOLED.setTextSize(1); 
    if (CymGame.playersNo == 2) {
      if (LeftPlayer.score >= CymGame.maxPoints) {
        myOLED.print("Lewa WYGRYWA!");
      } else {
        myOLED.print("Prawa WYGRYWA!");
      }
    } else if (CymGame.playersNo == 1) {
      if (LeftPlayer.score > RightPlayer.score && LeftPlayer.score >= CymGame.maxPoints) {
        myOLED.print("Wygrales z AI!");
      } else if (RightPlayer.score == LeftPlayer.score && LeftPlayer.score == CymGame.maxPoints) {
        myOLED.print("REMIS!");
      }else {
        myOLED.print("AI Cie pokonal!");
      }
    } else if (CymGame.playersNo == 0) {
      myOLED.print("To tylko demo ;)");
    }
    myOLED.display();
    delay(800);
  }

  void DisplayScore(gameStruct& CymGame, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    myOLED.setCursor(HalfFieldX-16, 5);
    myOLED.print(LeftPlayer.score);
    myOLED.setCursor(HalfFieldX+12, 5);
    myOLED.print(RightPlayer.score);

    if (CymGame.playersNo < 2) {
      myOLED.setCursor(HalfFieldX+12, FieldYmax-10);
      myOLED.print("AI");
    }
    if (CymGame.playersNo == 0) {
      myOLED.setCursor(HalfFieldX-16, FieldYmax-10);
      myOLED.print("AI");
    }
  }

  void DrawGameScreen(gameStruct& CymGame, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    myOLED.clearDisplay();
    myOLED.setTextColor(SH110X_WHITE);
    DrawBall(Ball);
    DrawField();
    DrawPadle(CymGame, LeftPlayer, RightPlayer);
    DisplayScore(CymGame, LeftPlayer, RightPlayer);
    myOLED.display();
    CymGame.updateScreen = false;
  }

  void DisplayGoal(gameStruct& CymGame, cymBallStruct& Ball, cymPlayerStruct& LeftPlayer, cymPlayerStruct& RightPlayer) {
    DrawBall(Ball);
    Ball.prevX = Ball.x;
    Ball.prevY = Ball.y;
    myOLED.display();
    myOLED.setTextSize(1);
    Ball.dY = Ball.dY/2;

    int xStringField = 1;
    if (Ball.x < HalfFieldX) xStringField = -1;
      
    for (int i = 0; i < 5; i++) {
      Ball.x = Ball.x + xStringField*2;
      UpdateBallYpos(Ball, LeftPlayer, RightPlayer);
      myOLED.fillRect(HalfFieldX+xStringField*HalfFieldX-3, 2, 10, FieldYmax-3, SH110X_BLACK);
      DrawPadle(CymGame, LeftPlayer, RightPlayer);
      DrawBall(Ball);
      myOLED.setTextColor(SH110X_WHITE);
      myOLED.setCursor(63+xStringField*50, 5);
      myOLED.print("R");
      myOLED.setCursor(49-xStringField*29, 25);
      myOLED.print("GOAL!");
      myOLED.display();
      if (SoundEnabled) {
        tone(BUZZER_PIN, TON_PALETKA_FREQ, DELAY250MS);
      } else delay(DELAY100MS);
      noTone(BUZZER_PIN);
      Ball.prevX = Ball.x;
      Ball.prevY = Ball.y;
      myOLED.setTextColor(SH110X_BLACK);
      myOLED.setCursor(63+xStringField*50, 5);
      myOLED.print("R");
      myOLED.display();
      delay(130);
    }
    myOLED.setTextColor(SH110X_WHITE);
  }

} // namespace CymbergajGame

const GameInfo GameInfo_Cymbergaj = {
  "Cymbergaj",
  "Klasyczny air hockey na OLED",
  CymbergajGame::Game_Cymbergaj,
  Game_CymbergajRecord
};