--! file: gameLevel01.lua

-- Luanch

Gamestate = require 'libs.hump.gamestate'

local gameLevel01 = {}
function gameLevel01:init()
    timer = 1
    bgAlpha = 0
    bgFadein = 1
    background = love.graphics.newImage("assets/CityBG8bit.png")
end
    
    
function gameLevel01:update(dt)
    timer = timer + dt
    if bgAlpha ~= 1 then
        if timer > bgFadein then 
            bgAlpha = fade(dt, bgAlpha, 0.9)
        end
    end

end
    
function gameLevel01:draw()
    love.graphics.setColor(255, 255, 255, bgAlpha)
    love.graphics.draw(background, 20, 0)
    love.graphics.setColor(255, 255, 255, 1)

    love.graphics.setColor(0.53, 0.39, 0.32)
    love.graphics.polygon("fill", objects.ground.body:getWorldPoints(objects.ground.shape:getPoints()))
  
    love.graphics.setColor(1.0, 1.0, 1.0)
    love.graphics.draw(objects.tower.image, system.winWidth/10, system.winHeight/2.7, 0, 3, 3)


    --building:draw()
end

function gameLevel01:keypressed(key, scancode, isrepeat)
    if key == "s" then
        score = score + 1
        print(score)
    end
    if key == "escape" then
        love.event.quit()
    elseif key == "space" then
        Gamestate.switch(gameLevel02)
    end
end

return gameLevel01