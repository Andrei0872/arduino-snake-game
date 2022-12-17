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
  
  char records[5][13];
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
  int crtDirection;
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
  GameOverScreen1,
  GameOverScreen2,
  ResetHighscore,
  ResetHighscoreSuccessful,
  DifficultyLevel,
};

const int GREETING_MESSAGE_TIME = 2500;
const int GAME_OVER_SCREEN1_TIME = 2500;
const int HS_RESET_MESSAGE_TIME = 2500;

const int MENU_ITEMS_LENGTH = 5;
const int MENU_SETTINGS_INDEX = 2;
const char* menuItems[] = {
  "1. Play",
  "2. Highscore",
  "3. Settings",
  "4. About",
  "5. How to play"
};

const int SETTINGS_MENU_ITEMS_LENGTH = 6;
const char* settingsMenuItems[] = {
  "1. Diff. Level",
  "2. LCD contrast",
  "3. LCD BRT",
  "4. Matrix BRT",
  "5. Sound",
  "6. Reset HS",
};

const int ABOUT_ITEMS_LENGTH = 2;
const char* aboutItems[] = {
  "Andrei Gatej",
  "andreigatej.dev"
};

const int DIFFICULTY_LEVELS_LENGTH = 3;
const char* difficultyLevels[] = {
  "1. Easy",
  "2. Medium",
  "3. Hard",
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

const int HIGHSCORE_RECORDS = 5;

const int LCD_CONTRAST_RANGE_STEP = 255 / 16;
const int LCD_BRIGHTNESS_RANGE_STEP = 255 / 16;
const int MATRIX_BRIGHTNESS_RANGE_STEP = 1;

const int difficultyLevelsValues[3] = { 600, 350, 200 };

/* ============================================= */

bool isJoystickNeutral = true;
int updateSnakeDotsInterval = 500;

// { 0, 0 } = bottom left corner.
Position crtPos = Position { 0, 1 };

Directions crtDirection = -1;

Position foodPos; 
bool isFoodDotActive = true;
unsigned long foodBlinkTimestamp = millis();

ProgramState crtProgramState = Greeting;
// ProgramState crtProgramState = ResetHighscoreSuccessful;
// ProgramState crtProgramState = Playing;
// ProgramState crtProgramState = GameOverScreen1;
// ProgramState crtProgramState = GameOverScreen2;

unsigned long greetingMessageTimestamp = millis();
unsigned long gameOverScreen1Timestamp = millis();
unsigned long HSResetSuccessfulMessageTimestamp = millis();

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

int crtScore = 0;
bool hasDisplayedInitialScore = false;

unsigned long updatedSnakeTimestamp = millis();

Position* snakeDots[MATRIX_SIZE * MATRIX_SIZE];
int snakeDotsCount = 0;
Directions turningPoints[MATRIX_SIZE][MATRIX_SIZE];

char username[4] = "AAA";
int selectedUsernameCharIdx = 0;

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

  // writeDummyData();

  readDataFromStorage(HIGHSCORE_START_OFFSET, highscoreData);
  readDataFromStorage(SETTINGS_START_OFFSET, settingsData);

  printHighscoreRecords();

  // Serial.println(settingsData.LCDBrightness);
  settingsData.LCDContrast = !!settingsData.LCDContrast ? settingsData.LCDContrast : DEFAULT_LCD_CONTRAST_VALUE;
  settingsData.LCDBrightness = !!settingsData.LCDBrightness ? settingsData.LCDBrightness : DEFAULT_LCD_BRIGHTNESS_VALUE;
  settingsData.matrixBrightness = !!settingsData.matrixBrightness ? settingsData.matrixBrightness : DEFAULT_MATRIX_BRIGHTNESS_VALUE;

  Serial.println(settingsData.LCDContrast);
  // Serial.println(settingsData.LCDBrightness);

  // settingsData.LCDContrast = 80;
  analogWrite(lcdContrastPin, settingsData.LCDContrast);
  analogWrite(LCD_BRIGHTNESS_PIN, settingsData.LCDBrightness);
  lc.setIntensity(0, settingsData.matrixBrightness);
  updateSnakeDotsInterval = difficultyLevelsValues[settingsData.difficultyLevel];

  Serial.println(updateSnakeDotsInterval);

  // Serial.println(highscoreData.first);
  // Serial.println(highscoreData.second);
  // Serial.println(highscoreData.third);
  // Serial.println(nextOffset);

  // strcpy(highscoreData.first, "");
  // strcpy(highscoreData.second, "");
  // strcpy(highscoreData.third, ""); 
  // writeDataToStorage(HIGHSCORE_START_OFFSET, highscoreData);

  // toggleAllMatrixPoints(true);

  crtPos.crtDirection = -1;
  // Position head = crtPos;
  snakeDots[snakeDotsCount++] = &crtPos;

  memset(turningPoints, -1, sizeof(turningPoints));
  // Serial.println(getHighestSurpassedOnPodium(8));
  // Serial.println(strlen(username));
  // Serial.println(sizeof(username));
  // Serial.println(username);

  // saveUsernameAndScore();
  randomSeed(analogRead(0));

}

void writeDummyData () {
  strcpy(highscoreData.records[0], "aaa:30");
  strcpy(highscoreData.records[1], "bbb:25");
  strcpy(highscoreData.records[2], "ccc:20"); 
  strcpy(highscoreData.records[3], "ddd:15"); 
  strcpy(highscoreData.records[4], "eee:10"); 
  writeDataToStorage(HIGHSCORE_START_OFFSET, highscoreData);
  
  settingsData.difficultyLevel = 1;
  settingsData.hasSoundsOn = true;
  settingsData.LCDBrightness = 100;
  settingsData.matrixBrightness = 10;
  settingsData.LCDContrast = 120;
  writeDataToStorage(SETTINGS_START_OFFSET, settingsData);
}

void printHighscoreRecords () {
  for (int i = 0; i < HIGHSCORE_RECORDS; i++) {
    Serial.print("Record ");
    Serial.print(i);
    Serial.print(" :");
    Serial.println(highscoreData.records[i]);
  }
}

void loop() {
  switch (crtProgramState) {
    case Greeting: {
      showGreetingMessage();
      break;
    }
    case Playing: {
      if (!hasDisplayedInitialScore) {
        hasDisplayedInitialScore = true;
        displayCrtScore();
      }

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
    case GameOverScreen1: {
      showGameOverScreen1();
      break;
    }
    case GameOverScreen2: {
      showGameOverScreen2();
      break;
    }
    case ResetHighscore: {
      resetHighscoreTable();
      break;
    }
    case ResetHighscoreSuccessful: {
      showHSResetSuccessfulMessage();
      break;
    }
    case DifficultyLevel: {
      showDifficultyLevels();
      break;
    }
  }
}

void showDifficultyLevels () {

  showMenu(difficultyLevels, DIFFICULTY_LEVELS_LENGTH); 
}

void showHSResetSuccessfulMessage () {  
  lcd.setCursor(0, 0);
  lcd.print("HS Records");

  lcd.setCursor(0, 1);
  lcd.print("Have been reset!");

  if (millis() - HSResetSuccessfulMessageTimestamp > HS_RESET_MESSAGE_TIME) {
    lcd.clear();
    
    crtProgramState = Menu;
    isJoystickNeutral = true;
  }
}

void resetHighscoreTable () {
  for (int i = 0; i < HIGHSCORE_RECORDS; i++) {
    if (i == 0) {
      strcpy(highscoreData.records[i], "");
      continue;
    }

    strcpy(highscoreData.records[i], "???");
  }

  writeDataToStorage(HIGHSCORE_START_OFFSET, highscoreData);
  lcd.clear();
  crtProgramState = ResetHighscoreSuccessful;
  HSResetSuccessfulMessageTimestamp = millis();
}

// Screen 1: congrats & show score.
// Screen 2(if 1, 2 or 3): type in username
  // when leaving: save username along with HS   
void showGameOverScreen1 () {
  lcd.setCursor(0, 0);
  lcd.print("Congratulations");

  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(crtScore);

  if (millis() - gameOverScreen1Timestamp > GAME_OVER_SCREEN1_TIME) {
    lcd.clear();
    crtProgramState = GameOverScreen2;
  }
}

void showGameOverScreen2 () {
  int surpassedPlayerIdx = getHighestSurpassedOnPodium(crtScore);
  if (surpassedPlayerIdx == -1) {
    lcd.clear();
    lcd.print("The podium");
    lcd.setCursor(0, 1);
    lcd.print("stays untouched!");

    Directions nextDirection = getDirectionFromJoystick();
    if (nextDirection != -1 && nextDirection == UP) {
      handleItemExit(menuSelectedItemIdx);
      lcd.clear();
      return;
    }
    
    return;
  }

  const char* usernameStr = "Username: ";
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(usernameStr);
  
  getUsernameFromUser(strlen(usernameStr));
}

void getUsernameFromUser (int startCol) {
  lcd.setCursor(startCol + selectedUsernameCharIdx, 0);
  lcd.write((byte)0);

  lcd.setCursor(startCol, 1);
  lcd.print(username);

  int joySwitchValue = !digitalRead(JOY_SW_PIN);
  if (joySwitchValue) {
    saveUsernameAndScore();
    resetGame();

    lcd.clear();
    
    crtProgramState = Menu;

    return;
  }

  Directions nextDirection = getDirectionFromJoystick();
  if (nextDirection == -1) {
    return;
  }

  switch (nextDirection) {
    case RIGHT: {
      modifySelectedUsernameChar(+1, username);
      break;
    }
    case LEFT: {
      modifySelectedUsernameChar(-1, username);
      break;
    }
    case UP: {
      selectedUsernameCharIdx--;
      break;
    }
    case DOWN: {
      selectedUsernameCharIdx++;
      break;
    }
  }

  if (selectedUsernameCharIdx >= 3) {
    selectedUsernameCharIdx = selectedUsernameCharIdx % strlen(username);
  } else if (selectedUsernameCharIdx < 0) {
    selectedUsernameCharIdx = strlen(username) - 1;
  }
}

void resetGame () {
  resetUsername(username);
  crtScore = 0;
  toggleAllMatrixPoints(false);

  hasDisplayedInitialScore = false;

  for (int i = 1; i < snakeDotsCount; i++) {
    free(snakeDots[i]);
  }

  snakeDotsCount = 1;
  snakeDots[0]->row = 0;
  snakeDots[0]->col = 1;
}

void resetUsername (char* username) {
  strcpy(username, "AAA");
}

void modifySelectedUsernameChar (int charStep, char* username) {
  char usernameCh = username[selectedUsernameCharIdx];

  usernameCh += charStep; 
  // TODO: avoid magic numbers!
  if (usernameCh > 90) {
    usernameCh = 65;
  } else if (usernameCh < 65) {
    usernameCh = 90;
  }

  username[selectedUsernameCharIdx] = usernameCh;
}

void saveUsernameAndScore () {
  int surpassedPlayerIdx = getHighestSurpassedOnPodium(crtScore);

  char* recordToBeUpdated = highscoreData.records[surpassedPlayerIdx];

  char scoreStr[9];
  sprintf(scoreStr, "%d", crtScore);

  char newRecord[13];
  strcpy(newRecord, username);
  strcpy(newRecord + strlen(newRecord), ":");   
  strcpy(newRecord + strlen(newRecord), scoreStr);

  // Serial.println(newRecord);
  strcpy(recordToBeUpdated, newRecord);
  
  // Serial.println(highscoreData.first);
  // Serial.println(highscoreData.second);
  // Serial.println(highscoreData.third);
  writeDataToStorage(HIGHSCORE_START_OFFSET, highscoreData);
}

int getHighestSurpassedOnPodium (int crtScore) {
  for (int i = 0; i < HIGHSCORE_RECORDS; i++) {
    int value = atoi(strstr(highscoreData.records[i], ":") + 1);
    if (crtScore > value) {
      return i;
    }
  }

  return -1;
}

void showMenu (char* menuItems[], int menuItemsLength) {
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

  int joySwitchValue = !digitalRead(JOY_SW_PIN);
  if (menuCrtSwitchValue != joySwitchValue && joySwitchValue) {
    // Serial.println("Clicked!!!!!");
    bool shouldContinue = handleItemClick(menuSelectedItemIdx);
    if (!shouldContinue) {
      return;
    }
  }
  menuCrtSwitchValue = joySwitchValue;


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
    
    crtProgramState = Menu;
    isJoystickNeutral = true;
  }
}

void playGame () {
  blinkFood();
  updateSnakeDots();
  checkIfFoodEaten();
  
  // TODO: refactor `isGameOver()`.
  for (int i = 1; i < snakeDotsCount; i++) {
    Position& snakeDot = *snakeDots[i];

    if (arePositionsEqual(crtPos, snakeDot)) {
      gameOverHandler();
      return;
    }
  }

  Directions nextDirection = getDirectionFromJoystick();
  if (nextDirection == -1 || areDirectionsOpposite(nextDirection, crtPos.crtDirection)) {
    return;
  }

  if (nextDirection != crtPos.crtDirection) {
    crtPos.crtDirection = nextDirection;
    markTurningPoint(crtPos);

    if (isGameOver(crtPos, nextDirection)) {
      gameOverHandler(); 
      return;
    }

    checkIfFoodEaten();
  }
}

void gameOverHandler () {
  Serial.println("GAME OVER!");
      
  lcd.clear();
  crtProgramState = GameOverScreen1;
  gameOverScreen1Timestamp = millis();
}

bool isGameOver (Position& crtPos, int nextDirection) {
  Position nextPos = crtPos;
  computeNextPosition(nextDirection, nextPos);

  for (int i = 1; i < snakeDotsCount; i++) {
    Position& snakeDot = *snakeDots[i];

    if (arePositionsEqual(nextPos, snakeDot)) {
      return true;
    }
  }

  return false;
}

void markTurningPoint (Position& pos) {
  if (snakeDotsCount == 1) {
    return;
  }

  turningPoints[pos.row][pos.col] = pos.crtDirection;
}

void checkIfFoodEaten () {
  if (arePositionsEqual(crtPos, foodPos)) {
    addToTail();
    computeRandomFoodPosition();
    crtScore += (settingsData.difficultyLevel + 1) * 1;
    displayCrtScore();
  }
}

void addToTail () {
  Position& tail = *snakeDots[snakeDotsCount - 1];
  Position* newTail = new Position();
  *newTail = tail;

  // Adding in the opposite direction the tail is currently having.
  switch (tail.crtDirection) {
    case UP: {
      computeNextPosition(DOWN, *newTail);
      newTail->crtDirection = UP;      
      break;
    }
    case DOWN: {
      computeNextPosition(UP, *newTail);
      newTail->crtDirection = DOWN;
      break;
    }
    case LEFT: {
      computeNextPosition(RIGHT, *newTail);
      newTail->crtDirection = LEFT;
      break;
    }
    case RIGHT: {
      computeNextPosition(LEFT, *newTail);
      newTail->crtDirection = RIGHT;
      break;
    }
  }

  snakeDots[snakeDotsCount++] = newTail;
}

// Helper fn.
void printPos (Position& pos) {
  Serial.print("col: ");
  Serial.println(pos.col);
  Serial.print("row: ");
  Serial.println(pos.row);
  Serial.print("crtDirection: ");
  Serial.println(pos.crtDirection);
}

bool areDirectionsOpposite (int d1, int d2) {
  switch (d1) {
    case UP: {
      return d2 == DOWN;
    }
    case DOWN: {
      return d2 == UP;
    }
    case LEFT: {
      return d2 == RIGHT;
    }
    case RIGHT: {
      return d2 == LEFT;
    }
  }
}

void updateSnakeDots () {
  if (crtPos.crtDirection == -1) {
    updatedSnakeTimestamp = millis();
    return;
  }

  if (!(millis() - updatedSnakeTimestamp > updateSnakeDotsInterval)) {
    return;
  }

  applyTurningPoints();

  for (int i = 0; i < snakeDotsCount; i++) {
    Position& pos = *snakeDots[i];

    deactivatePointOnMatrix(pos);
    computeNextPosition(pos.crtDirection, pos);
    activatePointOnMatrix(pos);
  }

  updatedSnakeTimestamp = millis();
}

void applyTurningPoints () {
  for (int i = 0; i < snakeDotsCount; i++) {
    Position& pos = *snakeDots[i];
    
    int newDirection = turningPoints[pos.row][pos.col];
    if (newDirection != -1) {
      pos.crtDirection = newDirection;

      if (i == snakeDotsCount - 1) {
        turningPoints[pos.row][pos.col] = -1;
      }
    }
  }
}

void displayCrtScore () {
  lcd.clear();
  lcd.print("Score: ");
  lcd.print(crtScore);
}

void showHighscoreMenu () {
  if (!strlen(highscoreData.records[0])) {
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
  
  // Spent too much time on trying to convert `char[][13]` to `char**`.
  char* records[] = {
    highscoreData.records[0],
    highscoreData.records[1],
    highscoreData.records[2],
    highscoreData.records[3],
    highscoreData.records[4],
  };

  showMenu(records, HIGHSCORE_RECORDS);
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

void computeNextPosition (int direction, Position& crtPos) {
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

  bool needsNewPosition = false;
  for (int i = 0; i < snakeDotsCount; i++) {
    if (arePositionsEqual(foodPos, *snakeDots[i])) {
      needsNewPosition = true;
      break;
    }
  }

  if (!needsNewPosition) {
    return;
  }

  for (int i = 0; i < MATRIX_SIZE; i++) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
      for (int k = 0; k < snakeDotsCount; k++) {
        Position& pos = *snakeDots[i];
        if (pos.row == i && pos.col == j) {
          goto COLUMN;
        } else {
          foodPos.row = i;
          foodPos.col = j;
          
          return;
        }
      }
      COLUMN: continue;
    }
  }
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

bool handleItemClick (int itemIdx) {
  switch (crtProgramState) {
    case SettingsMenu: {
      switch (itemIdx) {
        case 5: {
          // Reset HS.
          crtProgramState = ResetHighscore;

          return false;
        }        
      }
      
      break;
    }
  }

  return true;
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
        case 0: {
          // Difficulty levels.
          crtProgramState = DifficultyLevel;
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
    case GameOverScreen2: {
      crtProgramState = Menu;
      resetGame();
      break;
    }

    case SettingLCDContrast:
    case SettingLCDBrightness:
    case SettingMatrixrightness: {
      crtProgramState = SettingsMenu;
      break;
    }

    case DifficultyLevel: {
      updateSnakeDotsInterval = difficultyLevelsValues[itemIdx];

      settingsData.difficultyLevel = itemIdx;
      writeDataToStorage(SETTINGS_START_OFFSET, settingsData);

      crtProgramState = SettingsMenu;
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
  // Serial.println(nextDirection);
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