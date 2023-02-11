--! file: mainmenu.lua

Gamestate = require 'libs.hump.gamestate'
gameLevelGoal03 = {}

function gameLevelGoal03:enter()
    love.graphics.reset()
    windowWidth = love.graphics.getWidth()
    windowHeight = love.graphics.getHeight()
    system.timer = 1
    bgAlpha = 0
    titleAlpha = 0
    textAlpha = 0
    bgFadein = 1
    titleFadein = 2
    textFadin = 2


end
    
    
function gameLevelGoal03:update(dt)
    system.timer = system.timer + dt
    if bgAlpha ~= 1 then
        if system.timer > bgFadein then 
            bgAlpha = fade(dt, bgAlpha, 0.9)
        end
    end
    if titleAlpha ~= 1 then
        if system.timer > titleFadein then
            titleAlpha = fade(dt,titleAlpha, 1.0)
        end
    end
    if system.timer > textFadin then
        if ((math.floor(system.timer)) % 2 == 0) then
            textAlpha = 0
        else textAlpha = 1
        end
    end

end
    
function gameLevelGoal03:draw()
    love.graphics.setColor(255, 255, 255, bgAlpha)
    love.graphics.setFont(screenFont)
    love.graphics.printf("Congratulations!\n\nMartin Tower has safely landed on the red planet, Mars. Your expert navigation skills and quick reflexes made this historic mission a success.\n\nWell done!",system.winWidth * 0.1, system.winHeight * 0.05, system.winWidth * 0.8, 'center', 0)
    love.graphics.printf(("Meteoroids hit: "..system.itemsDestroyed.."\n"..
        "Fuel: "..math.floor(system.score03).."\n"..
        "Bonus: "..math.floor(system.bonus).."\n"..
        "Final Score: "..math.floor(system.score03) + math.floor(system.bonus)),
        system.winWidth * 0.1, system.winHeight * 0.6, system.winWidth * 0.8, 'center', 0)
    love.graphics.printf("Press an arrow key to restart",system.winWidth * 0.1, system.winHeight * 0.9, system.winWidth * 0.8, 'center', 0)


end

function gameLevelGoal03:keypressed(key, scancode, isrepeat)

    if key == "escape" then
        love.event.quit()
    elseif system.timer >= 3 then
        if key == "space" or key =="left" or key =="right" or key =="down" or key =="up" then
            Gamestate.switch(gameLevel03)
        end
    end
    if key == "m" and objects.audio.mainTheme:isPlaying() then
        playSound(objects.audio.mainTheme,'pause',false) -- pause BG music
    elseif key == "m" then
        playSound(objects.audio.mainTheme,'play',false) -- unpause BG music
    end
end
function gameLevelGoal03:touchpressed(id, x, y, pressure)
    if system.timer >= 3 then
      objects.audio.mainTheme:setVolume(0.42)
      Gamestate.switch(gameLevel03)
    end
  end

function gameLevel03:leave()
    system.level01over = false
    system.level02over = false
    system.level03over = false
    system.BGScale = 1
    system.BGcolorR = 1
    system.BGcolorG = 1
    system.BGcolorB = 1
    system.score01 = math.floor(system.score03) + math.floor(system.bonus)
    system.score02 = 0
    system.score03 = 0
    system.score = 0
    system.landed = false
    system.landedTimer = 2
    system.launch = "left"
    system.crashed = false
    system.winner = false
    system.timer = 0
    system.bonus = 0
    objects.tower.strengthTorque = 2000
    objects.tower.strengthMain = 2000
    objects.tower.strengthSide = 300
    objects.tower.fire = false
    objects.tower.empty = false
    objects.tower.crashed = false

end





return gameLevelGoal03