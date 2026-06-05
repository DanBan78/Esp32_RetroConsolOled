// Include 
#include "global/global.h"
#include "global/global.cpp"

// Tablica wskaźników do struktur z informacjami o grach
const GameInfo* allGames[] = {
    &GameInfo_Statki,
    &GameInfo_Dino,
    &GameInfo_Snoopy,
    &GameInfo_Slalom,  
    &GameInfo_HackMe,
    &GameInfo_Cymbergaj,
    &GameInfo_Snake,
    &HelpInfo,
  };

const int totalGamesNo = sizeof(allGames)/sizeof(allGames[0]);

//####################################################################################
void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(A0));

  pinMode(BTN_0, INPUT_PULLUP);
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);
  pinMode(BTN_4, INPUT_PULLUP);

  Wire.begin(8, 9); // SDA, SCL dla ESP32-C3
  EEPROM.begin(64);
  ledcAttach(BUZZER_PIN, BUZZER_CHANNEL, LEDC_LOW_SPEED_MODE); // Buzzer
 
  Wire.setClock(400000); // 400 kHz
  delay(250); // wait for the OLED to power up
  myOLED.begin(SCREEN_ADDRESS, true); 
  myOLED.clearDisplay();

  WelcomeScreen();
  delay(300);
  CheckIfResetHighscores();
  delay(100);

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {
    wakeFromSleep();
  }  
  lastButtonPressTime = millis();
}
//####################################################################################
void loop() {
  Serial.println("Start");
  Score = 0;
  
  int selected = GameSelectMenu();
  if (selected >= 0 && selected < (sizeof(allGames)/sizeof(allGames[0]))) {
    // wywolanie funkcji wybranej gry
    allGames[selected]->launch();
  }
}