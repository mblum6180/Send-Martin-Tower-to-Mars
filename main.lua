--! file: main.lua


Gamestate = require 'libs.hump.gamestate'


mainMenu = require 'gamestates.mainmenu'
gameLevel01 = require 'gamestates.gameLevel01'
gameLevel02 = require 'gamestates.gameLevel02'
gameLevel03 = require 'gamestates.gameLevel03'
pause = require 'gamestates.pause'

score = 0

function love.load()
  Gamestate.registerEvents()
  Gamestate.switch(mainMenu)
end

function fade(dt,d,x)  --dt, Alpha, duration 
  if d < 1 then
    d = d + x * dt
  else d = 1
  end
  --print(d)
  return d

end


function beginContact()
  Gamestate.beginContact()
end