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

/*
EEPROM storage conventions:

- 0-42: highscore data;
*/

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
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;

const byte lcdContrastPin = 3;

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

const char EEPROM_DATA_SEPARATOR = ';';

// The format of a row: `1. ABC: 000`. `1. ` takes up 3 bytes.
// There are 13 bytes remaning + 1 separator for each -> 3 * 14 = 42 necessary bytes.
const int HIGHSCORE_RECORDS = 3;
const int BYTES_PER_HIGHSCORE_RECORD = 14;
const int HIGHSCORE_BYTES_COUNT = HIGHSCORE_RECORDS * BYTES_PER_HIGHSCORE_RECORD;
/* ============================================= */

char highscoreList[3][16]; 

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

/* ============================================= */

void setup() {
  Serial.begin(9600);

  pinMode(JOY_SW_PIN, INPUT_PULLUP);

  lc.shutdown(0, false);
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);

  pinMode(lcdContrastPin, OUTPUT);
  lcd.begin(16, 2);
  // lcdContrast = EEPROM.read(0); // 80
  analogWrite(lcdContrastPin, 80);

  lcd.createChar(0, arrorwDownGlyph);
  lcd.createChar(1, arrowUpGlyph);
  lcd.createChar(2, snakeGlyph);

  // char hs[48] = "abc.";
  // EEPROM.put(0, hs);
  // char var = '*';
  // EEPROM.put(1, var);

  // char res[48];
  // EEPROM.get(0, res);
  // Serial.println(res);
  // char hs[42] = "aaa:111;bbb:222;ccc:333;";
  // char hs[42] = "aaa:111;BBB:999;ccc:333;";
  // EEPROM.put(0, hs);

  readHighscoreData(0);

  // Serial.println(highscoreList[0]);
  // Serial.println(highscoreList[1]);
  // Serial.println(highscoreList[2]);

  // char hs[14] = "BbB:888";
  // strcpy(highscoreList[1], hs);

  // writeHighscoreData(0);
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
  }
}

void showMenu (const char* menuItems[], int menuItemsLength) {
  menuItemIdx = *menuItemIdxPtr;
  menuSelectedItemIdx = *menuSelectedItemIdxPtr;

  Serial.println(menuItems[menuItemIdx]);
  Serial.println(menuItems[menuItemIdx + 1]);

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

  int joySwitchValue = !digitalRead(JOY_SW_PIN);
  if (menuCrtSwitchValue != joySwitchValue && joySwitchValue) {
    Serial.println("Clicked!!!!!");
  }

  menuCrtSwitchValue = joySwitchValue;

  Directions nextDirection = getDirectionFromJoystick();
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
    Serial.println("Clicked!");
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
  if (!strlen(highscoreList[0])) {
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
    highscoreList[0],
    highscoreList[1],
    highscoreList[2],
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
          Serial.println("SETTINGS");
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

    default: {
      return;
    }
  }
}

int readHighscoreData (int offset) {  
  char highscoreData[HIGHSCORE_BYTES_COUNT];
  EEPROM.get(offset, highscoreData);

  int highscoreRecordCount = 0;
  int highscoreRecordStartIdx = 0;

  for (int i = 0; i < HIGHSCORE_BYTES_COUNT; i++) {
    char ch = highscoreData[i];
    if (ch == EEPROM_DATA_SEPARATOR) {
      char* slicedStr = slice(highscoreData, highscoreRecordStartIdx, i);
      strcpy(highscoreList[highscoreRecordCount++], slicedStr);

      highscoreRecordStartIdx = i + 1;
      free(slicedStr);
    }
  }

  return offset + HIGHSCORE_BYTES_COUNT;
}

void writeHighscoreData (int startPos) {
  char newHighscoreData[HIGHSCORE_BYTES_COUNT];

  strcpy(newHighscoreData, "");

  for (int i = 0; i < HIGHSCORE_RECORDS; i++) {
    char* hsRecord = malloc(strlen(highscoreList[i]) + 1);
    strcpy(hsRecord, highscoreList[i]);

    strcpy(newHighscoreData + strlen(newHighscoreData), hsRecord);
    strcpy(newHighscoreData + strlen(newHighscoreData), ";");
  }

  EEPROM.put(0, newHighscoreData);
}

char* slice (char* str, int startIdx, int endIdx) {
  int len = endIdx - startIdx;
  char *res = malloc(len);

  for (int i = startIdx; i < endIdx; i++) {
    res[i - startIdx] = str[i];
  }

  res[len] = '\0';

  return res;
}