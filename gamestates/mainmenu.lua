--! file: mainmenu.lua

Gamestate = require 'libs.hump.gamestate'
mainMenu = {}

function mainMenu:enter()
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
    backgroundWidth = background:getWidth()
    backgroundHeight = background:getHeight()
    title = love.graphics.newImage("assets/title8bit.png")
    text = love.graphics.newImage("assets/Press-space-to-start8bit.png")
    textWidth = text:getWidth()
    textHeight = text:getHeight()

    objects.audio.mainTheme:setLooping(true) -- play music forever
    playSound(objects.audio.mainTheme,'play',false) -- play BG music
end
    
    
function mainMenu:update(dt)
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
    
function mainMenu:draw()
    love.graphics.setColor(255, 255, 255, bgAlpha)
    love.graphics.draw(background, 0, 0, 0, system.winWidth / backgroundWidth, system.winHeight / backgroundHeight)
    love.graphics.setColor(255, 255, 255, titleAlpha)
    love.graphics.draw(title, 500,100, 0, 3, 3) -- Draw title 
    love.graphics.setFont(screenFont)
    love.graphics.printf(system.version, -system.scaling * 20, system.winHeight * 0.9, system.winWidth , 'right', 0) -- draw version
    love.graphics.setColor(255, 255, 255, textAlpha)
    love.graphics.draw(text, windowWidth / 2, (windowHeight / 3) * 2, 0, 3, 3 , textWidth / 2, textHeight / 2) -- Draw title 

   

end

function mainMenu:keypressed(key, scancode, isrepeat)
    if key == "escape" then
        love.event.quit()
    elseif system.timer >= 3 then
        if key == "space" or key =="left" or key =="right" or key =="down" or key =="up" then
            objects.audio.mainTheme:setVolume(0.42)
            Gamestate.switch(gameIntro)
        end
    end

end



return mainMenu