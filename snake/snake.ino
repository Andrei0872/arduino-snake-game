#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

const int JOY_X_PIN = A0;
const int JOY_Y_PIN = A1;
const int JOY_SW_PIN = 13;
const int JOY_LEFT_TRESHOLD = 400;
const int JOY_RIGHT_TRESHOLD = 600;
const int JOY_TOP_TRESHOLD = 400;
const int JOY_BOTTOM_TRESHOLD = 600;

const int HIGHSCORE_START_OFFSET = 0;
struct Highscore {
  // 13 = `X. ` occupies 3 bytes and the rest is destined for the actual content.
  
  char first[13];
  char second[13];
  char third[13];
} highscoreData;

const int SETTINGS_START_OFFSET = HIGHSCORE_START_OFFSET + sizeof(Highscore);
const int DEFAULT_LCD_CONTRAST_VALUE = 120;
const int DEFAULT_LCD_BRIGHTNESS_VALUE = 110;
const int DEFAULT_MATRIX_BRIGHTNESS_VALUE = 5;
struct Settings {
  byte difficultyLevel;
  byte LCDContrast = DEFAULT_LCD_CONTRAST_VALUE;
  byte LCDBrightness = DEFAULT_LCD_BRIGHTNESS_VALUE;
  byte matrixBrightness = DEFAULT_MATRIX_BRIGHTNESS_VALUE;
  bool hasSoundsOn;
} settingsData;

enum Directions {
  LEFT = 0,
  RIGHT = 1,
  UP = 2,
  DOWN = 3,
};

const int DIN_PIN = 12;
const int CLOCK_PIN = 11;
const int LOAD_PIN = 10;
const int MATRIX_SIZE = 8;

LedControl lc = LedControl(DIN_PIN, CLOCK_PIN, LOAD_PIN, 1);
byte matrixBrightness = 2;

struct Position {
  int row, col;
};

const int FOOD_BLINK_INTERVAL = 250;

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 2;
const byte d6 = 5;
const byte d7 = 4;

const byte lcdContrastPin = 3;

const short LCD_BRIGHTNESS_PIN = 6;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte lcdContrast;

enum ProgramState {
  Greeting,
  Playing,
  Menu,
  SettingsMenu,
  HighscoreMenu,
  About,
  HowToPlay,
  SettingLCDContrast,
  SettingLCDBrightness,
  SettingMatrixrightness,
};

const int GREETING_MESSAGE_TIME = 2500;

const int MENU_ITEMS_LENGTH = 5;
const int MENU_SETTINGS_INDEX = 2;
const char* menuItems[] = {
  "1. Play",
  "2. Highscore",
  "3. Settings",
  "4. About",
  "5. How to play"
};

const int SETTINGS_MENU_ITEMS_LENGTH = 5;
const char* settingsMenuItems[] = {
  "1. Diff. Level",
  "2. LCD contrast",
  "3. LCD BRT",
  "4. Matrix BRT",
  "5. Sound",
};

const int ABOUT_ITEMS_LENGTH = 2;
const char* aboutItems[] = {
  "Andrei Gatej",
  "andreigatej.dev"
};

const byte arrorwDownGlyph[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100
};

const byte arrowUpGlyph[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};

const byte snakeGlyph[8] = {
  0b00000,
  0b00000,
  0b00010,
  0b01110,
  0b01000,
  0b01000,
  0b11000,
  0b00000
};

const byte barGlyph[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

const int HIGHSCORE_RECORDS = 3;

const int LCD_CONTRAST_RANGE_STEP = 255 / 16;
const int LCD_BRIGHTNESS_RANGE_STEP = 255 / 16;
const int MATRIX_BRIGHTNESS_RANGE_STEP = 1;

/* ============================================= */

bool isJoystickNeutral = true;

// { 0, 0 } = bottom left corner.
Position crtPos = Position { 0, 1 };

Directions crtDirection = -1;

Position foodPos; 
bool isFoodDotActive = true;
unsigned long foodBlinkTimestamp = millis();

ProgramState crtProgramState = Greeting;

unsigned long greetingMessageTimestamp = millis();

int menuItemIdx = 0;
int menuSelectedItemIdx = 0;
int menuCrtSwitchValue;
int* menuItemIdxPtr;
int* menuSelectedItemIdxPtr;

// Main menu.
int mainMenuItemIdx = 0;
int mainMenuSelectedItemIdx = 0;

// Settings menu.
int settingsMenuItemIdx = 0;
int settingsMenuSelectedItemIdx = 0;

// About items.
int aboutItemIdx = 0;
int aboutSelectedItemIdx = 0;

// Highscore records.
int highscoreItemIdx = 0;
int highscoreSelectedItemIdx = 0;

int rangeValue = -1;
bool shouldRenderRangeSettingPixels = true;

/* ============================================= */

void setup() {
  Serial.begin(9600);

  pinMode(JOY_SW_PIN, INPUT_PULLUP);

  lc.shutdown(0, false);
  // lc.setIntensity(0, 2);
  lc.clearDisplay(0);

  pinMode(lcdContrastPin, OUTPUT);
  pinMode(LCD_BRIGHTNESS_PIN, OUTPUT);
  lcd.begin(16, 2);
  // lcdContrast = EEPROM.read(0); // 80
  // analogWrite(LCD_BRIGHTNESS_PIN, 110);
  // lcd.print("foobar!");
  // for (int i = 0; i < 255; i += 2) {
  //   analogWrite(lcdContrastPin, i);
  //   lcd.setCursor(9, 0);
  //   lcd.print(i);
  //   delay(80);
  // }

  // analogWrite(d7, 60);

  lcd.createChar(0, arrorwDownGlyph);
  lcd.createChar(1, arrowUpGlyph);
  lcd.createChar(2, snakeGlyph);
  lcd.createChar(3, barGlyph);

  readDataFromStorage(HIGHSCORE_START_OFFSET, highscoreData);
  readDataFromStorage(SETTINGS_START_OFFSET, settingsData);

  // Serial.println(settingsData.LCDBrightness);
  settingsData.LCDContrast = !!settingsData.LCDContrast ? settingsData.LCDContrast : DEFAULT_LCD_CONTRAST_VALUE;
  settingsData.LCDBrightness = !!settingsData.LCDBrightness ? settingsData.LCDBrightness : DEFAULT_LCD_BRIGHTNESS_VALUE;
  settingsData.matrixBrightness = !!settingsData.matrixBrightness ? settingsData.matrixBrightness : DEFAULT_MATRIX_BRIGHTNESS_VALUE;

  // Serial.println(settingsData.LCDContrast);
  // Serial.println(settingsData.LCDBrightness);

  analogWrite(lcdContrastPin, settingsData.LCDContrast);
  analogWrite(LCD_BRIGHTNESS_PIN, settingsData.LCDBrightness);
  lc.setIntensity(0, settingsData.matrixBrightness);

  // Serial.println(highscoreData.first);
  // Serial.println(highscoreData.second);
  // Serial.println(highscoreData.third);
  // Serial.println(nextOffset);

  // strcpy(highscoreData.second, "bbb:7777");

  // writeDataToStorage(HIGHSCORE_START_OFFSET, highscoreData);

  // toggleAllMatrixPoints(true);
}

void loop() {
  switch (crtProgramState) {
    case Greeting: {
      showGreetingMessage();
      break;
    }
    case Playing: {
      playGame();
      break;
    }
    case Menu: {
      menuItemIdxPtr = &mainMenuItemIdx;
      menuSelectedItemIdxPtr = &mainMenuSelectedItemIdx;
      
      showMenu(menuItems, MENU_ITEMS_LENGTH);
      break;
    }
    case SettingsMenu: {
      menuItemIdxPtr = &settingsMenuItemIdx;
      menuSelectedItemIdxPtr = &settingsMenuSelectedItemIdx;
      
      showMenu(settingsMenuItems, SETTINGS_MENU_ITEMS_LENGTH);
      
      break;
    }
    case HighscoreMenu: {
      showHighscoreMenu();
      break;
    }
    case About: {
      showAboutSection();

      break;
    }
    case HowToPlay: {
      showHowToPlaySection();
      break;
    }
    case SettingLCDContrast: {
      rangeValue = rangeValue != -1 ? rangeValue : settingsData.LCDContrast;
      showLCDContrastSettingView();
      break;
    }
    case SettingLCDBrightness: {
      rangeValue = rangeValue != -1 ? rangeValue : settingsData.LCDBrightness;
      showLCDBrightnessSettingView();
      break;
    }
    case SettingMatrixrightness: {
      rangeValue = rangeValue != -1 ? rangeValue : settingsData.matrixBrightness;
      // TODO: might not need to do this on every iteration.
      toggleAllMatrixPoints(true);
      showMatrixBrightnessSettingView();
      break;
    }
  }
}

void showMenu (const char* menuItems[], int menuItemsLength) {
  menuItemIdx = *menuItemIdxPtr;
  menuSelectedItemIdx = *menuSelectedItemIdxPtr;

  lcd.setCursor(0, 0);
  lcd.print(menuItems[menuItemIdx]);

  if (menuItemIdx > 0) {
    lcd.setCursor(15, 0);
    lcd.write((byte)1);
  }

  if (menuItemIdx + 1 != menuItemsLength) {
    lcd.setCursor(0, 1);
    lcd.print(menuItems[menuItemIdx + 1]);
  }

  if (menuItemIdx < menuItemsLength - 2) {
    lcd.setCursor(15, 1);
    lcd.write((byte)0);
  }

  int selectedLCDLine = menuSelectedItemIdx % 2;
  lcd.setCursor(strlen(menuItems[menuSelectedItemIdx]), selectedLCDLine);
  lcd.write((byte)2);

  // return;

  // int joySwitchValue = !digitalRead(JOY_SW_PIN);
  // if (menuCrtSwitchValue != joySwitchValue && joySwitchValue) {
  //   // Serial.println("Clicked!!!!!");
  // }

  // menuCrtSwitchValue = joySwitchValue;

  Directions nextDirection = getDirectionFromJoystick();
  // Serial.println(nextDirection);
  if (nextDirection != -1 && nextDirection == DOWN) {
    handleItemEnter(menuSelectedItemIdx);
    lcd.clear();
    return;
  }

  if (nextDirection != -1 && nextDirection == UP) {
    handleItemExit(menuSelectedItemIdx);
    lcd.clear();
    return;
  }
  
  if (nextDirection == - 1 || (nextDirection != LEFT && nextDirection != RIGHT)) {
    return;
  }

  menuSelectedItemIdx = nextDirection == RIGHT ? menuSelectedItemIdx + 1 : menuSelectedItemIdx - 1;
  menuSelectedItemIdx = constrain(menuSelectedItemIdx, 0, menuItemsLength - 1);

  if (nextDirection == RIGHT && menuSelectedItemIdx % 2 == 0) {
    menuItemIdx = menuSelectedItemIdx;
  } else if (nextDirection == LEFT && menuSelectedItemIdx % 2 == 1) {
    menuItemIdx = menuSelectedItemIdx - 1;
  }

  menuItemIdx = constrain(menuItemIdx, 0, menuItemsLength - 1);
  lcd.clear();

  *menuItemIdxPtr = menuItemIdx;
  *menuSelectedItemIdxPtr = menuSelectedItemIdx;
}

void showGreetingMessage () {
  lcd.setCursor(0, 0);
  lcd.print("Greeting message");

  if (millis() - greetingMessageTimestamp > GREETING_MESSAGE_TIME) {
    lcd.clear();
    
    // activatePointOnMatrix(crtPos);
    // computeRandomFoodPosition();
    crtProgramState = Menu;
    isJoystickNeutral = true;
  }
}

void playGame () {
  blinkFood();

  int joySwitchValue = !digitalRead(JOY_SW_PIN);
  if (joySwitchValue) {
    // Serial.println("Clicked!");
  }

  Directions nextDirection = getDirectionFromJoystick();
  if (crtDirection == nextDirection) {
    return;
  }

  deactivatePointOnMatrix(crtPos);

  crtDirection = nextDirection;
  
  if (nextDirection != -1) {
    computeNextPosition(nextDirection);
  }

  activatePointOnMatrix(crtPos);

  if (arePositionsEqual(crtPos, foodPos)) {
    computeRandomFoodPosition();
  }
}

void showHighscoreMenu () {
  if (!strlen(highscoreData.first)) {
    lcd.setCursor(2, 0);
    lcd.print("Nothing here");

    lcd.setCursor(5, 1);
    lcd.print("yet :(");

    Directions nextDirection = getDirectionFromJoystick();
    if (nextDirection != -1 && nextDirection == UP) {
      handleItemExit(menuSelectedItemIdx);
      lcd.clear();
      return;
    }
    
    return;
  }

  menuItemIdxPtr = &highscoreItemIdx;
  menuSelectedItemIdxPtr = &highscoreSelectedItemIdx;

  const char* highscoreListItems[] = {
    highscoreData.first,
    highscoreData.second,
    highscoreData.third,
  };
    
  showMenu(highscoreListItems, HIGHSCORE_RECORDS);
}

void showAboutSection () {
  lcd.setCursor(2, 0);
  lcd.print(aboutItems[0]);

  lcd.setCursor(0, 1);
  lcd.print(aboutItems[1]);

  Directions nextDirection = getDirectionFromJoystick();
  if (nextDirection != -1 && nextDirection == UP) {
    handleItemExit(menuSelectedItemIdx);
    lcd.clear();
    return;
  }
}

void showHowToPlaySection () {
  lcd.setCursor(3, 0);
  lcd.print("It's snake");

  Directions nextDirection = getDirectionFromJoystick();
  if (nextDirection != -1 && nextDirection == UP) {
    handleItemExit(menuSelectedItemIdx);
    lcd.clear();
    return;
  }
}

int getDirectionFromJoystick () {
  // The logic may seem odd because the joystick has been positioned
  // in a way that makes sense for the user.
  int joyYValue = analogRead(JOY_Y_PIN);
  int joyXValue = analogRead(JOY_X_PIN);

  if (joyXValue < JOY_LEFT_TRESHOLD && isJoystickNeutral) {
    isJoystickNeutral = false;
    return RIGHT;
  }

  if (joyXValue > JOY_RIGHT_TRESHOLD && isJoystickNeutral) {
    isJoystickNeutral = false;
    return LEFT;
  }

  if (joyYValue < JOY_TOP_TRESHOLD && isJoystickNeutral) {
    isJoystickNeutral = false;
    return DOWN;
  }

  if (joyYValue > JOY_BOTTOM_TRESHOLD && isJoystickNeutral) {
    isJoystickNeutral = false;
    return UP;
  }

  bool isNeutralHorizontally = JOY_LEFT_TRESHOLD <= joyXValue && joyXValue <= JOY_RIGHT_TRESHOLD;
  bool isNeutralVeritcally = JOY_TOP_TRESHOLD <= joyYValue && joyYValue <= JOY_BOTTOM_TRESHOLD;  
  if (isNeutralHorizontally && isNeutralVeritcally) {
    isJoystickNeutral = true;
    return -1;
  }

  return -1;
}

void activatePointOnMatrix(Position& crtPos) {
  lc.setLed(0, crtPos.col, crtPos.row, true);
}

void deactivatePointOnMatrix(Position& crtPos) {
  lc.setLed(0, crtPos.col, crtPos.row, false);
}

void computeNextPosition (Directions& direction) {
  switch (direction) {
    case DOWN: {
      crtPos.row = crtPos.row - 1 < 0 ? MATRIX_SIZE - 1 : crtPos.row - 1; 
      break;
    }
    case UP: {
      crtPos.row = crtPos.row + 1 == MATRIX_SIZE ? 0 : crtPos.row + 1;
      break;
    }
    case LEFT: {
      crtPos.col = crtPos.col - 1 < 0 ? MATRIX_SIZE - 1 : crtPos.col - 1;
      break;
    }
    case RIGHT: {
      crtPos.col = crtPos.col + 1 == MATRIX_SIZE ? 0 : crtPos.col + 1;
      break;
    }
    default: {
      return crtPos;
    }
  }
}

void computeRandomFoodPosition () {
  foodPos.row = random(MATRIX_SIZE);
  foodPos.col = random(MATRIX_SIZE);
}

void blinkFood () {
  unsigned long timePassed = millis() - foodBlinkTimestamp;
  if (timePassed > FOOD_BLINK_INTERVAL) {
    isFoodDotActive = !isFoodDotActive;
    foodBlinkTimestamp = millis();
  }

  if (isFoodDotActive) {
    activatePointOnMatrix(foodPos);
  } else {
    deactivatePointOnMatrix(foodPos);
  }
}

bool arePositionsEqual (Position& pos1, Position& pos2) {
  return pos1.row == pos2.row && pos1.col == pos2.col;
}

void handleItemEnter (int itemIdx) {
  switch (crtProgramState) {
    case Menu: {
      switch (itemIdx) {
        case 2: {
          // Settings.
          crtProgramState = SettingsMenu;

          break;
        }
        case 0: {
          // Play.
          activatePointOnMatrix(crtPos);
          computeRandomFoodPosition();
          crtProgramState = Playing;

          break;
        }
        case 1: {
          // HighScore.
          crtProgramState = HighscoreMenu;

          break;
        }
        case 3: {
          // About.
          crtProgramState = About;

          break;
        }
        case 4: {
          // How to play.
          crtProgramState = HowToPlay;
        }
      }
      break;
    }
    
    case SettingsMenu: {
      switch (itemIdx) {
        case 1: {
          // LCD Contrast.
          crtProgramState = SettingLCDContrast;

          break;
        }
        case 2: {
          // LCD Brightness.
          crtProgramState = SettingLCDBrightness;

          break;
        }
        case 3: {
          // Matrix Brightness.
          crtProgramState = SettingMatrixrightness;

          break;
        }
      }
      
      break;
    }

    default: {
      return;
    }
  }
}

void handleItemExit (int itemIdx) {
  switch (crtProgramState) {
    case SettingsMenu: {
      crtProgramState = Menu;
      break;
    }
    case HighscoreMenu: {
      crtProgramState = Menu;
      break;
    }
    case About: {
      crtProgramState = Menu;
      break;
    }
    case HowToPlay: {
      crtProgramState = Menu;
      break;
    }
    case SettingLCDContrast:
    case SettingLCDBrightness:
    case SettingMatrixrightness: {
      crtProgramState = SettingsMenu;
      break;
    }

    default: {
      return;
    }
  }
}

template<typename T> int readDataFromStorage (int offset, T& data) {  
  EEPROM.get(offset, data);

  return offset + sizeof(data);
}
template<typename T> int writeDataToStorage (int offset, T& data) {
  EEPROM.put(offset, data);

  return offset + sizeof(data);
}

void displayRange (int filledBars) {
  for (int i = 0; i < filledBars; i++) {
    lcd.setCursor(i, 1);
    lcd.write((byte)3);
  }
}

void showLCDContrastSettingView () {
  if (shouldRenderRangeSettingPixels) {
    lcd.clear();
    lcd.print("LCD Contrast");

    int filledBars = map(rangeValue, 0, 255, 0, 16);
    analogWrite(lcdContrastPin, rangeValue);

    displayRange(filledBars);
    shouldRenderRangeSettingPixels = false;
  }

  Directions nextDirection = getDirectionFromJoystick();
  if (nextDirection != -1 && nextDirection == UP) {
    handleItemExit(menuSelectedItemIdx);
    lcd.clear();

    settingsData.LCDContrast = rangeValue;
    writeDataToStorage(SETTINGS_START_OFFSET, settingsData);

    rangeValue = -1;

    shouldRenderRangeSettingPixels = true;
    return;
  }

  if (nextDirection != -1 && (nextDirection == RIGHT || nextDirection == LEFT)) {
    rangeValue += nextDirection == RIGHT ? LCD_CONTRAST_RANGE_STEP : -LCD_CONTRAST_RANGE_STEP;
    rangeValue = constrain(rangeValue, 0, 255);

    shouldRenderRangeSettingPixels = true;
    return;    
  }
}

void showLCDBrightnessSettingView () {
  if (shouldRenderRangeSettingPixels) {
    lcd.clear();
    lcd.print("LCD Brightness");

    int filledBars = map(rangeValue, 0, 255, 0, 16);
    analogWrite(LCD_BRIGHTNESS_PIN, rangeValue);

    displayRange(filledBars);
    shouldRenderRangeSettingPixels = false;
  }

  Directions nextDirection = getDirectionFromJoystick();
  if (nextDirection != -1 && nextDirection == UP) {
    handleItemExit(menuSelectedItemIdx);
    lcd.clear();

    settingsData.LCDBrightness = rangeValue;
    writeDataToStorage(SETTINGS_START_OFFSET, settingsData);

    rangeValue = -1;

    shouldRenderRangeSettingPixels = true;
    return;
  }

  if (nextDirection != -1 && (nextDirection == RIGHT || nextDirection == LEFT)) {
    rangeValue += nextDirection == RIGHT ? LCD_BRIGHTNESS_RANGE_STEP : -LCD_BRIGHTNESS_RANGE_STEP;
    rangeValue = constrain(rangeValue, 0, 255);

    shouldRenderRangeSettingPixels = true;
    return;    
  }
}

void showMatrixBrightnessSettingView () {
  if (shouldRenderRangeSettingPixels) {
    lcd.clear();
    lcd.print("Matrix Brightness");

    int filledBars = rangeValue;
    lc.setIntensity(0, rangeValue);

    displayRange(filledBars);
    shouldRenderRangeSettingPixels = false;
  }

  Directions nextDirection = getDirectionFromJoystick();
  if (nextDirection != -1 && nextDirection == UP) {
    handleItemExit(menuSelectedItemIdx);
    lcd.clear();

    settingsData.matrixBrightness = rangeValue;
    writeDataToStorage(SETTINGS_START_OFFSET, settingsData);

    rangeValue = -1;

    toggleAllMatrixPoints(false);

    shouldRenderRangeSettingPixels = true;
    return;
  }

  if (nextDirection != -1 && (nextDirection == RIGHT || nextDirection == LEFT)) {
    rangeValue += nextDirection == RIGHT ? MATRIX_BRIGHTNESS_RANGE_STEP : -MATRIX_BRIGHTNESS_RANGE_STEP;
    rangeValue = constrain(rangeValue, 0, 16);

    shouldRenderRangeSettingPixels = true;
    return;    
  }
}

void toggleAllMatrixPoints (bool shouldActivate) {
  for (int i = 0; i < MATRIX_SIZE; i++) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
      lc.setLed(0, j, i, shouldActivate);
    }
  }
}