#include "LedControl.h"

const int JOY_X_PIN = A0;
const int JOY_Y_PIN = A1;
const int JOY_SW_PIN = 13;
const int JOY_LEFT_TRESHOLD = 400;
const int JOY_RIGHT_TRESHOLD = 600;
const int JOY_TOP_TRESHOLD = 400;
const int JOY_BOTTOM_TRESHOLD = 600;

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

/* ============================================= */

bool isJoystickNeutral = true;

// { 0, 0 } = bottom left corner.
Position crtPos = Position { 0, 1 };

Directions crtDirection = -1, prevDirection;

Position foodPos;
bool isFoodDotActive = true;
unsigned long foodBlinkTimestamp = millis();
/* ============================================= */

void setup() {
  Serial.begin(9600);

  pinMode(JOY_SW_PIN, INPUT_PULLUP);

  lc.shutdown(0, false);
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);

  activatePointOnMatrix(crtPos);
  computeRandomFoodPosition();
}

void loop() {
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