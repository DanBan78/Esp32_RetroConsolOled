# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

RetroConsolEsp32Oled is an Arduino sketch for an **ESP32-C3** handheld retro game
console with a **128x64 SH1106 OLED** (I2C), four buttons, a buzzer, and EEPROM-backed
high scores. UI text and many comments are in Polish.

## Build / flash / run

This is an Arduino IDE project (no Makefile or CLI build script in the repo). There is
no test suite — verification is done by flashing to hardware and playing.

- **Sketch entry point:** [RetroConsolEsp32Oled.ino](RetroConsolEsp32Oled.ino)
- **Board:** ESP32-C3 (select the matching ESP32-C3 board in Arduino IDE / `arduino-cli`)
- **Serial monitor baud:** 115200

Required Arduino libraries (see includes in [global/global.h](global/global.h)):

- `Adafruit_GFX`
- `Adafruit_SH110X` (SH1106G driver)
- ESP32 core libraries: `EEPROM`, `Wire`, `SPI`, `esp_sleep`, `driver/gpio`

Example flash with `arduino-cli` (adjust FQBN and port):

```bash
arduino-cli compile --fqbn esp32:esp32:esp32c3 RetroConsolEsp32Oled.ino
arduino-cli upload  --fqbn esp32:esp32:esp32c3 -p COM3 RetroConsolEsp32Oled.ino
```

## Hardware pin map

Defined in [global/global.h](global/global.h):

- Buttons (active-LOW, `INPUT_PULLUP`): `BTN_0`=GPIO0, `BTN_1`=GPIO1, `BTN_3`=GPIO3, `BTN_4`=GPIO4
- I2C OLED: SDA=GPIO8, SCL=GPIO9, address `0x3C`, clock 400 kHz
- Buzzer: GPIO2 via `ledc` channel 0

Button semantics use the `btPressedCode` enum (`UpLeft`, `DownLeft`, `UpRight`,
`DownRight`, `NONE`, `ALL_BTN`). In menus and games: left buttons navigate, `UpRight`
toggles sound, `DownRight` is enter/start. **Pressing all four buttons at once calls
`ESP.restart()`** (handled inside `ReadButton` / `IsPressed`).

## Architecture

### Non-standard include model (important)

This project does **not** rely on the Arduino multi-`.ino` build. Instead it
aggregates all translation units through a single chain of `#include` of `.cpp`
files so everything compiles as one unit:

- The `.ino` includes [global/global.h](global/global.h) **and** `global/global.cpp`.
- [global/global.cpp](global/global.cpp) in turn `#include`s every game's `.cpp`
  (Cymbergaj, Slalom, Dino, HackMe, Snoopy, Striker, Snake).

Consequences when editing:

- Adding a new game means adding `#include "gameX/X.cpp"` to
  [global/global.cpp](global/global.cpp), not relying on the IDE to pick it up.
- Do not add a game `.cpp` to the build independently — it is pulled in via this
  chain. Header guards (`#ifndef`) are what prevent double definitions.

### Game registration

Each game is registered through a `const GameInfo` struct (defined in
[global/global.h](global/global.h)):

```cpp
typedef struct GameInfo {
  const char* name;
  const char* description;
  GameLaunchFunc launch;   // void(*)() entry point
  int eepromAddress;       // EEPROM offset for highscore, -1 if none
};
```

To add a game to the menu, append its `&GameInfo_X` to the `allGames[]` array in
[RetroConsolEsp32Oled.ino](RetroConsolEsp32Oled.ino). `totalGamesNo` is derived from
the array size automatically. The main `loop()` shows `GameSelectMenu()` and calls
`allGames[selected]->launch()`.

> Note: `gameStriker/` is in the build and menu but is a **work-in-progress** — only
> its welcome screen is implemented (`Game_Striker` just loops `WelcomeStrikerScreen`).

### Per-game file convention

Each `gameX/` folder has three files following the same pattern (see
[gameDino/](gameDino/) as the reference implementation):

- `X.h` — public header: includes `../global/global.h`, declares the
  `namespace XGame { void Game_X(); }` entry point and `extern const GameInfo GameInfo_X;`
- `X_private.h` — game-internal structs, sprite `PROGMEM` bitmaps, constants
  (`GameConstStr`), and the full namespaced function declarations.
- `X.cpp` — implementation inside `namespace XGame`, ending with the
  `const GameInfo GameInfo_X = { ... }` definition at file scope.

Game logic is written as a blocking loop (`while(true)` driving frames with
`delay()`), polling buttons each frame rather than using interrupts.

### Shared state and EEPROM

Global mutable state lives in [global/global.cpp](global/global.cpp) — notably
`SoundEnabled` and `Score` (the shared current-game score, used by every game).
High scores are stored in EEPROM at fixed 4-byte offsets defined in
[global/global.h](global/global.h) (`Game_DinoRecord`=0, `Game_SlalomRecord`=8,
`Game_CymbergajRecord`=12, `Game_SnoopyRecord`=16, `Game_StrikerRecord`=24,
`Game_SnakeRecord`=28; `Game_WolfRecord`=4 and `Game_SpaceShooterRecord`=20 are
reserved but unused). `EEPROM.begin(64)` is called in `setup()`; writes must be
followed by `EEPROM.commit()`. Holding `UpLeft + UpRight + DownLeft` at boot resets
the high scores of the games that record one (`CheckIfResetHighscores`).

### Sleep mode

After `SLEEP_TIMEOUT_MS` (10 min) of no button press, `checkForSleep()` triggers
`enterSleepMode()`, which puts the ESP32-C3 into **deep sleep**. Deep sleep wake
causes a full restart, so `wakeFromSleep()` runs from `setup()` (gated on
`esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO`). Wake is configured on GPIO3
(`BTN_3`) low level. Any new game loop should call `ReadButton`/`IsPressed` (which
internally check for sleep) so the idle timeout keeps working.

### Sound

`MyTune(freq, ms)` and direct `tone()` / `noTone()` on `BUZZER_PIN` produce sound,
gated by the global `SoundEnabled` flag. Predefined tone freq/duration constants
(`TON_*`) live at the bottom of [global/global.h](global/global.h).
