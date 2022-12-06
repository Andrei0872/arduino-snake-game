## mini game

<!-- - install joystick -->
<!-- - properly get values from joystick -->
<!-- - add single snake dot on matrix -->
<!-- - make dot move based on joystick movements -->
<!-- - (1) spawn a food dot at a random position -->
<!-- - make the food dot blink -->
<!-- - when food is taken by the snake dot, apply (1) -->

---

## LCD menu

<!-- - greeting message for a few seconds -->
<!-- - highlight currently selected option -->
<!-- - show menu with options -->
<!-- - Play game. on enter -> start -->
<!-- - switch from parent menu to child menu -->
- Highscore.
  <!-- - handle the case where there are no highscores: display message -->
  <!-- - save data in EEPROM
  - read data from EEPROM
  - update certain highscore -->
  - update(if needed) after game is done
- Settings:
  - difficulty level(i.e. snake's speed)
  - LCD contrast. save in EEPROM
  - LCD brightness. save in EEPROM
  - Matrix brightness. save in EEPROM
  - Sounds on or off. save in EEPROM
  **note**: you can use something similar to a range input
<!-- - About: name + GH link -->
<!-- - How to play: it's snake, it doesn't need further explications -->

<!-- switch from parent menu to child menu: -->
<!-- - refactor `showMenu` so that it accepts params(i.e. becomes reusable) -->
<!-- - onClick: switch form parent to child -->
<!-- - onClick: switch form child to parent -->

---

## Snake game

- settings: difficulty level = snake's speed
- add buzzer
- make the snake move continuously in one direction
- keep track of score
- display the score on the LCD
- sound: when eating food
- sound: when losing
- after food is eaten: grow snake
- after food is eaten: spawn another food dot randomly
- if head touches tail: game over
- while playing: display current score
- after game over
  show *Congratulations on reaching level/score X. (1)You did better than y people!*.
  if `(1)` is true:
    ask for username in a second screen
    upon button press, show settings main menu
  else:
    show main menu
- after game over: show sad face on matrix
- fix: perturbation joystick(i.e. use debounce)
- fix(food): always random when starting the game; the crt problem is that the food is always spawned in the same spot
- fix: ensure the random food point is not spawned in an occupied position