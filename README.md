2D Game Projectile in Opengl(GLFW)
=================

### How to compile:
Run the command make in terminal.

### How to run the game:
Run the command ./game on terminal.

### Rules of the game:

* Basic aim of the game is to collect golden balls to increase your score.
* You have a total of 8 lives (balls) to complete the game.
* The obstacles are in the form of boxes and circles which stop the cannon ball from reaching the coins.

### Controls :
The user can play either by mouse or keyboard.

##### Keyboard Controls

| KEY | ACTION |
| ---:| ---:|
|r|Reload|
|a|Tilt the cannon up|
|b|Tilt the cannon down|
|f|Increase the speed|
|s|Decrease the speed|
|space|Release the Ball|
|esc or q|Quit|
|up arrow|Zoom in|
|down arrow|Zoom out|
|left arrow|Pan left|
|right arrow|Pan right|


#####Mouse Controls
* The position where the user clicks is used to decide the direction and speed of the shot.
* Scroll up to Zoom in.
* Scroll down to Zoom out.
* Press Left Mouse Button to fire the ball from the cannon.
* Press the Right Mouse Button and then move the mouse in the
direction(left/right) you want to Pan.

### Additional Features:
* Red blocks are immovable blocks.
* Red balls act as movable obstacles i.e when hit, they start to move.
* Pink blocks are removable blocks i.e. if you touch them once, they'll dissapear.
* Golden balls are the scoring objects of the game which can be collected by the player.
