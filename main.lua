--! file: main.lua


Gamestate = require 'libs.hump.gamestate'


mainMenu = require 'gamestates.mainmenu'
gameLevel01 = require 'gamestates.gameLevel01'
gameLevel02 = require 'gamestates.gameLevel02'
gameLevel03 = require 'gamestates.gameLevel03'
pause = require 'gamestates.pause'

function love.load()
  Gamestate.registerEvents()
  Gamestate.switch(mainMenu)
end
