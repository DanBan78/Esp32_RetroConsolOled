#include "global.h"
#include "gameCymbergaj/Cymbergaj.cpp"
#include "gameSlalom/Slalom.cpp"
#include "gameDino/Dino.cpp"
#include "gameHackMe/HackMe.cpp"
#include "gameSnoopy/Snoopy.cpp"
#include "gameStriker/Striker.cpp"
#include "gameSnake/Snake.cpp"


Adafruit_SH1106G myOLED = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool SoundEnabled = false;
int GameSelected = 1;
int Score = 0;
unsigned long lastButtonPressTime = 0;
bool sleepModeActive = false;

timerStruct Timer1Sec = {false, DELAY1000MS, 0, 0};

btPressedCode ReadButton(void (*callback)(timerStruct&), timerStruct& t) {
  btPressedCode ButtonCode = NONE;
  if (digitalRead(BTN_1) == LOW && digitalRead(BTN_0) == LOW && digitalRead(BTN_3) == LOW && digitalRead(BTN_4) == LOW) {
    ESP.restart();
    return ButtonCode;
  }
  if  (digitalRead(BTN_1) == LOW) ButtonCode = DownLeft;
  if  (digitalRead(BTN_0) == LOW) ButtonCode = UpLeft;
  if  (digitalRead(BTN_3) == LOW) ButtonCode = DownRight;
  if  (digitalRead(BTN_4) == LOW) ButtonCode = UpRight;

  if (callback) callback(t); //update timera
  delay(10); // debounce
  if (checkForSleep()) {
    enterSleepMode();
  }
  return ButtonCode;
}

bool IsPressed(btPressedCode button) {
  bool pressed = false;
  delay(10); // debounce
  if (digitalRead(BTN_1) == LOW && 
      digitalRead(BTN_0) == LOW && 
      digitalRead(BTN_3) == LOW && 
      digitalRead(BTN_4) == LOW) {
    ESP.restart();
  }
  switch(button) {
    case DownLeft:
      if (digitalRead(BTN_1) == LOW) pressed = true;
      break;
    case UpLeft:
      if (digitalRead(BTN_0) == LOW) pressed = true;
      break;
    case DownRight:
      if (digitalRead(BTN_3) == LOW) pressed = true;
      break;
    case UpRight:
      if (digitalRead(BTN_4) == LOW) pressed = true;
      break;
  }
  if (pressed) lastButtonPressTime = millis();
  return pressed;
}

void WaitForAnyButtonToContinue() {
  while (ReadButton(nullptr, Timer1Sec) == NONE) {
    if (checkForSleep()) {
      enterSleepMode();
    }
  }
  WaitForButtonRelease();
}

void WaitForButtonRelease() {
  while (ReadButton(nullptr, Timer1Sec) != NONE);
} 

void WelcomeScreen() {
  myOLED.setTextColor(SH110X_WHITE);
  myOLED.clearDisplay();
  myOLED.display();
  myOLED.setTextSize(1);
  myOLED.setCursor(10,20);
  myOLED.print("RetroConsolEsp");
  myOLED.display();
  delay(500);
}

int GameSelectMenu() {

  int GameSelected = 0;
  DisplayMenu(GameSelected, totalGamesNo);
  btPressedCode btn = NONE;
  unsigned long lastDisplayUpdate = millis();

  while(btn != DownRight) {
    btn = ReadButton(nullptr, Timer1Sec);
    
    if (checkForSleep()) {
      enterSleepMode();
    }
    
    if (btn != NONE) {
      if (btn == DownLeft && GameSelected < (totalGamesNo - 1)) {
        GameSelected++;
      }
      if (btn == UpLeft && GameSelected > 0) {
        GameSelected--;
      }
      if (btn == UpRight) {
        SoundEnabled = !SoundEnabled;
      }
      WaitForButtonRelease();
      DisplayMenu(GameSelected, totalGamesNo);
      // lastDisplayUpdate = millis();
    }
    delay(50);
  }
  return GameSelected;
}


void DisplayMenu(int MenuStartRow, int totalGamesNo){ 
  if (MenuStartRow > totalGamesNo) return;

  myOLED.clearDisplay();
  myOLED.setTextColor(SH110X_WHITE);
  myOLED.setTextSize(1);

  if (totalGamesNo==0) return;
  if (MenuStartRow < 0) MenuStartRow = 0;
  if (MenuStartRow > totalGamesNo) MenuStartRow = totalGamesNo;
    myOLED.setCursor(1,0);
    myOLED.print(">");

  for (int i=MenuStartRow; i<MenuStartRow+maxMenuRowsOnScreen && i< totalGamesNo; i++){
    myOLED.setCursor(10,(i-MenuStartRow)*12);
    myOLED.print(String(i+1)+". ");
    myOLED.print(allGames[i]->name);
  }
  DisplaySoundInfo(120, 0, SoundEnabled);
  myOLED.display();
}

bool CheckIfTimePassed(unsigned long& LastTimeCheckOneSec, unsigned long interval) {
  if (millis()-LastTimeCheckOneSec > interval) {
      LastTimeCheckOneSec  = millis();
      return true;
  }
  return false;
}

void MyTune(int freq, int duration_ms) {
  tone(BUZZER_PIN, freq, duration_ms);
  delay(duration_ms);
  noTone(BUZZER_PIN);
}
void DisplaySoundInfo(uint8_t x, uint8_t y, bool sound){
  myOLED.setTextSize(1);
  if (sound) {
  myOLED.drawBitmap(x, y, Glosnik, Glosnik_x_y, Glosnik_x_y, SH110X_WHITE);
  } else {
    myOLED.fillRect(x, y, Glosnik_x_y, Glosnik_x_y, SH110X_BLACK);
  }
}

void CheckIfResetHighscores(){
  if (IsPressed(UpLeft) && IsPressed(UpRight) && IsPressed(DownLeft)) {
    int reset = 0;
    EEPROM.put(Game_WolfRecord, reset);
    EEPROM.put(Game_SlalomRecord, reset);
    EEPROM.put(Game_DinoRecord, reset);
    EEPROM.put(Game_SnoopyRecord, reset);
    EEPROM.put(Game_CymbergajRecord, reset);
    EEPROM.put(Game_SpaceShooterRecord, reset);
    
    EEPROM.commit();
  }
 }

void DisplayHighscores(){
  myOLED.clearDisplay();
  myOLED.setTextSize(1);

  myOLED.setCursor(5,1);
  myOLED.print("Highscores");

  myOLED.setCursor(5,15);
  myOLED.print("DinoRecord: ");
  int highscore;
  EEPROM.get(Game_DinoRecord, highscore);
  myOLED.print(highscore);
  myOLED.setCursor(5,27);
  myOLED.print("Snoopy: ");
  EEPROM.get(Game_SnoopyRecord, highscore);
  myOLED.print(highscore);
  myOLED.setCursor(5,39);
  myOLED.print("Slalom: ");
  EEPROM.get(Game_SlalomRecord, highscore);
  myOLED.print(highscore);
  myOLED.display();
  delay(1000);
 }

const GameInfo HelpInfo = {
  "Pomoc/info",
  "opis obslugi konsoli",
  DisplayHelpInfo,
  -1  // Pomoc nie zapisuje highscore
};

void DisplayHelpInfo() {
  myOLED.clearDisplay();
  myOLED.setTextSize(1);
  for (int i=0; i<4; i++) {
    char buffer[64];
    const char* ptr = (const char*)pgm_read_ptr(&(teksty[i]));
    strcpy_P(buffer, ptr);
    if (i==0) {
      myOLED.setCursor(0, 1);
    } else myOLED.setCursor(0, 14+i*12);
    myOLED.print(buffer);
  }
  myOLED.display();
  WaitForAnyButtonToContinue();
  myOLED.clearDisplay();
  for (int i=4; i<9; i++) {
    char buffer[120];
    const char* ptr = (const char*)pgm_read_ptr(&(teksty[i]));
    strcpy_P(buffer, ptr);
    if (i==4) {
      myOLED.setCursor(0, 1);
    } else myOLED.setCursor(0, 14+(i-4)*12);
    myOLED.print(buffer);
  }
  myOLED.display();
  WaitForAnyButtonToContinue();
  myOLED.clearDisplay();
  for (int i=9; i<12; i++) {
    char buffer[120];
    const char* ptr = (const char*)pgm_read_ptr(&(teksty[i]));
    strcpy_P(buffer, ptr);
    if (i==9) {
      myOLED.setCursor(0, 1);
    } else myOLED.setCursor(0, 14+(i-9)*12);
    myOLED.print(buffer);
  }
  myOLED.display();
  WaitForAnyButtonToContinue();
  myOLED.clearDisplay();
  for (int i=12; i<17; i++) {
    char buffer[120];
    const char* ptr = (const char*)pgm_read_ptr(&(teksty[i]));
    strcpy_P(buffer, ptr);
    myOLED.setCursor(0, (i-12)*12);
    myOLED.print(buffer);
  }
  myOLED.display();
  WaitForAnyButtonToContinue();
  myOLED.clearDisplay();
  for (int i=17; i<21; i++) {
    char buffer[120];
    const char* ptr = (const char*)pgm_read_ptr(&(teksty[i]));
    strcpy_P(buffer, ptr);
    myOLED.setCursor(0, (i-17)*12);
    myOLED.print(buffer);
  }
  myOLED.display();
  delay(1000);
  WaitForAnyButtonToContinue();
 }

void SerialPrintFreeRam() {
  Serial.print("Wolna pamięć RAM: ");
  Serial.println(ESP.getFreeHeap());      // ilość wolnej pamięci heap (RAM)
  Serial.print("Największy możliwy blok do alokacji: ");
  Serial.println(ESP.getMaxAllocHeap());  // największy możliwy blok do alokacji
  Serial.println("----");
 }

void enterSleepMode() {
  sleepModeActive = true;  // Ustaw flagę sleep mode
  
  // Pokaż komunikat o sleep mode
  myOLED.clearDisplay();
  myOLED.setTextColor(SH110X_WHITE);
  myOLED.setTextSize(1);
  myOLED.setCursor(0, 20);
  myOLED.println("Entering Sleep Mode");
  myOLED.setCursor(0, 35);
  myOLED.println("Press BTN_3 to wake");
  myOLED.display();
  delay(2000);
  
  myOLED.clearDisplay();
  myOLED.display();
       
  // Konfiguracja wake-up na przycisk BTN_3 (GPIO 3)
  // Dla ESP32-C3 - uproszczona konfiguracja
  esp_sleep_enable_gpio_wakeup();
  gpio_wakeup_enable(GPIO_NUM_3, GPIO_INTR_LOW_LEVEL);
  
  // Przejście w deep sleep (restart po wybudzeniu)
  esp_deep_sleep_start();
}

void wakeFromSleep() {
  // Deep sleep powoduje restart - ta funkcja będzie wywołana w setup()
  
  // Reinicjalizacja wyświetlacza po sleep
  myOLED.clearDisplay();
  myOLED.setTextColor(SH110X_WHITE);
  myOLED.setTextSize(1);
  myOLED.setCursor(0, 20);
  myOLED.println("Wake from Sleep");
  myOLED.setCursor(0, 35);
  myOLED.println("System ready");
  myOLED.display();
  delay(2000); // Pokaż komunikat przez 2 sekundy
  
  // Reset wszystkich zmiennych stanu
  lastButtonPressTime = millis();
  sleepModeActive = false;
  Score = 0;
  
  // Poczekaj aż wszystkie przyciski zostaną puszczone
  while (digitalRead(BTN_0) == LOW || digitalRead(BTN_1) == LOW || 
         digitalRead(BTN_3) == LOW || digitalRead(BTN_4) == LOW) {
    delay(50);
  }
  
  // Dodatkowe opóźnienie na stabilizację
  delay(500);
}

bool checkForSleep() {
  if (millis() - lastButtonPressTime > SLEEP_TIMEOUT_MS) {
    return true;
  }
  return false;
}

bool isSleeping() {
  return sleepModeActive;
}