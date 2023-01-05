--! file: mainmenu.lua

local score = 0

Gamestate = require 'libs.hump.gamestate'

local mainmenu = Gamestate.new()

function mainmenu:init()
    background = love.graphics.newImage("assets/marsmountain.png")
    
end
    
    
function mainmenu:update(dt)

end
    
function mainmenu:draw()
  
    love.graphics.draw(background, 0, 0)
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