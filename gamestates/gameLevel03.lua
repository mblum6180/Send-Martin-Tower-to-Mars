--! file: stage03.lua

Gamestate = require 'libs.hump.gamestate'

local gameLevel03 = {}
function gameLevel03:init()
    vertices  = {000,000, 001,002, 100,100, 200,200, 300,200, 400,300, 500,30, system.winWidth,150, system.winWidth,300, 0,300}
    background = love.graphics.newImage("assets/marsmountain8bit.png") -- Background image

    mars:setCallbacks(beginContact, endContact, preSolve, postSolve)

    love.graphics.setBackgroundColor(0.92, 0.70, 0.60)
    objects.ground.body = love.physics.newBody(mars, 0, love.graphics.getPixelHeight() * 0.5)
    --objects.ground.shape = love.physics.newChainShape(true, 0, 0, 150, 150, system.winWidth, 150, system.winWidth, 300, 0, 300)
    objects.ground.shape = love.physics.newChainShape(true, vertices, "static")
    objects.ground.fixture = love.physics.newFixture(objects.ground.body, objects.ground.shape)

    objects.tower.body = love.physics.newBody(mars, love.math.random(0, system.winWidth), 0, "dynamic")
    objects.tower.shape = love.physics.newRectangleShape(objects.tower.width / 2, objects.tower.height / 2, objects.tower.width, objects.tower.height, 0)
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
        objects.tower.body:applyForce(objects.tower.strengthMain * math.cos(objects.tower.body:getAngle() - 1.57), objects.tower.strengthMain * math.sin(objects.tower.body:getAngle() - 1.57))
    end
    if love.keyboard.isDown("a") then
        objects.tower.body:applyForce(objects.tower.strengthSide * math.cos(objects.tower.body:getAngle() + 3.14), objects.tower.strengthSide * math.sin(objects.tower.body:getAngle() + 3.14))
    elseif love.keyboard.isDown("d") then
        objects.tower.body:applyForce(objects.tower.strengthSide * math.cos(objects.tower.body:getAngle() + 0), objects.tower.strengthSide * math.sin(objects.tower.body:getAngle() + 0))
    end
    edge(objects.tower.body:getX(), objects.tower.body:getY())

    
end
  
function gameLevel03:draw()
    love.graphics.setColor(system.BGcolorR, system.BGcolorG, system.BGcolorB)
    love.graphics.draw(background, 0, 0)
    love.graphics.setColor(0.53, 0.39, 0.32)
    --love.graphics.polygon("line", vertices)
    love.graphics.polygon("line", objects.ground.body:getWorldPoints(objects.ground.shape:getPoints()))
    love.graphics.setColor(1.0, 1.0, 1.0)
    love.graphics.draw(objects.tower.image, objects.tower.body:getX(), objects.tower.body:getY(), objects.tower.body:getAngle() )
    love.graphics.polygon("line", objects.tower.body:getWorldPoints(objects.tower.shape:getPoints()))

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
            gameLevel03:crash()
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

function gameLevel03:crash()
    --if objects.audio.crash then
    --    objects.audio.crash:rewind()
    --end
    -- objects.audio.crash:play()
    playSound(objects.audio.crash)
end


return gameLevel03