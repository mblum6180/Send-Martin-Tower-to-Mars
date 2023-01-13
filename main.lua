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
  system.BGScale = 1
  system.BGcolorR = 1
  system.BGcolorG = 1
  system.BGcolorB = 1
  love.physics.setMeter(64)  -- physics setup
  earth = love.physics.newWorld(0, 9.80*64, true)
  space = love.physics.newWorld(0, 0.01*64, true)
  mars = love.physics.newWorld(0, 3.72*64, true)
  require "objects"
  Gamestate.registerEvents()
  score = 0
  coolDown = 0
  if arg[2] == "debug" then
    debugMode = true
    print("Debug mode = ", debugMode)
  end
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

function edge(x, y)
  if x < 100 then
      objects.tower.body:applyForce(400, 0)
      system.BGcolorG,system.BGcolorB = 0.25, 0.25
      if x < 10 then
          objects.tower.body:applyForce(4000, 0)
          system.BGcolorG,system.BGcolorB = 0.01, 0.01  
      end
  elseif x > 1100 then
      objects.tower.body:applyForce(-400, 0)
      system.BGcolorG,system.BGcolorB = 0.25, 0.25
      if x > 1170 then
          objects.tower.body:applyForce(-4000, 0)
          system.BGcolorG,system.BGcolorB = 0.01, 0.01  
      end
  else
      system.BGcolorG,system.BGcolorB = 1.0, 1.0
  end
end

function playSound(sound)
  sound:seek(0)
  pitchMod = 0.8 + love.math.random(0, 10)/25
  sound:setPitch(pitchMod)
  sound:play()
end
