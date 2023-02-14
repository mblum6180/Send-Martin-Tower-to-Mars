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
  gameIntro02 = require 'gamestates.gameIntro02'
  require "objects"


  love.graphics.setDefaultFilter('nearest', 'nearest')
  --love.window.setMode(1280, 720) -- The actual resulting resolution will match the screen.

  love.physics.setMeter(64 * system.scaling)  -- physics setup, set the meter to 64 pixels
  earth = love.physics.newWorld(0, 9.80*64, true)  -- create a new world with gravity of 9.8 m/s^2
  space = love.physics.newWorld(0, 0.1*64, true)  -- create a new world with gravity of 0.1 m/s^2
  mars = love.physics.newWorld(0, 3.72*64, true)  -- create a new world with gravity of 3.72 m/s^2

  screenFont = love.graphics.newFont('assets/font.ttf', 42  / love.window.getDPIScale( ))  -- create a new font with a size of 42
  scoreFont = love.graphics.newFont('assets/font.ttf', 128  / love.window.getDPIScale( ))  -- create a new font with a size of 128
  messageFont = love.graphics.newFont('assets/font.ttf', 80  / love.window.getDPIScale( ))  -- create a new font with a size of 80



  Gamestate.registerEvents()


  coolDown = 0
  if arg[2] == "debug" then
    debugMode = true
    print("Debug mode = ", debugMode)
    levelSelect(arg[3])
  else
    Gamestate.switch(gameIntro)
  end

  if debugMode then
    system.score02 = 9000
    system.score03 = 9000
  end
  
end

function fade(dt,d,x)  --dt, Alpha, duration
  if d < 1 then --if the alpha is less than 1
    d = d + x * dt --add the alpha to the duration
  else d = 1 --if the alpha is greater than 1, set it to 1
  end
  --print(d)
  return d

end

function touchpressed(id, x, y, dx, dy, pressure)
  Gamestate.touchpressed(id, x, y, dx, dy, pressure)
end

function touchreleased(id, x, y, pressure)
  Gamestate.touchreleased(id, x, y, pressure)
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
    Gamestate.switch(gameIntro)
  end
end

function getEasterSunday(year)
  local a = year % 19
  local b = math.floor(year / 100)
  local c = year % 100
  local d = math.floor(b / 4)
  local e = b % 4
  local f = math.floor((b + 8) / 25)
  local g = math.floor((b - f + 1) / 3)
  local h = (19 * a + b - d - g + 15) % 30
  local i = math.floor(c / 4)
  local k = c % 4
  local l = (32 + 2 * e + 2 * i - h - k) % 7
  local m = math.floor((a + 11 * h + 22 * l) / 451)
  local month = math.floor((h + l - 7 * m + 114) / 31)
  local day = ((h + l - 7 * m + 114) % 31) + 1
  return year, month, day
end

