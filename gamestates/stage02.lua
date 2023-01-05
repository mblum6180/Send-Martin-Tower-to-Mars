--! file: stage02.lua

local Scene = {}

function Scene:load()
background = love.graphics.newImage("assets/marsmountain.png")
    
end
    
    
function Scene:update(dt)

end
    
function Scene:draw()
  
    love.graphics.draw(background, 0, 0)
end

function Scene:keypressed(key, scancode, isrepeat)
    if key == "s" then
        score = score + 1
        print(score)
    end
    if key == "escape" then
        love.event.quit()
    elseif key == "space" then
        changeScene("stage03")
    end
end

return Scene