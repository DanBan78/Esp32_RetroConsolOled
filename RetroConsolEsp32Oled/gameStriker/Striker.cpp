#include "Striker.h"
#include "Striker_private.h"

namespace StrikerGame {

void Game_Striker() {
  while (true) {
    WelcomeStrikerScreen();
  }
}

void WelcomeStrikerScreen() {
  myOLED.clearDisplay();
  myOLED.setTextColor(SH110X_WHITE);
  myOLED.setTextSize(2);
  myOLED.setCursor(2, 1);
  myOLED.println("Striker");
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
  myOLED.clearDisplay();
  myOLED.drawBitmap(0, 0, bitmap_LEWY, 128, 64, SH110X_WHITE);
  myOLED.display();
  delay(DELAY1000MS);
  myOLED.clearDisplay();
  myOLED.drawBitmap(0, 0, bitmap_SZCZENA, 128, 64, SH110X_WHITE);
  myOLED.display();
  delay(DELAY1000MS);
}

} // namespace StrikerGame

const GameInfo GameInfo_Striker = {
  "Striker",
  "Lewy vs Szczena",
  StrikerGame::Game_Striker,
  Game_StrikerRecord
};