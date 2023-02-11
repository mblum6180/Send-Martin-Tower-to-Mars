--! file: gameIntro.lua

Gamestate = require 'libs.hump.gamestate'
gameIntro = {}

function gameIntro:enter()
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
    
    
function gameIntro:update(dt)
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
    
function gameIntro:draw()
    love.graphics.setColor(255, 255, 255, bgAlpha)
    love.graphics.setFont(screenFont)
    love.graphics.printf("Martin Tower, once a Bethlehem landmark, is now a symbol of human determination and technological advancement. As a rocket ship, it will embark on a journey to Mars to serve as the foundation for a new colony. The launch will mark the beginning of humanity's expansion beyond Earth and solidify our presence on the Red Planet. Martin Tower will stand as a testament to our unrelenting spirit of exploration and innovation. The world awaits its historic journey.",
        system.winWidth * 0.1, system.winHeight * 0.1, system.winWidth * 0.8, 'center', 0)

end

function gameIntro:keypressed(key, scancode, isrepeat)
    if key == "s" then
        score = score + 1
        print(score)
    end
    if key == "escape" then
        love.event.quit()
    elseif system.timer >= 3 then
        if key == "space" or key =="left" or key =="right" or key =="down" or key =="up" then
            Gamestate.switch(gameIntro02)
        end
    end
    if key == "m" and objects.audio.mainTheme:isPlaying() then
        playSound(objects.audio.mainTheme,'pause',false) -- pause BG music
    elseif key == "m" then
        playSound(objects.audio.mainTheme,'play',false) -- unpause BG music
    end
end

function gameIntro:touchpressed(id, x, y, pressure)
    if system.timer >= 3 then
      objects.audio.mainTheme:setVolume(0.42)
      Gamestate.switch(mainmenu)
    end
  end



return gameIntro