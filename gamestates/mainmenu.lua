--! file: mainmenu.lua

Gamestate = require 'libs.hump.gamestate'

local mainmenu = {}

function mainmenu:init()
    timer = 1
    bgAlpha = 0
    titleAlpha = 0
    textAlpha = 0
    bgFadein = 1
    titleFadein = 2

    background = love.graphics.newImage("assets/title.png")
    title = love.graphics.newImage("assets/title8bit.png")
    
end
    
    
function mainmenu:update(dt)
    timer = timer + dt
    if timer > bgFadein then 
        bgAlpha = fade(dt, bgAlpha, 0.9)
    end

    if timer > titleFadein then
        titleAlpha = fade(dt,titleAlpha, 1.0)
    end
end
    
function mainmenu:draw()
    love.graphics.setColor(255, 255, 255, bgAlpha)
    love.graphics.draw(background, 0, 0)
    love.graphics.setColor(255, 255, 255, titleAlpha)
    love.graphics.draw(title, 500,100, 0, 3, 3) -- Draw title TEMP xy

end

function mainmenu:keypressed(key, scancode, isrepeat)
    if key == "s" then
        score = score + 1
        print(score)
    end
    if key == "escape" then
        love.event.quit()
    elseif key == "space" then
        Gamestate.switch(gameLevel03)
    end
end



return mainmenu