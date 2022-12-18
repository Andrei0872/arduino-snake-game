# Arduino Snake Game

*[Other repo](https://github.com/Andrei0872/IntroductionToRobotics) with robotics homework*.

- [Arduino Snake Game](#arduino-snake-game)
  - [Task requirements](#task-requirements)
  - [Picture of the set-up](#picture-of-the-set-up)
  - [Link to the video](#link-to-the-video)
  - [Used components](#used-components)
  - [Interesting Challenges](#interesting-challenges)
    - [The Snake Game](#the-snake-game)
    - [Working with pointers](#working-with-pointers)
    - [`showMenu` used everywhere](#showmenu-used-everywhere)
    - [Getting input from joystick](#getting-input-from-joystick)
    - [Writing to and reading from EEPROM](#writing-to-and-reading-from-eeprom)
    - [Using templates](#using-templates)

## Task requirements

* basic sounds when eating, when navigating through the menu, when the game is over, etc.
* difficulty levels
* the option to customize settings(e.g. LCD contrast/brightness, sounds enabled or not) and save the settings
* highscore board which will be updated accordingly

---

## Picture of the set-up

<div style="text-align: center;">
  <img src="./assets/matrice.jpg">
</div>

## Link to the video

The video showcasing the functionality can be found [here](https://youtu.be/osQ7XLqH9wk).

---

## Used components

* LCD
* a joystick
* a buzzer
* led matrix
* shift register

---

## Interesting Challenges

### The Snake Game

Although it's a simple game, there were a few challenges when it came to its implementation. The building blocks are the following:

```cpp
enum Directions {
  LEFT = 0,
  RIGHT = 1,
  UP = 2,
  DOWN = 3,
};

Position* snakeDots[MATRIX_SIZE * MATRIX_SIZE];
int snakeDotsCount = 0;
Directions turningPoints[MATRIX_SIZE][MATRIX_SIZE];
```

`turningPoints` will keep track of the **current direction** of each snake dot. A snake, especially if it is comprised of multiple points, it can turn many times. Finding out the direction of each point can be achieved by using `turningPoints[x][y]`.

Part of the reason as to why `turningPoints` is needed is that the snake will continuously move in one certain direction, once the game has started. However, not all of the snake's dots will follow the same direction, mainly because the snake will most likely turn a few times. So, we must take into account the turning points as well when the snake is moving.

Before determining the next position on the matrix of each snake dot, `applyTurningPoints()` is called:

```cpp
void applyTurningPoints () {
  for (int i = 0; i < snakeDotsCount; i++) {
    Position& pos = *snakeDots[i];
    
    int newDirection = turningPoints[pos.row][pos.col];
    if (newDirection != -1) {
      pos.crtDirection = newDirection;

      // If the tail reached this point, then we can no longer consider this turning point.
      if (i == snakeDotsCount - 1) {
        turningPoints[pos.row][pos.col] = -1;
      }
    }
  }
}
```

### Working with pointers

The concept of pointers never seemed daunting to me, but I could never understand them properly. I'm very glad that, after completing this project, I feel more comfortable working pointers.

I feel proud of being able to utilize such syntax:

```cpp
Position* snakeDots[MATRIX_SIZE * MATRIX_SIZE];

/* ... */

Position& pos = *snakeDots[i];
```

### `showMenu` used everywhere

Whenever a (scrollable) menu is displayed on the LCD, the `showMenu()` function is responsible for that. I've used it for showing the main menu, the settings menu, the highscore records and more:

```cpp
/* ... */
showMenu(menuItems, MENU_ITEMS_LENGTH);
/* ... */
showMenu(settingsMenuItems, SETTINGS_MENU_ITEMS_LENGTH);
/* ... */
showMenu(difficultyLevels, DIFFICULTY_LEVELS_LENGTH); 
/* ... */
```

One language feature that was crucial for this function was the use of **pointers**. Each menu, if scrollable, needs a pair of indexes, one for keeping track of the currently selected item and the other for keeping track of the *scrollable view*. Here is how pointers came handy:

```cpp
// Example: the Settings menu.

menuItemIdxPtr = &settingsMenuItemIdx;
menuSelectedItemIdxPtr = &settingsMenuSelectedItemIdx;

showMenu(settingsMenuItems, SETTINGS_MENU_ITEMS_LENGTH);

/* ... */

void showMenu (char* menuItems[], int menuItemsLength) {
  // Getting the actual values here.
  menuItemIdx = *menuItemIdxPtr;
  menuSelectedItemIdx = *menuSelectedItemIdxPtr;

  /* ... indexes logic, getting input from joystick, etc. ... */

  // Updating the pointers with the new values!
  *menuItemIdxPtr = menuItemIdx;
  *menuSelectedItemIdxPtr = menuSelectedItemIdx;
}
```

### Getting input from joystick

This was one of the most challenging parts, but luckily, I got it right within the first attempts.

The `getDirectionFromJoystick()` has throttling included, meaning that it doesn't return the same direction(i.e. `UP`, `DOWN`, `LEFT`, `RIGHT`) unless the handle reaches again the *neutral* state.

### Writing to and reading from EEPROM

Initially, I thought I would store all the data as a big string with some *cleverly* chosen separators. I'm glad that I ended up with a different approach: using **structs**:

```cpp
struct Highscore {
  char records[5][13];
} highscoreData;

const int SETTINGS_START_OFFSET = HIGHSCORE_START_OFFSET + sizeof(Highscore);
/* ... */
struct Settings {
  byte difficultyLevel;
  byte LCDContrast = DEFAULT_LCD_CONTRAST_VALUE;
  byte LCDBrightness = DEFAULT_LCD_BRIGHTNESS_VALUE;
  byte matrixBrightness = DEFAULT_MATRIX_BRIGHTNESS_VALUE;
  bool hasSoundsOn;
} settingsData;
```

Now, writing to and reading from `EEPROM` is as simple as rightly calling `EEPROM.get` and `EEPROM.put`, respectively.

Moreover, if I'd want to store more data to `EEPROM`, I'd simply do:

```cpp
const int OTHER_CONFIGS_START_OFFSET = SETTINGS_START_OFFSET + sizeof(Settings);
struct OtherConfigs {
  char config1[10];
  int config2;
  byte config3;
}
```

### Using templates

Although I'm familiar with the concept(`TypeScript` has something similar, called [*generics*](https://www.typescriptlang.org/docs/handbook/2/generics.html)), I never really got the change to use them in C/C++.

Here's how I used them and I think they are perfectly suited for this case:

```cpp
template<typename T> int readDataFromStorage (int offset, T& data) {  
  EEPROM.get(offset, data);

  return offset + sizeof(data);
}
template<typename T> int writeDataToStorage (int offset, T& data) {
  EEPROM.put(offset, data);

  return offset + sizeof(data);
}
```