--! file: stage01.lua

local Scene = {}

function Scene:load()
background = love.graphics.newImage("Assets/marsmountain.png")
    
end
    
    
function Scene:update(dt)

end
    
function Scene:draw()
  
    love.graphics.draw(background, 20, 0)
end

function Scene:keypressed(key, scancode, isrepeat)
    if key == "escape" then
        love.event.quit()
    elseif key == "space" then
        changeScene("stage02")
    end
end

return Scene