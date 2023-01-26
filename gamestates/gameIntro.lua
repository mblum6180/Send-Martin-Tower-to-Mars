--! file: mainmenu.lua

Gamestate = require 'libs.hump.gamestate'

local mainmenu = {}

function mainmenu:init()
    windowWidth = love.graphics.getWidth()
    windowHeight = love.graphics.getHeight()
    timer = 1
    bgAlpha = 0
    titleAlpha = 0
    textAlpha = 0
    bgFadein = 1
    titleFadein = 2
    textFadin = 2



end
    
    
function mainmenu:update(dt)
    timer = timer + dt
    if bgAlpha ~= 1 then
        if timer > bgFadein then 
            bgAlpha = fade(dt, bgAlpha, 0.9)
        end
    end
    if titleAlpha ~= 1 then
        if timer > titleFadein then
            titleAlpha = fade(dt,titleAlpha, 1.0)
        end
    end
    if timer > textFadin then
        if ((math.floor(timer)) % 2 == 0) then
            textAlpha = 0
        else textAlpha = 1
        end
    end

end
    
function mainmenu:draw()
    love.graphics.setColor(255, 255, 255, bgAlpha)
    love.graphics.printf("Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when",
    system.winWidth * 0.1, system.winHeight * 0.1, system.winWidth * .14, 'left', 0, 6, 6)

end

function mainmenu:keypressed(key, scancode, isrepeat)
    if key == "s" then
        score = score + 1
        print(score)
    end
    if key == "escape" then
        love.event.quit()
    elseif key == "space" then
        Gamestate.switch(gameLevel01)
    end
end



return mainmenu