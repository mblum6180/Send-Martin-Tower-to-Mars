--! file: gameLevel01.lua

--Launch

Gamestate = require 'libs.hump.gamestate'

local gameLevel01 = {}

function gameLevel01:init()
    launch = false
    timer = 1
    bgAlpha = 0
    bgFadein = 1
    background = love.graphics.newImage("assets/CityBG8bit.png")

    objects.ground.body = love.physics.newBody(earth, system.winWidth/2, love.graphics.getPixelHeight() - 42)
    objects.ground.shape = love.physics.newRectangleShape(system.winWidth, 150)
    objects.ground.fixture = love.physics.newFixture(objects.ground.body, objects.ground.shape)

    objects.tower.body = love.physics.newBody(earth, love.math.random(0, system.winWidth), 0, "dynamic")
    objects.tower.shape = love.physics.newRectangleShape(objects.tower.width * 3, (objects.tower.height * 3) + 285) 
    objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)

    objects.tower.body:setX(system.winWidth/10)
    objects.tower.body:setY(system.winHeight/2.7)
end
    
    
function gameLevel01:update(dt)
    earth:update(dt) 
    timer = timer + dt
    if bgAlpha ~= 1 then
        if timer > bgFadein then 
            bgAlpha = fade(dt, bgAlpha, 0.9)
        end
    end

    if objects.tower.body:getY() < -375 then
        --objects.tower.fixture:destroy()
        system.level01over = true
    end

    if launch then 
        objects.tower.body:applyForce(0, -35000)
    end

    if system.level01over then
        Gamestate.switch(gameLevel02)
    end
    if debugMode then
        print(objects.tower.body:getLinearVelocity())
    end


end
    
function gameLevel01:draw()
    love.graphics.setColor(1.0, 1.0, 1.0, bgAlpha)
    love.graphics.draw(background, 20, 0)
    love.graphics.setColor(1.0, 1.0, 1.0, 1)

    love.graphics.setColor(0.149, 0.361, 0.259, 0.4)
    love.graphics.polygon("fill", objects.ground.body:getWorldPoints(objects.ground.shape:getPoints()))
  
    love.graphics.setColor(1.0, 1.0, 1.0, bgAlpha)  
    love.graphics.draw(objects.tower.image, objects.tower.body:getX(), objects.tower.body:getY(), 0, 3, 3)

end

function gameLevel01:keypressed(key, scancode, isrepeat)
    if debugMode then
        if key == "s" then
        score = score + 1
        print(score)
        end
    end
    if key == "escape" then
        love.event.quit()
    elseif key == "space" then
        launch = true
    end
end

return gameLevel01