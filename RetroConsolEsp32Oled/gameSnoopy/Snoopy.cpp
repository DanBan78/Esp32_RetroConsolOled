#include "Snoopy.h"
#include "Snoopy_private.h"

namespace SnoopyGame {

void Game_Snoopy() {
	snoopyStr Snoopy;
  BallLineStr allBallLines[4];

  Snoopy.sesionScore = 0;
  while (true) {
    WelcomeSnoopyScreen();
    SnoopyInit(Snoopy, allBallLines); 
    WaitForAnyButtonToContinue();
    Snoopy.prevUpdateTime  = millis();

    while (Snoopy.lives!=0) {
      DisplaySoundInfo(120, 0, SoundEnabled);
      if (ItsTimeToMoveBall(Snoopy)) {
        DisplayBallsOnScreen(allBallLines);
        CheckIfBallDropped(Snoopy, allBallLines);
        // update game speed every 10 points
        if (Score > 30) Snoopy.frameUpdateRate = Snoopy.initFrameUpdateRate - (Score/10)*15;
        if (Snoopy.lives == 0) break;
        if (NewBallIsRequired(Snoopy, allBallLines)) {
          for (int i = 0; i < 4; i++) {
            DrawBalls(allBallLines[i], 1);
          }
        }
      }
      DisplayScore(Snoopy);
      CheckIfSnopyMoved(Snoopy);
      myOLED.display();
    }
  }
}

void WelcomeSnoopyScreen() {
  myOLED.clearDisplay();
  myOLED.setTextColor(SH110X_WHITE);
  myOLED.setTextSize(2);
  myOLED.setCursor(2, 1);
  myOLED.println("Snoopy");
  myOLED.setTextSize(1);
  myOLED.setCursor(0, 20);
  myOLED.println("o > left up shelf");
  myOLED.setCursor(0, 31);

  myOLED.println("o > left down shelf");
  myOLED.setCursor(2, 44);
  myOLED.println("  right up shelf > o");
  myOLED.setCursor(2, 55);

  myOLED.println("right down shelf > o");
  myOLED.display();
  delay(DELAY1000MS);
} 

void AddFirstBall(BallLineStr allBallLines[4]) {
  orientation newBallOnLine = (orientation)random(0, 4);
  allBallLines[newBallOnLine].position |= 1;
}

void CheckIfSnopyMoved(snoopyStr& Snoopy) {
  orientation prevPos = Snoopy.direction;

  if (IsPressed(UpLeft)) Snoopy.direction = sLeftUp;
  if (IsPressed(DownLeft)) Snoopy.direction = sLeftDown;
  if (IsPressed(UpRight)) Snoopy.direction = sRightUp; 
  if (IsPressed(DownRight)) Snoopy.direction = sRightDown;
  
  DrawSnoopy(prevPos, SH110X_BLACK);
  DrawSnoopy(Snoopy.direction, SH110X_WHITE);
}

void DrawSnoopy(orientation Pos, uint16_t color) {
    switch(Pos) {
    case sLeftDown:
      myOLED.drawBitmap(30, 13, SnoopyLD, 50, 50, color);
      break;
    case sLeftUp:
      myOLED.drawBitmap(31, 10, SnoopyLU, 50, 50, color);
      break;
    case sRightDown:
      myOLED.drawBitmap(45, 13, SnoopyRD, 50, 50, color);
      break;
    case sRightUp:
      myOLED.drawBitmap(44, 10, SnoopyRU, 50, 50, color);
      break;
    case sWaiting:
      myOLED.drawBitmap(37, 10, SnoopySit, 50, 50, color);
      break;
    case sResting:
      myOLED.drawBitmap(37, 10, SnoopyNap, 50, 50, color);
      break;
  }
}

void DisplayScore(snoopyStr& Snoopy) {
  myOLED.fillRect(0, 0, 60,12, SH110X_BLACK);
  myOLED.setTextColor(SH110X_WHITE);
  myOLED.setTextSize(1);
  myOLED.setCursor(0,0);
  myOLED.print(F("Balls: "));
  myOLED.print(Score);
}

void DisplayBallsOnScreen(BallLineStr allBallLines[4]) {
  bool BallsOnScreen = false;

  for (int i = 0; i < 4; i++) {
    DrawBalls(allBallLines[i], 0);
    allBallLines[i].position = allBallLines[i].position << 1;
    DrawBalls(allBallLines[i], 1);
  }
  for (int i = 0; i < 4; i++) {
    if (allBallLines[i].position != 0) {
      BallsOnScreen = true;
      break;
    }
  }
  if (SoundEnabled && BallsOnScreen) {
    MyTune(TON_Ball_FREQ, TON_Ball_CZAS);
  } else delay(TON_Ball_CZAS);
}

void DrawBalls(BallLineStr& BallLine, uint16_t color){
  for (int i=0; i<7; i++) {
    if ((BallLine.position & (1 << i)) != 0) {
      int x=0+i*5; 
      int y=17+i;
      switch(BallLine.direction) {
        case sLeftUp:
          myOLED.drawCircle(x, y, Ball_DIMENSION, color);
        break;
        case sLeftDown:
          myOLED.drawCircle(x, y+20, Ball_DIMENSION, color);
        break;
        case sRightUp:
          myOLED.drawCircle(127-x, y, Ball_DIMENSION, color);
        break;
        case sRightDown:
          myOLED.drawCircle(127-x, y+20, Ball_DIMENSION, color);
        break;
      }
    }
  }
}

bool ItsTimeToMoveBall(snoopyStr& Snoopy) {
  return TimerElapsed(Snoopy.prevUpdateTime, Snoopy.frameUpdateRate);
}

void SnoopyInit(snoopyStr& Snoopy, BallLineStr allBallLines[4]) {
  myOLED.clearDisplay();
  //draw lives circles
  myOLED.drawCircle(2, 58, 2, SH110X_WHITE);
  myOLED.drawCircle(10, 58, 2, SH110X_WHITE);
  myOLED.drawCircle(18, 58, 2, SH110X_WHITE);
  //draw lines
  int y = 21, dy0 = 5, dy1 = 20, dx = 30;
  for (int line = 0; line < 2; line++) {           // 2 linie (góra/dół)
    for (int thick = 0; thick < 2; thick++) {      // podwójna grubość
      int currentY = y + line * dy1 + thick;
      myOLED.drawLine(0, currentY, dx, currentY + dy0, SH110X_WHITE);           // lewa
      myOLED.drawLine(127, currentY, 127-dx, currentY + dy0, SH110X_WHITE);     // prawa  
    }
  }
  // init values
  Snoopy.lives = 3;
  Score = 0;
  Snoopy.direction = sResting;
  Snoopy.frameUpdateRate = Snoopy.initFrameUpdateRate;
  DisplayScore(Snoopy);
  DrawSnoopy(Snoopy.direction, SH110X_WHITE);

  ClearBallLines(allBallLines);
  AddFirstBall(allBallLines);
  myOLED.display();
}

void ClearBallLines(BallLineStr allBallLines[4]) {
  for (int i = 0; i < 4; i++) {
    DrawBalls(allBallLines[i], 0);
    allBallLines[i].position = 0;
    allBallLines[i].direction = (orientation)i;
  }
}

void CheckIfBallDropped(snoopyStr& Snoopy, BallLineStr allBallLines[4]) {
  for (int i = 0; i < 4; i++) {
    if ((allBallLines[i].position & 128) == 128) {
      if (Snoopy.direction == allBallLines[i].direction) {
        Score++;
        if(SoundEnabled) {
          MyTune(TON_POINT_FREQ, TON_POINT_CZAS);
        } else delay(TON_POINT_CZAS);
      } else {
        Snoopy.lives --;
        DisplayBrokenBall(Snoopy, allBallLines[i].direction);

        if (Snoopy.lives == 0) {
          DisplayGameOver(Snoopy);
          WaitForAnyButtonToContinue();
        }
        ClearBallLines(allBallLines);
      }
    }
  }
}

void DisplayBrokenBall(snoopyStr& Snoopy, orientation pos) {
  int x;
  int y = 44; 
  int z = 36;

  if (pos == sRightUp || pos == sRightDown) {
    x = 75;
  } else x = 30;
  if (pos == sLeftUp || pos == sRightUp) {
    myOLED.drawCircle(x + 5, z, Ball_DIMENSION, SH110X_WHITE);
    myOLED.display();
    delay(DELAY250MS);
  }
  myOLED.drawCircle(x + 5, z, Ball_DIMENSION, SH110X_BLACK);
  DrawSnoopy(Snoopy.direction, SH110X_BLACK);
  Snoopy.direction = sWaiting;
  DrawSnoopy(Snoopy.direction, SH110X_WHITE);
  myOLED.drawBitmap(x, y, BrokenBall_bmp, 23, 16, SH110X_WHITE);
  myOLED.display();
  if(SoundEnabled) {
    MyTune(TON_ERROR_FREQ, TON_ERROR_CZAS);
  } else delay(TON_ERROR_CZAS);
  
  myOLED.drawBitmap(x, y, BrokenBall_bmp, 23, 16, SH110X_BLACK);
  myOLED.drawCircle(2 + Snoopy.lives * 8, 58, 2, SH110X_BLACK);
}

bool NewBallIsRequired(snoopyStr& Snoopy, BallLineStr allBallLines[4]) {
  uint8_t totalBallsPos = 0;
  uint8_t BallsOnLinesCount = 0;
  orientation randomLineForNewBall;

  for (int i = 0; i < 4; i++) {
    BallsOnLinesCount += __builtin_popcount(allBallLines[i].position);
    totalBallsPos |= allBallLines[i].position;
  }

  if (Score < 5 || totalBallsPos == 128 || BallsOnLinesCount == 0) {
    if ((totalBallsPos == 128 && BallsOnLinesCount == 1) || (BallsOnLinesCount == 0)) {
      AddFirstBall(allBallLines);
      return true;
    }
    return false;

  } else if (Score < 20 || BallsOnLinesCount == 1) {
    if ((totalBallsPos & 0b00000111) != 0 || (BallsOnLinesCount == 2)) return false;
    AddFirstBall(allBallLines);
    return true;

  } else if ((Score >= 20) && (BallsOnLinesCount == 2)) {
    if ((totalBallsPos & 0b00000111) != 0) return false;
    while (1) {
      randomLineForNewBall = (orientation)random(0, 4);
      if ((allBallLines[randomLineForNewBall].position & 0b00000111) == 0) {
        allBallLines[randomLineForNewBall].position |= 1;
        return true;
      }
    }
  }
  return false;
}

void DisplayGameOver(snoopyStr& Snoopy) {
  // Aktualizuj session score
  if (Score > Snoopy.sesionScore) {
    Snoopy.sesionScore = Score;
  }
  
  // Sprawdź i zapisz highscore
  int highscore;
  EEPROM.get(Game_SnoopyRecord, highscore);
  
  // Walidacja - jeśli EEPROM niezainicjalizowane, ustaw 0
  if (highscore < 0 || highscore > 9999) {
    highscore = 0;
  }
  
  if (Score > highscore) {
    EEPROM.put(Game_SnoopyRecord, Score);
    EEPROM.commit();
    highscore = Score; // Aktualizuj lokalną wartość dla wyświetlenia
  }
  
  // Wyświetl ekran końca gry
  delay(500);
  myOLED.clearDisplay();
  myOLED.setTextColor(SH110X_WHITE);
  myOLED.setTextSize(2);
  myOLED.setCursor(10, 5);
  myOLED.println(F("Game Over"));
  
  myOLED.setTextSize(1);
  myOLED.setCursor(5, 25);
  myOLED.print(F("Your score: "));
  myOLED.print(Score);
  
  myOLED.setCursor(5, 37);
  myOLED.print(F("Session best: "));
  myOLED.print(Snoopy.sesionScore);
  
  myOLED.setCursor(5, 49);
  myOLED.print(F("All time best: "));
  myOLED.print(highscore);
  
  myOLED.display();
}

} // namespace SnoopyGame

const GameInfo GameInfo_Snoopy = {
  "Snoopy",
  "klasyczne lapanie jajek",
  SnoopyGame::Game_Snoopy,
  Game_SnoopyRecord
};