--! file: mainmenu.lua

Gamestate = require 'libs.hump.gamestate'
mainMenu = {}

function mainMenu:enter()
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
    titleWidth = title:getWidth()
    titleHeight = title:getHeight()
    text = love.graphics.newImage("assets/Press-space-to-start8bit.png")
    textWidth = text:getWidth()
    textHeight = text:getHeight()

    objects.audio.mainTheme:setLooping(true) -- play music forever
    if objects.audio.mainTheme:tell() == 0 then --check if at start
        playSound(objects.audio.mainTheme,'play',false) -- play BG music
    end
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
    love.graphics.draw(background, 0, 0, 0, system.winWidth / backgroundWidth, system.winHeight / backgroundHeight) -- Draw BG
    love.graphics.setColor(255, 255, 255, titleAlpha)
    love.graphics.draw(title, system.winWidth * 0.5, system.winHeight * 0.2, 0, 3 * system.scaling , 3 * system.scaling, titleWidth / 2, titleHeight / 2) -- Draw title 
    love.graphics.setFont(screenFont)
    love.graphics.printf(system.version, -system.scaling * 20, system.winHeight * 0.9, system.winWidth , 'right', 0) -- draw version
    love.graphics.setColor(255, 255, 255, textAlpha)
    love.graphics.draw(text, system.winWidth * 0.5, (system.winHeight / 3) * 2, 0, 3 * system.scaling, 3  * system.scaling, textWidth / 2, textHeight / 2) -- Draw Press start 

   

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
    if key == "m" and objects.audio.mainTheme:isPlaying() then
        playSound(objects.audio.mainTheme,'pause',false) -- pause BG music
    elseif key == "m" then
        playSound(objects.audio.mainTheme,'play',false) -- unpause BG music
    end

end

function mainMenu:touchpressed(id, x, y, pressure)
    if system.timer >= 3 then
      objects.audio.mainTheme:setVolume(0.42)
      Gamestate.switch(gameIntro)
    end
  end



return mainMenu