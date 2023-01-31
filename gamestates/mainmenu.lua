--! file: mainmenu.lua

Gamestate = require 'libs.hump.gamestate'

local mainmenu = {}

function mainmenu:init()
    windowWidth = love.graphics.getWidth()
    windowHeight = love.graphics.getHeight()
    system.timer = 1
    bgAlpha = 0
    titleAlpha = 0
    textAlpha = 0
    bgFadein = 1
    titleFadein = 2
    textFadin = 2


    background = love.graphics.newImage("assets/title.png")
    title = love.graphics.newImage("assets/title8bit.png")
    text = love.graphics.newImage("assets/Press-space-to-start8bit.png")
    textWidth = text:getWidth()
    textHeight = text:getHeight()

    objects.audio.mainTheme:setLooping(true) -- play music forever
    playSound(objects.audio.mainTheme,'play',false) -- play BG music
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
    love.graphics.draw(background, 0, 0)
    love.graphics.setColor(255, 255, 255, titleAlpha)
    love.graphics.draw(title, 500,100, 0, 3, 3) -- Draw title TEMP xy
    love.graphics.setColor(255, 255, 255, textAlpha)
    love.graphics.draw(text, windowWidth / 2, (windowHeight / 3) * 2, 0, 3, 3 , textWidth / 2, textHeight / 2) -- Draw title TEMP xy

end

function mainmenu:keypressed(key, scancode, isrepeat)
    if key == "escape" then
        love.event.quit()
    elseif system.timer >= 3 then
        if key == "space" or key =="left" or key =="right" or key =="down" or key =="up" then
            objects.audio.mainTheme:setVolume(0.42)
            Gamestate.switch(gameIntro)
        end
    end

end



return mainmenu