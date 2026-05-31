
#include "Slalom.h"
#include "Slalom_private.h"

namespace SlalomGame {

  SpriteCode NewRow_Array[4] = {FUEL,FUEL,FUEL,FUEL};
  SpriteCode Sprites_Array[6][4] = {{CLEAN, CLEAN, CLEAN, CLEAN}, 
                                    {CLEAN, CLEAN, CLEAN, CLEAN},
                                    {CLEAN, CLEAN, CLEAN, CLEAN},
                                    {CLEAN, CLEAN, CLEAN, CLEAN},
                                    {CLEAN, CLEAN, CLEAN, CLEAN}};


  static const uint8_t RowCode[14] = {1, 2, 4, 8, 3, 5, 6, 9, 10, 12, 7, 11, 13, 14};

  const int sprite_x[5][4] = {{23,42,61,80},{20,41,62,83},{17,40,63,86},{14,39,64,89},{4,34,63,92}};
  const int sprite_y[5] = {-4,4,13,27,40};

  // Caly stan rozgrywki w jednej strukturze (zgrupowany, jak w innych grach).
  SlalomState S;

  void display_roadLine() {
    S.LineSwitch = !S.LineSwitch;
	    myOLED.drawLine(95,64, 83, 0, SH110X_BLACK);
      myOLED.drawLine(64,0, 64, 64, SH110X_BLACK);
      myOLED.drawLine(33,64, 45, 0, SH110X_BLACK);
    if (S.LineSwitch) {
      myOLED.drawLine(92,48, 89, 32, SH110X_WHITE);
      myOLED.drawLine(86,16, 83, 0, SH110X_WHITE);

      myOLED.drawLine(64,0, 64, 8, SH110X_WHITE);
      myOLED.drawLine(64,17, 64, 24, SH110X_WHITE);
      myOLED.drawLine(64,33, 64, 40, SH110X_WHITE);
      myOLED.drawLine(64,49, 64, 56, SH110X_WHITE);
      
      myOLED.drawLine(33,64, 36, 48, SH110X_WHITE);
      myOLED.drawLine(39,32, 42, 16, SH110X_WHITE); 
    }
    if (!S.LineSwitch) {
      myOLED.drawLine(95,64, 92, 48, SH110X_WHITE);
      myOLED.drawLine(89,32, 86, 16, SH110X_WHITE); 

      myOLED.drawLine(64,9, 64, 16, SH110X_WHITE);
      myOLED.drawLine(64,25, 64, 32, SH110X_WHITE); 
      myOLED.drawLine(64,41, 64, 48, SH110X_WHITE);
      myOLED.drawLine(64,57, 64, 64, SH110X_WHITE);

      myOLED.drawLine(36,48, 39, 32, SH110X_WHITE);
      myOLED.drawLine(42,16, 45, 0, SH110X_WHITE);
    }
  }
  
  void LivesLeftDisplay(int k, uint8_t color) {
    myOLED.drawBitmap(120, 1, live_yes, 5, 6, SH110X_WHITE);
    myOLED.setTextColor(color);
    myOLED.setCursor(113,0);
    myOLED.print(k);
  }

  void GameSlalomInit() {
    S.LivesLeft = LivesInit;
    S.FuelLevel = FuelInit;
    S.GameLevel = 0;
    S.car_x = random(1,5);
    Score = 0;
    S.FrameDelay = DelayFramesInit;
    S.OldScoreForLivesAdding =0;
    S.LastFuelBlinkTime = millis();
    S.FuelBlinkState = false;

    for (int i = 0; i<6; i++) {
      for (int j = 0; j<4;j++) {
        Sprites_Array[i][j] = CLEAN;
      }
    }
    DisplaySoundInfo(1,24, SoundEnabled);
    DisplayRoad();
    DisplayInitTrack(SH110X_WHITE);
    displayPoints();
    display_roadLine();
    LivesLeftDisplay(S.LivesLeft, SH110X_WHITE);
  }

  void DisplayInitTrack(uint8_t color) {
    for (int row = 0; row < 5; row++){
      for (int column = 0; column < 4; column++) {
        DisplaySprite(row, column, Sprites_Array[row][column], color);
      }
    }
  }
  void DrawAddedFuelStation(uint8_t color) {
    for (int row = 0; row < 5; row++){
      int column = 3;
      DisplaySprite(row, column, Sprites_Array[row][column], color);
    }
  }

  void DisplaySprite(int row, int column, SpriteCode Sprite, uint8_t color) {
    int x;
    int y;
    GetSprite_x_y(row, column, &x, &y);
    switch(Sprite) {
      case(GATE):
        if (row == 0)         {myOLED.drawBitmap(x, y, bramka1, sprite_width, Sprite_height, color);
        } else if (row == 1)  {myOLED.drawBitmap(x, y, bramka2, sprite_width, Sprite_height, color);
        } else if (row == 2)  {myOLED.drawBitmap(x, y, bramka3, sprite_width, Sprite_height, color);
        } else if (row == 3)  {myOLED.drawBitmap(x, y, bramka4, sprite_width, Sprite_height, color);}
        break;
      case(FUEL):
        if (row == 0)         {
          myOLED.drawBitmap(x, y, fuel1, sprite_width, Sprite_height, color);
        }
        else if (row == 1)  {
          myOLED.drawBitmap(x, y, fuel2, sprite_width, Sprite_height, color);
        } else if (row == 2)  {
          myOLED.drawBitmap(x, y, fuel2, sprite_width, Sprite_height, color);
        } else if (row == 3)  {
          myOLED.drawBitmap(x, y, fuel3, sprite_width, Sprite_height, color);
        }
        break;
      case(OIL):
        if (row == 0)         {myOLED.drawBitmap(x, y, oil1, sprite_width, Sprite_height, color);
        } else if (row == 1)  {myOLED.drawBitmap(x, y, oil2, sprite_width, Sprite_height, color);
        } else if (row == 2)  {myOLED.drawBitmap(x, y, oil3, sprite_width, Sprite_height, color);
        } else if (row == 3)  {myOLED.drawBitmap(x, y, oil4, sprite_width, Sprite_height, color);
        }
        break;
      case(CLEAN): 
      default:
        break;
    }
  }

  void DisplayCar(int car_x) {
    if (car_x<1 || car_x>4) return;
    EraseCarFromDisplay();
    switch(car_x) {
      case(1):
      case(2):
        myOLED.drawBitmap(sprite_x[4][car_x-1], sprite_y[4], motor_2, f1_width, f1_height, SH110X_WHITE);
        break;
      case(3):
        myOLED.drawBitmap(sprite_x[4][car_x-1], sprite_y[4], motor_2, f1_width, f1_height, SH110X_WHITE);
        break;
      case(4):
        myOLED.drawBitmap(sprite_x[4][car_x-1], sprite_y[4], motor_2, f1_width, f1_height, SH110X_WHITE);
        break;
      default:
        break;
    }
  }

  void DisplayRoad() {
    myOLED.drawLine(sprite_x[4][0]-3, SCREEN_HEIGHT, sprite_x[0][0]+4, 0, SH110X_WHITE);
    myOLED.drawLine(sprite_x[4][0]-4, SCREEN_HEIGHT, sprite_x[0][0]+3, 0, SH110X_WHITE);

    myOLED.drawLine(sprite_x[4][3]+f1_width, SCREEN_HEIGHT, sprite_x[0][3]+sprite_width-5, 0, SH110X_WHITE);
    myOLED.drawLine(sprite_x[4][3]+f1_width-1, SCREEN_HEIGHT, sprite_x[0][3]+sprite_width-6, 0, SH110X_WHITE);
  }

  void GetSprite_x_y(int row, int column, int *x, int *y) {
    *x = sprite_x[row][column];
    *y = sprite_y[row];
  }

  bool CheckFuelBlink() {
    if (S.FuelLevel <= 14) {
      if (TimerElapsed(S.LastFuelBlinkTime, 300)) { // Miga co 300ms
        S.FuelBlinkState = !S.FuelBlinkState;
        return true;
      }
    } else {
      S.FuelBlinkState = false; // Zawsze widoczny gdy fuel > 14
    }
    return false;
  }

  void displayPoints() {
    myOLED.setTextSize(1);
    
    // Sprawdź czy trzeba zmienić stan migania
    CheckFuelBlink();
    
    // Wyczyść poprzedni tekst
    myOLED.setTextColor(SH110X_BLACK);
    myOLED.setCursor(0,0);
    myOLED.print("F");
    myOLED.print(S.FuelLevel+1);
    
    // Wyświetl nowy tekst tylko jeśli powinien być widoczny
    if (S.FuelLevel > 14 || S.FuelBlinkState) {
      myOLED.setTextColor(SH110X_WHITE);
      myOLED.setCursor(0,0);
      myOLED.print("F");
      myOLED.print(S.FuelLevel);
    }
  }

  void RedrawScreen() {
    display_roadLine();
    DisplayInitTrack(SH110X_BLACK);

    UpdateRaceArray();
    DisplayInitTrack(SH110X_WHITE);
  }

  void UpdateRaceArray() {
    SpriteCode tempArray[4] ={CLEAN,CLEAN,CLEAN,CLEAN};

    for (int column = 0; column < 4; column++) {
      Sprites_Array[4][column] = Sprites_Array[3][column]; //z 3 na 4 czyli kolizyjny
    }
    for (int column = 0; column < 4; column++) {
      Sprites_Array[3][column] = Sprites_Array[2][column]; //z 2 na 3
    }
    for (int column = 0; column < 4; column++) {
      Sprites_Array[2][column] = Sprites_Array[1][column]; //z 1 na 2
    }
    for (int column = 0; column < 4; column++) {
      Sprites_Array[1][column] = Sprites_Array[0][column]; //z 0 na 1
    }
    
    GenerateNextRow(tempArray);

    for (int column = 0; column < 4; column++) {
      Sprites_Array[0][column] = tempArray[column]; //
    }
  }

  void NewRowFromCode(int Code) {
    for (int i = 0; i < 4; i++) {
      NewRow_Array[i] = CLEAN;
      if ((Code >> i) & 1) NewRow_Array[i] = GATE; 
    }
    return;
  }

  void GenerateNextRow(SpriteCode Array[4]) {
    int KodWiersza;
    int LosLevelRow;
    switch (S.GameLevel) {
      case(0):
        //jedno auto na wiersz
        KodWiersza = random(0,4);
        NewRowFromCode(RowCode[KodWiersza]);
        break;
      case(1):
        if (random(1,3) == 1) {KodWiersza = random(0,10);}
        else {KodWiersza = random(0,4);}
        NewRowFromCode(RowCode[KodWiersza]);
        break;
      case(2):
        LosLevelRow = random(1,3);
        if (LosLevelRow == 1) {KodWiersza = random(0,4);}
        else if (LosLevelRow == 2) {KodWiersza = random(0,10);}
        else {KodWiersza = random(0,14);}
        NewRowFromCode(RowCode[KodWiersza]);
        break;
    }

    for (int j=0; j<4; j++) {
      Array[j] = NewRow_Array[j];
    }
  }

  void EraseCarFromDisplay() {
    myOLED.drawBitmap(sprite_x[4][0], sprite_y[4], motor_2, f1_width, f1_height, SH110X_BLACK);
    myOLED.drawBitmap(sprite_x[4][1], sprite_y[4], motor_2, f1_width, f1_height, SH110X_BLACK);
    myOLED.drawBitmap(sprite_x[4][2], sprite_y[4], motor_2, f1_width, f1_height, SH110X_BLACK);
    myOLED.drawBitmap(sprite_x[4][3], sprite_y[4], motor_2, f1_width, f1_height, SH110X_BLACK);
  }

  bool CheckIfNextFrame() {
    return TimerElapsed(S.LastTime, S.FrameDelay);
  }

  bool CheckIfRoadLanesSwitch() {
    return TimerElapsed(S.LastTime2, S.FrameDelay);
  }

  void CheckIfColissionHappen() {
    switch(Sprites_Array[4][S.car_x-1]) {
      case(CLEAN):
        if (SoundEnabled) MyTune(TON_RAMKA_FREQ,30);
        Score++;
        S.FuelLevel--;
        break;
      case(GATE):
        if (SoundEnabled) MyTune(TON_ODLICZANIE_FREQ,30);
        S.FuelLevel--;
        LivesLeftDisplay(S.LivesLeft, SH110X_BLACK);
        S.LivesLeft--;
        LivesLeftDisplay(S.LivesLeft, SH110X_WHITE);
        ColissionDetected();
        break;
      case (FUEL):
        DisplayTankowanie();
        break;
      default:
        break;
    }
  }

  void DisplayTankowanie() {
    DisplayCar(S.car_x);
    myOLED.drawBitmap(117, 30, f1_tanking, 18, 18, SH110X_WHITE);
    for (int i = S.FuelLevel; i < S.FuelLevel +1+ IncreaseFuel; i++) {
      if (SoundEnabled) MyTune(TON_KONIECGRY_FREQ+i, 4);

      myOLED.setTextColor(SH110X_WHITE);
      myOLED.setCursor(0,0);
      myOLED.print("F");
      myOLED.print(i);
      myOLED.display();
      delay(4);
      myOLED.setTextColor(SH110X_BLACK);
      myOLED.setCursor(0,0);
      myOLED.print("F");
      myOLED.print(i);
      myOLED.display();
    }
    myOLED.setTextColor(SH110X_WHITE);
    myOLED.drawBitmap(117, 30, f1_tanking, 18, 18, SH110X_BLACK);
    S.FuelLevel = S.FuelLevel -1 + IncreaseFuel;
    S.FrameDelay = DelayFramesInit;
    myOLED.display();
  }

  void ResetRaceArray() {
    for (int column = 0; column < 4; column++) {
      Sprites_Array[0][column] = Sprites_Array[2][column];
    }
    for (int column = 0; column < 4; column++) {
      Sprites_Array[1][column] = Sprites_Array[3][column];
    }
    for (int column = 0; column < 4; column++) {
      Sprites_Array[2][column] = CLEAN;
    }
    for (int column = 0; column < 4; column++) {
      Sprites_Array[3][column] = CLEAN;
    }
    for (int column = 0; column < 4; column++) {
      Sprites_Array[4][column] = CLEAN;
    }
  }

	void ColissionDetected() {
    DisplayInitTrack(SH110X_BLACK);
    ResetRaceArray();
    DisplayInitTrack(SH110X_WHITE);
    displayPoints();
    EraseCarFromDisplay();
    myOLED.drawBitmap(sprite_x[4][S.car_x-1], sprite_y[4], Wrak, f1_width, f1_height, SH110X_WHITE);
    myOLED.display();
    delay(400);
    myOLED.drawBitmap(sprite_x[4][S.car_x-1], sprite_y[4], Wrak, f1_width, f1_height, SH110X_BLACK);

  }

  void GameOverSlalomDisplay() {
    myOLED.setTextSize(2);
    myOLED.setTextColor(SH110X_WHITE); 
    myOLED.clearDisplay();
    if (Score > S.SesionScore) {
      S.SesionScore = Score;
    }
    int Slalom_highScore;
    EEPROM.get(Game_SlalomRecord, Slalom_highScore);
    if (Score > Slalom_highScore) {
      EEPROM.put(Game_SlalomRecord, Score);
      EEPROM.commit();
    }
    if (S.FuelLevel == 0) {
      myOLED.setCursor(10,0);
      myOLED.print("No fuel!");
    }

    myOLED.setTextSize(1); 
    myOLED.setCursor(1,22);
    myOLED.print("Your Score:   ");
    myOLED.print(Score);
    myOLED.setCursor(1,34);
    myOLED.print("Sesion best:  ");
    myOLED.print(S.SesionScore);
    myOLED.setCursor(1,46);
    myOLED.print("All time best: ");
    EEPROM.get(Game_SlalomRecord, Slalom_highScore);
    myOLED.print(Slalom_highScore);
    myOLED.display();
  }

  void CleanRow5() {
    for (int column = 0; column < 4; column++) {
      Sprites_Array[4][column] = CLEAN;
    }
  }

  void CalculateGameSpeedAndLevel() {
    for (const SpeedStep& step : SpeedTable) {
      if (Score >= step.scoreThreshold) S.FrameDelay = step.frameDelay;
    }
    if (Score >= SlalomLevel1Score) S.GameLevel = 1;
    if (Score >= SlalomLevel2Score) S.GameLevel = 2;
  }

  void checkIfGameOver() {
    if (S.LivesLeft == 0 || S.FuelLevel == 0) S.GameOver=true;
    if (Score - S.OldScoreForLivesAdding > 40) {
      LivesLeftDisplay(S.LivesLeft, SH110X_BLACK);
      S.LivesLeft++;
      LivesLeftDisplay(S.LivesLeft, SH110X_WHITE);
      for (int i=1; i<4; i++){
        if (SoundEnabled) MyTune(TON_ODLICZANIE_FREQ+10*i, 30);
        delay(20);
      }
      S.OldScoreForLivesAdding = Score;
    }
  }

  void AddFuelStation() {
    if (Sprites_Array[0][3] == CLEAN && 
        Sprites_Array[1][3] == CLEAN && 
        S.FuelLevel < 70 &&
        Sprites_Array[1][3] != FUEL && 
        Sprites_Array[2][3] != FUEL && 
        Sprites_Array[3][3] != FUEL) {
      DrawAddedFuelStation(SH110X_BLACK);
      Sprites_Array[0][3] = CLEAN;
      Sprites_Array[1][3] = FUEL;
      Sprites_Array[2][3] = CLEAN;
      Sprites_Array[3][3] = CLEAN;
      DrawAddedFuelStation(SH110X_WHITE);
    }
  }

  void Pause(){
    myOLED.setTextSize(1); 
    myOLED.setCursor(40,28);
    myOLED.setTextColor(SH110X_WHITE);
    myOLED.print("PAUSE");
    myOLED.display();
    while (!IsPressed(DownRight));

    myOLED.setCursor(40,28);
    myOLED.setTextColor(SH110X_BLACK);
    myOLED.print("PAUSE");
    myOLED.setTextColor(SH110X_WHITE);
  }

// //############################################

void WelcomeSlalomScreen() {
  myOLED.clearDisplay();
  myOLED.setTextColor(SH110X_WHITE);
  myOLED.setTextSize(2);
  myOLED.setCursor(2, 1);
  myOLED.println("Just ride");
  myOLED.setTextSize(1);
  myOLED.setCursor(45, 31);
  myOLED.println("    pause > o");

  myOLED.setCursor(0, 42);
  myOLED.println("o > left");

  myOLED.setCursor(45, 42);
  myOLED.println("    right > o");
  myOLED.display();
  delay(DELAY1500MS);
} 

void Game_Slalom() {

  while(1==1) {  
    S.GameOver = false;

    WelcomeSlalomScreen();
    myOLED.clearDisplay();
    GameSlalomInit();
    DisplaySoundInfo(1,24, SoundEnabled);

    myOLED.display();

    while (!S.GameOver){
      bool carMoved = false;
      carMoved = CheckButtonsSlalom();
 
      if (CheckIfNextFrame()) {
        RedrawScreen();
        displayPoints();
        DisplayCar(S.car_x);
        CheckIfColissionHappen();
        CleanRow5();
        AddFuelStation();
        myOLED.display();
      } else if (carMoved) {
        // Auto się poruszyło, ale nie ma nowej klatki - tylko przemaluj auto
        DisplayCar(S.car_x);
        myOLED.display(); // Natychmiast odśwież ekran po ruchu
      }
      CalculateGameSpeedAndLevel();
      checkIfGameOver();
    }
    if (S.GameOver) GameOverSlalomDisplay();
    while (!IsPressed(DownRight));
  }
}

bool CheckButtonsSlalom(){
  btPressedCode btn;
  bool carMoved = false;

  btn = ReadButton(nullptr, Timer1Sec);
  if (!S.ButtonPressed) {
    if (btn == UpRight){
      S.ButtonPressed = true;
      Pause();
      return false;
    }
    if (btn == DownLeft) {
      S.ButtonPressed = true;
      if (S.car_x > 1) {
        S.car_x--;
        carMoved = true;
      }
    } else if (btn == DownRight) {
        S.ButtonPressed = true;
      if (S.car_x < 4) {
        S.car_x++;
        carMoved = true;
      }
    }
  } else {
    if (btn == NONE) S.ButtonPressed = false;
  }
  return carMoved;
}

} // namespace SlalomGame

// GameInfo musi być poza namespace, żeby był dostępny globalnie
const GameInfo GameInfo_Slalom = {
	"Slalom",
	"Zjeżdżaj, omijaj bramki i przeszkody!",
	SlalomGame::Game_Slalom,
	Game_SlalomRecord
};