--! file: stage03.lua

Gamestate = require 'libs.hump.gamestate'

local gameLevel03 = {}
function gameLevel03:init()
    background = love.graphics.newImage("assets/marsmountain8bit.png") -- Background image

    world:setCallbacks(beginContact, endContact, preSolve, postSolve)

    love.graphics.setBackgroundColor(0.92, 0.70, 0.60)
end

function gameLevel03:update(dt)
    world:update(dt) 

    if love.keyboard.isDown("right") then
      objects.tower.body:applyForce(400, 0)
    elseif love.keyboard.isDown("left") then
      objects.tower.body:applyForce(-400, 0)
    end
    if love.keyboard.isDown("up") then
        objects.tower.body:applyForce(0, -3000)
    end
end
  
function gameLevel03:draw()
    love.graphics.draw(background, 0, 0)
    love.graphics.setColor(0.53, 0.39, 0.32)
    love.graphics.polygon("fill", objects.ground.body:getWorldPoints(objects.ground.shape:getPoints()))
  
    love.graphics.setColor(1.0, 1.0, 1.0)
    love.graphics.draw(objects.tower.image, objects.tower.body:getX(), objects.tower.body:getY() )


end

function gameLevel03:beginContact()
    local x, y = objects.tower.body:getLinearVelocity()
    print (y)
    print(objects.tower.body:getAngularVelocity())
    if y < 100 then
        print"landed"
    else 
        print"Boom!"
    end
end

function gameLevel03:keypressed(key, scancode, isrepeat)
    if key == "s" then
        score = score + 1
        print(score)
    end
    if key == "escape" then
        love.event.quit()
    elseif key == "space" then
        love.event.quit()
    end
end


return gameLevel03