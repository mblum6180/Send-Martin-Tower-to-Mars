--! file: main.lua

function love.load()
  changeScene("title")

end
  
  
  function love.update(dt)
    if Scene.update then Scene:update(dt)
    end
  end
  
function love.draw()
  if Scene.draw then Scene:draw()
  end
end



function love.keypressed(key, scancode, isrepeat)
  if Scene.keypressed then Scene:keypressed(key, scancode, isrepeat)
  end
end


function changeScene(scene)
  Scene = require("Scenes/"..scene)
  if Scene.load then Scene:load()
  end
end 