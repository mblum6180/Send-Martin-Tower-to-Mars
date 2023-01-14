--! file: stage02.lua

Gamestate = require 'libs.hump.gamestate'

local gameLevel02 = {}
function gameLevel02:init()
    background = love.graphics.newImage("assets/spaceBG.png") -- Background image

    space:setCallbacks(beginContact, endContact, preSolve, postSolve)

    love.graphics.setBackgroundColor(0.92, 0.70, 0.60)


    objects.ground.body = love.physics.newBody(space, system.winWidth/2, system.winHeight)
    objects.ground.shape = love.physics.newRectangleShape(system.winWidth, 150)
    objects.ground.fixture = love.physics.newFixture(objects.ground.body, objects.ground.shape)

    objects.tower.body = love.physics.newBody(space, love.math.random(system.winWidth * 0.2, system.winWidth * 0.8), system.winHeight * 0.9, "dynamic")
    objects.tower.shape = love.physics.newRectangleShape(objects.tower.width, objects.tower.height + 75)
    objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)
    objects.tower.fixture:setRestitution(0.3) 
    objects.tower.fixture:setFriction(0.98)

end

function gameLevel02:update(dt)
    space:update(dt) 
    if coolDown > 0 then
        coolDown = coolDown - 1 * dt
    end
    print (coolDown)
    system.BGScale = system.BGScale + 0.005 * dt
    if objects.tower.body:getY() < 0 then
        --objects.tower.fixture:destroy()
        system.level02over = true
    end


    if system.level02over then
        Gamestate.switch(gameLevel03)
    end

    if love.keyboard.isDown("right") then
      objects.tower.body:applyForce(400, 0)
    elseif love.keyboard.isDown("left") then
      objects.tower.body:applyForce(-400, 0)
    end

    if not objects.fire.body  then
        if (love.keyboard.isDown("up") and coolDown <= 0)then
            objects.fire.body = love.physics.newBody(space, objects.tower.body:getX() + 15, objects.tower.body:getY() - 50, "dynamic")
            objects.fire.shape = love.physics.newRectangleShape(objects.fire.width, objects.fire.height)
            objects.fire.fixture = love.physics.newFixture(objects.fire.body, objects.fire.shape, 1)
            objects.fire.body:applyForce(0, -30000)
            coolDown = 1
        end
    elseif objects.fire.body:getY() < 0 then
        objects.fire.body = null
    end
    edge(objects.tower.body:getX(), objects.tower.body:getY())


end
  
function gameLevel02:draw()
    love.graphics.setColor(system.BGcolorR, system.BGcolorG, system.BGcolorB)
    love.graphics.draw(background, 0, 0, 0, system.BGScale)
    --love.graphics.setColor(0.53, 0.39, 0.32)
    --love.graphics.polygon("fill", objects.ground.body:getWorldPoints(objects.ground.shape:getPoints()))
  
    love.graphics.setColor(1.0, 1.0, 1.0)
    love.graphics.draw(objects.tower.image, objects.tower.body:getX(), objects.tower.body:getY() )
    if objects.fire.body then
        love.graphics.draw(objects.fire.image, objects.fire.body:getX(), objects.fire.body:getY(), 0, 3, 3)
    end

end

function gameLevel02:beginContact()
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

function gameLevel02:keypressed(key, scancode, isrepeat)
    if debugMode then
        if key == "s" then
        score = score + 1
        print(score)
        end
    end
    if key == "escape" then
        love.event.quit()
    --elseif key == "space" then
    --    Gamestate.switch(gameLevel03)
    end
end


return gameLevel02