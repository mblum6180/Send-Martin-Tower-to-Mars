--! file: gameLevel02.lua

Gamestate = require 'libs.hump.gamestate'

local gameLevel02 = {}

function gameLevel02:init()
    background = love.graphics.newImage("assets/marsmountain.png")
end
    
    
function gameLevel02:update(dt)

end
    
function gameLevel02:draw()
    love.graphics.draw(background, 20, 0)
end

function gameLevel02:keypressed(key, scancode, isrepeat)
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

return gameLevel02