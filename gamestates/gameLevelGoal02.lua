--! file: mainmenu.lua

Gamestate = require 'libs.hump.gamestate'

gameLevelGoal02 = {}

function gameLevelGoal02:enter()
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
    
    
function gameLevelGoal02:update(dt)
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
    
function gameLevelGoal02:draw()
    love.graphics.setColor(255, 255, 255, bgAlpha)
    love.graphics.setFont(screenFont)
    love.graphics.printf("Well done! You navigated the dangerous meteoroid field with ease.\n\nNext, it's time to softly land Martin Tower at its new home on the red planet, Mars.\nPress in the center or up to slow your descent.",
    system.winWidth * 0.1, system.winHeight * 0.1, system.winWidth * 0.8, 'center', 0)
end

function gameLevelGoal02:keypressed(key, scancode, isrepeat)

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

function gameLevelGoal02:mousepressed(x, y, istouch)
    if system.timer >= 3 then
      objects.audio.mainTheme:setVolume(0.42)
      Gamestate.switch(gameLevel03)
    end
  end







return gameLevelGoal02