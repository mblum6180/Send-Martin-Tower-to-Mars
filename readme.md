# Send Martin Tower to Mars!
#### Video Demo:  <https://youtu.be/lVubAQINHxs>
#### Description:
## Send a tower on a journey through space to land on Mars.

Send Martin Tower to Mars! is a game developed using LUA LÖVE framework. Libraries it uses include Box2D(for physics) and HUMP (for gamestate)
Game was done in a 8bit style with a limited color pallete used. 

## About each major file
## main.lua
This file set up the game and contains game wide functions.
Functions include fade() which does screen fades
edge() which pushes the tower object if its too close to the edge of the screen
playSound() which controls the audio and provides stop and pitch control
levelSelect() which controls the current gamestate.

## objects.lua
Definitions for system wide variables. 
Including setup for animations using quads.


## gameLevel01.lua
Gamestate for level one.
Simplest of the three levels in the game. The main mechanic is to press left and right repeatedly to fill up the fuel tank before launch. 

Uses a countdown timer which once reaches zero will no longer accept input and will start the launch fuction.

## gameLevel02.lua
Level two is a flying in-space simulation. It's a timed level with the player object moving at a fixed rate toward the top of the screen. Once they reach the top the level ends. The player must avoid the debris that is randomly generated. If the player hits debris once there is fuel loss and if they hit the same debris again there is even more fuel loss and the debris is destroyed. 
All debris is destroyed once it scrolls off-screen.

Background scales as time goes on to imply the feeling of movement.

## gameLevel03.lua
Level three is a Lunar Lander style game. The player must land on the surface slowly enough to not crash and onto a level surface to not tip over. Upon load, the surface is randomly generated to create a new experience for each play. If the player lands without crashing the bonus score is to be calculated based on landing speed and object angle.

## mainmenu.lua
Main menu for the game with a splash screen with the title. 

## gameLevelIntro.lua
Basic text info screen that tells the player the controls and goals for the first level.

## gameLevelGoal01.lua
Basic text info screen that tells the player of the goals for the second level.

## gameLevelGoal02.lua
Basic text info screen that tells the player of the goals for the third level.

## gameLevelGoal03.lua
Basic text info screen that tells the player the final score and an option to play again.