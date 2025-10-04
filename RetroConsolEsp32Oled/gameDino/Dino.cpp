#include "Dino.h"
#include "Dino_private.h"

namespace DinoGame {

  SpriteSetStr Skins[2] = {
    {dino1, dino2, dino3, Obst1, Obst2, Bird1, Bird2, Bird3, "Auuuc!!"},      // zestaw 1
    {cat1, cat2, cat3, murek1, murek2, bird1, bird2, bird3, "Miau!"} // zestaw 2
  };

  void Game_Dino() {
    dGameStr DinoGame;
    jumpStr  Dino;
    birdStr  Bird;
    obstStr  Obst1, Obst2;
    DinoGame.sessionScore = 0;

    while(true){
      static const GameConstStr GameConst;

      WelcomeScreen();
      while (1) {
        InitParams(DinoGame, Dino, Bird, Obst1, Obst2, GameConst);
        DisplayFirstFrame(DinoGame, Dino, Obst1, Obst2, GameConst);
        GameLoop(DinoGame, Dino, Bird, Obst1, Obst2, GameConst);
        DisplayGameSummary(DinoGame, Dino);
        WaitForAnyButtonToContinue();
        break;
      }
    }
  }

//####################################################################################

void InitParams(dGameStr& DinoGame, jumpStr& Dino, birdStr& Bird, obstStr& Obst1, obstStr& Obst2, const GameConstStr& GameConst) {
  Score = 0;

  Dino.y     = GameConst.jumper_baseY;
  Dino.food  = GameConst.jumper_foodInit;
  Dino.state = true;
  Dino.jump  = 0;

  DinoGame.speed     = 0;
  DinoGame.pause = false;

  Obst1.x        = GameConst.first_obst_x;
  Obst1.type     = OBST_NARROW;
  Obst2.x        = GameConst.new_obst_x_init;
  Obst2.type     = OBST_WIDE;

  Bird.x         = GameConst.new_obst_x_init;
  Bird.y         = GameConst.Bird_initY;
  Bird.state     = BirdUp;
  Bird.colission = false;
}

void WelcomeScreen(){
  myOLED.clearDisplay();
  myOLED.setTextColor(SH110X_WHITE);
  myOLED.setTextSize(2);
  myOLED.setCursor(10,1);
  myOLED.println("Dino Run");
  myOLED.setTextSize(1);
  myOLED.setCursor(0, 20);
  myOLED.println("o > change skin");
  myOLED.setCursor(0, 31);

  myOLED.println("o > high jump");
  myOLED.setCursor(45, 44);
  myOLED.println("    pause > o");
  myOLED.setCursor(45, 55);

  myOLED.println("long jump > o");
  myOLED.display();
}

void DisplayFirstFrame(dGameStr& DinoGame, jumpStr& Dino, obstStr& Obst1, obstStr& Obst2, const GameConstStr& GameConst) {
  myOLED.clearDisplay();
  myOLED.drawBitmap(GameConst.jumper_baseX, GameConst.jumper_baseY, dino1, GameConst.jumper_width, GameConst.jumper_height, SH110X_WHITE);
  MoveAndDisplayTree(DinoGame, Obst1, GameConst);
  MoveAndDisplayTree(DinoGame, Obst2, GameConst);
  myOLED.drawLine(0, GameConst.baselineY, 127, GameConst.baselineY, SH110X_WHITE);
  myOLED.setCursor(5, 8);
  myOLED.println("Dino jump, run!");
  myOLED.display();
  delay(500);
}

void UpdateGameFrame(dGameStr& DinoGame, jumpStr& Dino, birdStr& Bird, obstStr& Obst1, obstStr& Obst2, const GameConstStr& GameConst) {
  myOLED.clearDisplay();
  DisplayScore(DinoGame, Dino);
  DisplaySoundInfo(120, 0, SoundEnabled);
  if (Dino.food <= 0) DisplayOutOfFood();

  MoveAndDisplayTree(DinoGame,Obst1, GameConst);
  MoveAndDisplayTree(DinoGame,Obst2, GameConst);
  MoveAndDisplayDino(DinoGame, Dino, GameConst);
  MoveAndDisplayBird(DinoGame, Bird, GameConst);
  myOLED.drawLine(0, GameConst.baselineY, 127, GameConst.baselineY, SH110X_WHITE);
}

void DisplayOutOfFood(){
  myOLED.setTextSize(1);
  myOLED.setCursor(15,20);
  myOLED.print("No Food - no jump!");
}

// Main play function
void GameLoop(dGameStr& DinoGame, jumpStr& Dino, birdStr& Bird, obstStr& Obst1, obstStr& Obst2, const GameConstStr& GameConst) {
  while (true) {
    myOLED.clearDisplay();
    CheckButtons(DinoGame, Dino);
    if ((Dino.food <= 0) && (Dino.jump == 1)) Dino.jump = 0;
    if (Dino.jump != 0) HandleDinoJump(DinoGame, Dino, GameConst);
      
    UpdateGameFrame(DinoGame, Dino, Bird, Obst1, Obst2, GameConst);
    CheckIfBirdColision(DinoGame, Dino, Bird, GameConst);
    CalcTreesXpos(DinoGame, Obst1, Obst2, GameConst);
    CalcBirdXYpos(DinoGame, Bird, GameConst);
    CalcGameSpeed(DinoGame, GameConst);
    if (ItsCollisionWithTree(DinoGame, Dino, Obst1, GameConst) || ItsCollisionWithTree(DinoGame, Dino, Obst2, GameConst)) {
      GameOverTreeColisionDetected(DinoGame, Dino, GameConst);
      myOLED.display();
      delay(50);
      return;
    }
    myOLED.display();
    delay(GameConst.maxspeed - DinoGame.speed);
    if (DinoGame.pause) PauseGame(DinoGame);
  }
}
  
void PauseGame(dGameStr& DinoGame) {
  myOLED.setTextSize(2);
  myOLED.setCursor(20,18);
  myOLED.print("PAUSE");
  myOLED.display();
  myOLED.setTextSize(1);
  delay(300);
  DinoGame.pause = false;
  WaitForAnyButtonToContinue();
}

void TurboSpeed(dGameStr& DinoGame, bool direction, int16_t* sprite) {
  if (direction) {
    (*sprite)++;
    if (Score >60) (*sprite)++;
    if (Score >100) (*sprite)++;
    if (Score >150) (*sprite)++;
  } else {
    (*sprite)--;
    if (Score >60) (*sprite)--;
    if (Score >100) (*sprite)--;
    if (Score >150) (*sprite)--;
  }
}

void MoveAndDisplayDino(dGameStr& DinoGame, jumpStr& Dino, const GameConstStr& GameConst) {
  if (DinoGame.isRect) {
    int dinoCenterX       = GameConst.jumper_baseX + GameConst.jumper_width/2;
    int dinoColAreaStart  = dinoCenterX - GameConst.jumper_xColRange;
    int dinoColAreaWidth  = 2*GameConst.jumper_xColRange;
    int dinoTop           = Dino.y;
    int dinoBottom        = GameConst.jumper_height;
    myOLED.fillRect(dinoColAreaStart, dinoTop, dinoColAreaWidth, dinoBottom, SH110X_WHITE);
  } else {
    if (Dino.jump == 0) {
      if (Dino.state) {
        myOLED.drawBitmap(GameConst.jumper_baseX, Dino.y, Skins[DinoGame.spriteSetIndex].Jumper1, GameConst.jumper_width, GameConst.jumper_height, SH110X_WHITE);
      } else {
        myOLED.drawBitmap(GameConst.jumper_baseX, Dino.y, Skins[DinoGame.spriteSetIndex].Jumper2, GameConst.jumper_width, GameConst.jumper_height, SH110X_WHITE);
      }
      Dino.state = !Dino.state;
    } else {
      myOLED.drawBitmap(GameConst.jumper_baseX, Dino.y, Skins[DinoGame.spriteSetIndex].Jumper3, GameConst.jumper_width, GameConst.jumper_height, SH110X_WHITE);
    }
  }
}

void MoveAndDisplayBird(dGameStr& DinoGame, birdStr& Bird, const GameConstStr& GameConst) {
  if (DinoGame.isRect) {
    int BirdColAreaStart   = Bird.x;
    int BirdColAreaWidth  = GameConst.Bird_width;
    int BirdTop    = Bird.y;
    int BirdBottom = GameConst.Bird_height;
    myOLED.fillRect(BirdColAreaStart, BirdTop, BirdColAreaWidth, BirdBottom, SH110X_WHITE);
    if(Bird.colission) Bird.y+=4;
    return;
  } else {
    if (!Bird.colission) {
      if (random(0,2) == 1) {
        myOLED.drawBitmap(Bird.x, Bird.y, Skins[DinoGame.spriteSetIndex].Bird1, GameConst.Bird_width, GameConst.Bird_height, SH110X_WHITE);
      } else {
        myOLED.drawBitmap(Bird.x, Bird.y, Skins[DinoGame.spriteSetIndex].Bird2, GameConst.Bird_width, GameConst.Bird_height, SH110X_WHITE);
      }
    } else {
      Bird.y+=4;
      myOLED.drawBitmap(Bird.x, Bird.y, Skins[DinoGame.spriteSetIndex].Bird3, GameConst.Bird_width, GameConst.Bird_height, SH110X_WHITE);
    }
  }
}

void MoveAndDisplayTree(dGameStr& DinoGame, obstStr& tree, const GameConstStr& GameConst) {
  int treeCenterX       = tree.x + ObstParams[tree.type].width / 2;
  int treeColAreaStart  = treeCenterX - ObstParams[tree.type].xColisionRange;
  int treeColAreaWidth  = 2 * ObstParams[tree.type].xColisionRange;
  int treeTop           = GameConst.baselineY - ObstParams[tree.type].yColisionRange;
  int treeHeight        = ObstParams[tree.type].yColisionRange;

  if (DinoGame.isRect) {
    if(tree.type==OBST_NARROW) myOLED.fillRect(treeColAreaStart, treeTop, treeColAreaWidth, treeHeight, SH110X_WHITE);
    if(tree.type==OBST_WIDE) myOLED.fillRect(treeColAreaStart, treeTop, treeColAreaWidth, treeHeight, SH110X_WHITE);
  } else {
    if(tree.type==OBST_NARROW) myOLED.drawBitmap(tree.x, GameConst.obst_y, Skins[DinoGame.spriteSetIndex].ObstHigh, ObstParams[OBST_NARROW].width, ObstParams[OBST_NARROW].height, SH110X_WHITE);
    if(tree.type==OBST_WIDE) myOLED.drawBitmap(tree.x, GameConst.obst_y, Skins[DinoGame.spriteSetIndex].ObstWide, ObstParams[OBST_WIDE].width, ObstParams[OBST_WIDE].height, SH110X_WHITE);
  }
}

void GameOverTreeColisionDetected(dGameStr& DinoGame, jumpStr& Dino, const GameConstStr& GameConst) {
  myOLED.setTextSize(1);
  myOLED.setCursor((GameConst.jumper_baseX + GameConst.jumper_width + 4), Dino.y-4);
  myOLED.println(Skins[DinoGame.spriteSetIndex].collisionText);
  myOLED.display();
  if(SoundEnabled){
    tone(BUZZER_PIN,TON_PUNKT_CZAS,DELAY100MS);
    delay(DELAY100MS);
    tone(BUZZER_PIN,TON_PALETKA_FREQ,DELAY100MS);
    delay(DELAY100MS);
    tone(BUZZER_PIN,TON_ODLICZANIE_FREQ,DELAY250MS);
    delay(DELAY250MS);
    noTone(BUZZER_PIN);
  } else {
    delay(DELAY1000MS);
  }
  myOLED.clearDisplay();
}

void CheckIfBirdColision(dGameStr& DinoGame, jumpStr& Dino, birdStr& Bird, const GameConstStr& GameConst) {
  if ((((Bird.x < GameConst.jumper_baseX + GameConst.jumper_xColRange) ||
      (Bird.x + GameConst.Bird_width < GameConst.jumper_baseX - GameConst.jumper_xColRange) ) &&
      (Bird.y + GameConst.Bird_height > Dino.y)) && !Bird.colission) {
    Bird.colission = true;
    Dino.food = Dino.food + 5;
    if(SoundEnabled) {   
      tone(BUZZER_PIN,TON_PUNKT_FREQ,DELAY100MS);
      tone(BUZZER_PIN,TON_PUNKT_FREQ,DELAY100MS);
      noTone(BUZZER_PIN);
    } else delay(DELAY50MS);
  }
  if ((Bird.colission) && ((Bird.y < 64) || (Bird.x < -30))) {
    Bird.colission = false;
    Bird.x = GameConst.new_obst_x_init + random(0,40);
    Bird.y = GameConst.Bird_initY + random(-5,6);
  }
}

void HandleDinoJump(dGameStr& DinoGame, jumpStr& Dino, const GameConstStr& GameConst) {
  Serial.println("HandleDinoJump "+String(Dino.jump)+" height: "+String(GameConst.jumper_jumpHeight / Dino.jumpType));
  switch (Dino.jump) {
    case 1: // going up
      Dino.jumpHangTime = 0;
      if (SoundEnabled && !DinoGame.soundDone) void JumpSound();
      Dino.y -= 5;
      if (Dino.y < (GameConst.jumper_baseY - GameConst.jumper_jumpHeight / Dino.jumpType)) {
        Dino.y = (GameConst.jumper_baseY - GameConst.jumper_jumpHeight / Dino.jumpType);
        Dino.food--;
        if (Dino.jumpType == 1) Dino.food--;
        Dino.jump = 2;
      }
      break;
    case 2: // hang time
      Dino.jumpHangTime += 1;
      if (Dino.jumpHangTime > 5 * Dino.jumpType) Dino.jump = 3;
      break;
    case 3: // going down
      Dino.y += 5;
      if (Dino.y >= GameConst.jumper_baseY) {
        Dino.y = GameConst.jumper_baseY;
        Dino.jump = 0;
        Dino.jumpHangTime = 0;
        DinoGame.soundDone = false;
        Score++;
      }
      break;
    default:
      break;
  }
  //TurboSpeed(DinoGame,false, &Dino.y);
}


bool ItsCollisionWithTree(dGameStr& DinoGame, jumpStr& Dino, obstStr& tree, const GameConstStr& GameConst) {
  int dinoCenter        = GameConst.jumper_baseX + GameConst.jumper_width/2;
  int dinoColAreaStart  = dinoCenter - GameConst.jumper_xColRange;
  int dinoColAreaWidth  = 2 * GameConst.jumper_xColRange;
  int dinoTop           = Dino.y;
  int dinoBottom        = Dino.y + GameConst.jumper_height;
  int treeCenter        = tree.x + ObstParams[tree.type].width/2;
  int treeColAreaStart  = treeCenter - ObstParams[tree.type].xColisionRange;
  int treeColAreaWidth  = 2 * ObstParams[tree.type].xColisionRange;
  int treeTop           = GameConst.baselineY-ObstParams[tree.type].yColisionRange; 
  int treeHeight        = ObstParams[tree.type].yColisionRange;

  bool overlapX = (dinoColAreaStart + dinoColAreaWidth > treeColAreaStart) &&
                  (dinoColAreaStart < treeColAreaStart + treeColAreaWidth);
  bool overlapY = dinoBottom > treeTop;

  return overlapX && overlapY;
}

void CalcTreesXpos (dGameStr& DinoGame, obstStr& Obst1, obstStr& Obst2, const GameConstStr& GameConst) {
  int obstDistance = abs(Obst2.x-Obst1.x);
	Obst1.x-=2;
  TurboSpeed(DinoGame,false, &Obst1.x);
	Obst2.x-=2;
	TurboSpeed(DinoGame,false, &Obst2.x);

	if (Obst1.x<-ObstParams[Obst1.type].xColisionRange) {
		Obst1.x = GameConst.new_obst_x_init + random(0,20);
		if (random(0,2)==1) {
			Obst1.type =  true;
		} else Obst1.type = false;
	}
	if (Obst2.x<-ObstParams[Obst2.type].xColisionRange) {
		Obst2.x = GameConst.new_obst_x_init + random(0,20);
		if (random(0,2)==1) {
			Obst2.type = true;
		} else Obst2.type = false;
	}
	if (obstDistance < 50) {
		if (Obst2.x > Obst1.x) {
			Obst2.x = Obst2.x + 45;
		} else Obst1.x = Obst1.x + 45;
	}    
}

void CalcBirdXYpos(dGameStr& DinoGame, birdStr& Bird, const GameConstStr& GameConst) {
	int dir_y = random(0, 2);
	if (!Bird.colission) {
		Bird.x = Bird.x - random(2,6);
		if (dir_y == 0) {
			Bird.y = Bird.y + random(0,4);
		} else {
		  Bird.y = Bird.y - random  (0,4);
		}	
		if (Bird.y > 20) Bird.y = 18;
		if (Bird.y < 1) Bird.y = 2;
  } else {
		Bird.y += 2;
		Bird.x = Bird.x - 5;
	}
  if ((Bird.x < 0 - GameConst.Bird_width) || (Bird.y < 0)) {
    Bird.x = GameConst.new_obst_x_init + random(30,100);
    Bird.y = (GameConst.Bird_initY);
    Bird.colission = false;
  }
}

void CalcGameSpeed(dGameStr& DinoGame, const GameConstStr& GameConst) {
	DinoGame.speed = Score/3;
  if (DinoGame.speed > GameConst.maxspeed) DinoGame.speed = GameConst.maxspeed;
}

void ShowScore(dGameStr& DinoGame){
  if ((Score > DinoGame.sessionScore)) {
    DinoGame.sessionScore = Score;
  }
  myOLED.setCursor(1,4);
  myOLED.print("Your Score:   ");
  myOLED.print(Score);
  myOLED.setCursor(1,20);
  myOLED.print("Session best:  ");
  myOLED.print(DinoGame.sessionScore);
  myOLED.setCursor(1,36);
  myOLED.print("All time best: ");
  int highscore;
  EEPROM.get(Game_DinoRecord, highscore);
  myOLED.print(highscore);
  myOLED.setCursor(1,54);
  myOLED.println("Push button to Play!");
  myOLED.display();
  delay(300);
}

void DisplayGameSummary(dGameStr& DinoGame, jumpStr& Dino) {
  myOLED.clearDisplay();
  myOLED.setTextSize(2);
  myOLED.setCursor(10,20);
  myOLED.print("Game over ");
  myOLED.display();
  delay(DELAY500MS);
  myOLED.setTextSize(1);
  myOLED.clearDisplay();

  int highscore;
  EEPROM.get(Game_DinoRecord, highscore);
  Score = Score + Dino.food;
  if ((Score > highscore)) {
    EEPROM.put(Game_DinoRecord, Score);
    EEPROM.commit();
  }
  ShowScore(DinoGame);
}

void DisplayScore(dGameStr& DinoGame, jumpStr& Dino) {
  myOLED.setCursor(1,2);
  myOLED.print("food ");
  myOLED.print(Dino.food);
  myOLED.setCursor(61,2);
  myOLED.print("Jump: ");
  myOLED.print(Score);
}

void JumpSound(dGameStr& DinoGame){
  if (SoundEnabled && !DinoGame.soundDone) {
    tone(BUZZER_PIN,TON_ODLICZANIE_FREQ,DELAY100MS);
    delay(50);
    tone(BUZZER_PIN,TON_PUNKT_CZAS,DELAY100MS);
    delay(50);
    noTone(BUZZER_PIN);
    DinoGame.soundDone = true;
  }
}

void CheckButtons(dGameStr& DinoGame, jumpStr& Dino) {
  if(IsPressed(UpLeft)) {
    DinoGame.spriteSetIndex++;
    if (DinoGame.spriteSetIndex > 1){
      DinoGame.spriteSetIndex = 0;
    } 
    while(IsPressed(UpLeft));
  }
  if (IsPressed(DownLeft)) {
    if (Dino.jump == 0) Dino.jump = 1;
    Dino.jumpType = 1; // 1 - high jump
  }
  if (IsPressed(DownRight)) {
    if (Dino.jump == 0) Dino.jump = 1;
    Dino.jumpType = 2; // 2 - long jump
  }
  if (IsPressed(UpRight)) {
    DinoGame.pause = !DinoGame.pause;
    while (IsPressed(UpRight));
  }
}

} // namespace DinoGame

const GameInfo GameInfo_Dino = {
  "Dino",
  "Unikaj przeszkód, skacz dinozaurem!",
  DinoGame::Game_Dino,
  Game_DinoRecord
};
