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
  <!-- - fix: display highscore -->
  <!-- - handle the case where there are no highscores: display message -->
  <!-- - save data in EEPROM
  - read data from EEPROM
  - update certain highscore -->
  <!-- - update(if needed) after game is done -->
- Settings:
  - difficulty level(i.e. snake's speed)
  <!-- - LCD contrast. save in EEPROM - 0-255 -->
  <!-- - LCD brightness. save in EEPROM - 0-255; -->
    <!-- - connect to PWM pin(e.g. 6) -->
  <!-- - Matrix brightness. save in EEPROM - 0-15 -->
  - Sounds on or off. save in EEPROM
  <!-- **note**: you can use something similar to a range input -->
<!-- - About: name + GH link -->
<!-- - How to play: it's snake, it doesn't need further explications -->

<!-- switch from parent menu to child menu: -->
<!-- - refactor `showMenu` so that it accepts params(i.e. becomes reusable) -->
<!-- - onClick: switch form parent to child -->
<!-- - onClick: switch form child to parent -->

<!-- 1. refactor: use struct instead of separated string -->
<!-- 2. control LCD brightness(connect to PWM pin, e.g. 6) -->
<!-- 3. generic input range component(function) -->

refactor: `lcd.print` only when needed, not on every loop iteration
<!-- perf: make read & write fns to storage generic -->

---

## Snake game

<!-- - make the snake move continuously in one direction -->
<!-- - keep track of score -->
<!-- - after food is eaten: grow snake -->
<!-- - handle direction changes -->
<!-- - after food is eaten: **properly** spawn another food dot randomly -->
<!-- - prevent switching direction 180 deg -->
<!-- - if head touches tail or any part of the body: game over -->
<!-- - while playing: display current score -->
- after game over
  <!-- show *Congratulations on reaching level/score X. `(1)`You did better than y people!*. -->
  <!-- if `(1)` is true: -->
    <!-- - ask for username in a second screen -->
    <!-- - save score  -->
    <!-- - upon button press, show settings main menu -->
    <!-- - reset username after save -->
    <!-- - find arrow down glyph -->
  <!-- else: -->
    <!-- show main menu -->
<!-- - after game over: show sad face on matrix -->
<!-- - fix: ensure game over when teleporting results in touching the snake's body -->
<!-- - fix(food): always random when starting the game; the crt problem is that the food is always spawned in the same spot -->
<!-- - fix: ensure the random food point is not spawned in an occupied position -->
<!-- - fix: move snake glyph at the beginning -->

?
- add buzzer
- sound: when eating food
- sound: when losing

?
- settings: difficulty level = snake's speed

- refactor: clean code, constants, comments, cruft, simplify switch cases statements etc.

<!-- - quick presentation -->
<!-- - highscore: at least 5  -->
<!-- - *reset highscores* button -->
- format podium number to adjective
<!-- - properly format the HS -->