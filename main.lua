--! file: main.lua
function love.load()

  Gamestate = require 'libs.hump.gamestate' -- Game levels
  mainMenu = require 'gamestates.mainmenu'
  gameLevel01 = require 'gamestates.gameLevel01'
  gameLevel02 = require 'gamestates.gameLevel02'
  gameLevel03 = require 'gamestates.gameLevel03'
  pause = require 'gamestates.pause'

  system = {}
  system.winWidth = love.graphics.getPixelWidth()
  system.winHeight = love.graphics.getPixelHeight()
  system.level01over = false
  system.level02over = false
  system.level03over = false
  love.physics.setMeter(64)  -- physics setup
  earth = love.physics.newWorld(0, 9.80*64, true)
  space = love.physics.newWorld(0, 0.1*64, true)
  mars = love.physics.newWorld(0, 3.72*64, true)
  require "objects"
  Gamestate.registerEvents()
  score = 0
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