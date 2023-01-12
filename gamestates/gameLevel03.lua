--! file: stage03.lua

Gamestate = require 'libs.hump.gamestate'

local gameLevel03 = {}
function gameLevel03:init()
    background = love.graphics.newImage("assets/marsmountain8bit.png") -- Background image

    mars:setCallbacks(beginContact, endContact, preSolve, postSolve)

    love.graphics.setBackgroundColor(0.92, 0.70, 0.60)
    objects.ground.body = love.physics.newBody(mars, system.winWidth/2, love.graphics.getPixelHeight() - 75)
    objects.ground.shape = love.physics.newRectangleShape(system.winWidth, 150)
    objects.ground.fixture = love.physics.newFixture(objects.ground.body, objects.ground.shape)

    objects.tower.body = love.physics.newBody(mars, love.math.random(0, system.winWidth), 0, "dynamic")
    objects.tower.shape = love.physics.newRectangleShape(objects.tower.width, objects.tower.height + 75)
    objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)
    objects.tower.fixture:setRestitution(0.3) 
    objects.tower.fixture:setFriction(0.98)

end

function gameLevel03:update(dt)
    mars:update(dt) 

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
    if debugMode then
        print (y)
        print(objects.tower.body:getAngularVelocity())
        if y < 100 then
            print"landed"
        else 
            print"Boom!"
        end
    end
end

function gameLevel03:keypressed(key, scancode, isrepeat)
    if debugMode then
        if key == "s" then
        score = score + 1
        print(score)
        end
    end
    if key == "escape" then
        love.event.quit()
    elseif key == "space" then
        love.event.quit()
    end
end


return gameLevel03