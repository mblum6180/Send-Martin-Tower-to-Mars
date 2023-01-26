--! file: gameLevel01.lua

--Launch

Gamestate = require 'libs.hump.gamestate'

local gameLevel01 = {}

function gameLevel01:init()
    love.graphics.reset()
    launch = false
    timer = 1
    bgAlpha = 0
    bgFadein = 1
    countDown = 0


    objects.ground.body = love.physics.newBody(earth, system.winWidth/2, love.graphics.getPixelHeight() - 42)
    objects.ground.shape = love.physics.newRectangleShape(system.winWidth, 150)
    objects.ground.fixture = love.physics.newFixture(objects.ground.body, objects.ground.shape)

    objects.tower.body = love.physics.newBody(earth, love.math.random(0, system.winWidth), 0, "dynamic")
    objects.tower.shape = love.physics.newRectangleShape(objects.tower.width * 3, (objects.tower.height * 3) + 285) 
    objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)

    objects.tower.body:setX(system.winWidth * 0.69)
    objects.tower.body:setY(system.winHeight * 0.7)
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
        objects.tower.body:applyForce(0, -29000)
        objects.image.fireball.currentFrame = objects.image.fireball.currentFrame + 10 * dt
        if objects.image.fireball.currentFrame >= 4 then
            objects.image.fireball.currentFrame = 1
        end
    end

    if system.level01over then
        Gamestate.switch(gameLevelGoal01)
    end
    if debugMode then
        print(objects.tower.body:getLinearVelocity())
    end

    if countDown >= 10 then
        launch = true
        countDown = 10
    else
        countDown = countDown - (countDown * 0.2) * dt --set to 1.2 
        system.score01 = system.score01 - 200 * dt
    end



end
    
function gameLevel01:draw()
    if launch then
        love.graphics.translate(love.math.random(-5,5), love.math.random(-5,5))
    end
    love.graphics.setColor(1.0, 1.0, 1.0, bgAlpha)
    love.graphics.draw(objects.ground.background01, 0, 0)
    love.graphics.setColor(1.0, 1.0, 1.0, 1)

    love.graphics.setColor(0.149, 0.361, 0.259, 0.4)
    love.graphics.polygon("fill", objects.ground.body:getWorldPoints(objects.ground.shape:getPoints()))
  
    love.graphics.setColor(1.0, 1.0, 1.0, bgAlpha)  
    love.graphics.draw(objects.tower.image, objects.tower.body:getX(), objects.tower.body:getY(), 0, 3, 3)


    love.graphics.setColor(1.0, 0.9, 0.9, bgAlpha)
    love.graphics.rectangle("line", system.winWidth * 0.1, system.winHeight * 0.84, 750, 45)
    love.graphics.setColor(1.0, 0.1, 0.1, bgAlpha)
    love.graphics.rectangle("fill", system.winWidth * 0.1, system.winHeight * 0.84, countDown*75, 45)

    love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)
    love.graphics.print(math.floor(system.score01), system.winWidth * 0.1, system.winHeight * 0.1, 0, system.winWidth / 150, system.winWidth / 150)

    if launch then 
        love.graphics.setColor(1.0, 1.0, 1.0, bgAlpha)
        love.graphics.draw(objects.image.fireball.tex, objects.image.fireball.frames[math.floor(objects.image.fireball.currentFrame)], 
        objects.tower.body:getX() - objects.tower.width/1.5, objects.tower.body:getY() + objects.tower.height * 2.8, 0, 3, 3)
    end
end

function gameLevel01:keypressed(key, scancode, isrepeat)
    if debugMode then
        if key == "s" then
        system.score = system.score + 1
        print(system.score)
        end
    end
    if key == "escape" then
        love.event.quit()
    elseif key == "space" then
        countDown = countDown + love.math.random(0.3, 1.2)
    end

end

return gameLevel01