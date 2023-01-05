--! file: stage02.lua

local Scene = {}

function Scene:load()
background = love.graphics.newImage("Assets/marsmountain.png")
    
end
    
    
function Scene:update(dt)

end
    
function Scene:draw()
  
    love.graphics.draw(background, 0, 0)
end

function Scene:keypressed(key, scancode, isrepeat)
    if key == "escape" then
        love.event.quit()
    elseif key == "space" then
        changeScene("stage03")
    end
end

return Scene