--! file: main.lua
function love.load()

  Gamestate = require 'libs.hump.gamestate' -- Game levels
  mainMenu = require 'gamestates.mainmenu'
  gameLevel01 = require 'gamestates.gameLevel01'
  gameLevel02 = require 'gamestates.gameLevel02'
  gameLevel03 = require 'gamestates.gameLevel03'
  gameLevelGoal01 = require 'gamestates.gameLevelGoal01'
  gameLevelGoal02 = require 'gamestates.gameLevelGoal02'
  gameLevelGoal03 = require 'gamestates.gameLevelGoal03'
  gameIntro = require 'gamestates.gameIntro'
  pause = require 'gamestates.pause'
  require "objects"

  love.graphics.setDefaultFilter('nearest', 'nearest')

  love.physics.setMeter(64 * system.scaling)  -- physics setup
  earth = love.physics.newWorld(0, 9.80*64, true)
  space = love.physics.newWorld(0, 0.1*64, true)
  mars = love.physics.newWorld(0, 3.72*64, true)

  screenFont = love.graphics.newFont('assets/font.ttf', 42)


  Gamestate.registerEvents()
  
  coolDown = 0
  if arg[2] == "debug" then
    debugMode = true
    print("Debug mode = ", debugMode)
    levelSelect(arg[3])
  else
    Gamestate.switch(mainMenu)
  end

  if debugMode then
    system.score02 = 9000
    system.score03 = 9000
end
  
end

function fade(dt,d,x)  --dt, Alpha, duration 
  if d < 1 then
    d = d + x * dt
  else d = 1
  end
  --print(d)
  return d

end




function beginContact(obj1,obj2)
  Gamestate.beginContact(obj1,obj2)
end

function endContact(obj1,obj2)
  Gamestate.endContact(obj1,obj2)
end

function edge(x, y)
  if x < 100 then
      objects.tower.body:applyForce(100, 0)
      system.BGcolorG,system.BGcolorB = 0.25, 0.25
      if x < 10 then
          objects.tower.body:applyForce(1000, 0)
          system.BGcolorG,system.BGcolorB = 0.01, 0.01  
      end
  elseif x > system.winWidth - 100 then
      objects.tower.body:applyForce(-100, 0)
      system.BGcolorG,system.BGcolorB = 0.25, 0.25
      if x > system.winWidth - 10 then
          objects.tower.body:applyForce(-1000, 0)
          system.BGcolorG,system.BGcolorB = 0.01, 0.01  
      end
  else
      system.BGcolorG,system.BGcolorB = 1.0, 1.0
  end
end

function playSound(sound,mode,pitchMod,x,y)
  if x then 
    sound:setDirection( x, y, 0)
  end
  if mode == 'play' then
    if pitchMod == true then
      pitchMod = 0.8 + love.math.random(0, 10)/25
      sound:setPitch(pitchMod)
    end
    sound:play()
  elseif mode == 'stop' then
    sound:stop()
  elseif mode == 'pause' then
    sound:pause()
  end
end

function levelSelect(level)
  --print(level)
  if level == "01" then
    Gamestate.switch(gameLevel01)
  elseif level == "02" then
    Gamestate.switch(gameLevel02)
  elseif level == "03" then
    Gamestate.switch(gameLevel03)
  else
    Gamestate.switch(mainMenu)
  end
end
