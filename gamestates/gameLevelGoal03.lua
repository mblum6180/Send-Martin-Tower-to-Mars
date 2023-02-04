--! file: mainmenu.lua

Gamestate = require 'libs.hump.gamestate'

local mainmenu = {}

function mainmenu:init()
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
    
    
function mainmenu:update(dt)
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
    
function mainmenu:draw()
    love.graphics.setColor(255, 255, 255, bgAlpha)
    love.graphics.setFont(screenFont)
    love.graphics.printf(("Debris Destroyed: "..system.itemsDestroyed.."\n"..
        "Fuel: "..math.floor(system.score03).."\n"..
        "Bonus: "..math.floor(system.bonus).."\n"..
        "Final Score: "..math.floor(system.score03) + math.floor(system.bonus)),
        system.winWidth * 0.1, system.winHeight * 0.1, system.winWidth * 0.8, 'center', 0, system.scaling, system.scaling)
    love.graphics.printf("Press an arrow key to restart",system.winWidth * 0.1, system.winHeight * 0.1, system.winWidth * 0.8, 'center', 0, system.scaling, system.scaling)


end

function mainmenu:keypressed(key, scancode, isrepeat)

    if key == "escape" then
        love.event.quit()
    elseif system.timer >= 3 then
        if key == "space" or key =="left" or key =="right" or key =="down" or key =="up" then
            love.event.quit("restart")
        end
    end
end



return mainmenu