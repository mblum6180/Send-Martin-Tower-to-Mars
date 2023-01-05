--! file: gameLevel01.lua

Gamestate = require 'libs.hump.gamestate'

local gameLevel01 = Gamestate.new()

function gameLevel01:init()
    background = love.graphics.newImage("assets/marsmountain.png")
end
    
    
function gameLevel01:update(dt)

end
    
function gameLevel01:draw()
    love.graphics.draw(background, 20, 0)
end

function gameLevel01:keypressed(key, scancode, isrepeat)
    if key == "s" then
        score = score + 1
        print(score)
    end
    if key == "escape" then
        love.event.quit()
    elseif key == "space" then
        changeScene(gameLevel02)
    end
end

return gameLevel01