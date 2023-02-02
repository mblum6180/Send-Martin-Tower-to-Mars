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
    countDown = 10
    flow = 0


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

    if launch and system.crashed == false then 
        playSound(objects.audio.launch, "play")
        if system.score01 > 0 then
            objects.tower.body:applyForce(0, -31000)
        end
        if system.score01 > 0 then
            system.score01 = system.score01 - 351 * dt
        else 
            system.score01 = 0
        end
        objects.image.fireball.currentFrame = objects.image.fireball.currentFrame + 10 * dt
        if objects.image.fireball.currentFrame >= 4 then
            objects.image.fireball.currentFrame = 1
        end
    end

    if launch and system.score01 <= 0 and objects.tower.body:getY() > 266 then
        launch = false
        system.score01 = 0
        system.crashed = true
    end
    
    if system.level01over then
        Gamestate.switch(gameLevelGoal01)
    end
    if debugMode then
        print(objects.tower.body:getLinearVelocity())
    end

    
    if countDown > 0 and system.crashed == false then
        countDown = countDown - 1 * dt
        if countDown < 0 then
            countDown = 0
        end
        if math.floor(countDown) == 9 then
            playSound(objects.audio.countdown,'play',false)
        end
        print(countDown)
        flow = flow - (flow * 3.2) * dt --set to 1.2 
        system.score01 = system.score01 + flow
        
    elseif system.crashed == false then
        launch = true
        flow = 10
        countDown = 0
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


    love.graphics.setColor(1.0, 0.9, 0.9, bgAlpha)  -- Draw Bar 
    love.graphics.rectangle("line", system.winWidth * 0.1, system.winHeight * 0.84, 750, 45)
    love.graphics.setColor(1.0, 0.1, 0.1, bgAlpha)
    love.graphics.rectangle("fill", system.winWidth * 0.1, system.winHeight * 0.84, flow*75, 45)

    love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)  -- Draw Score
    love.graphics.print(math.floor(system.score01), system.winWidth * 0.1, system.winHeight * 0.1, 0, system.winWidth / 150, system.winWidth / 150)

    love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)  -- Draw CountDown
    love.graphics.print(math.floor(countDown), system.winWidth * 0.1, system.winHeight * 0.65, 0, system.winWidth / 150, system.winWidth / 150)

    if countDown == 0 and system.crashed == false then
        love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)  -- Draw Launch
        love.graphics.print("Launch!", system.winWidth * 0.1, system.winHeight * 0.3, 0, system.winWidth / 99, system.winWidth / 99)
    end

    if countDown == 0 and system.crashed == true then
        love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)  -- Draw Crashed
        love.graphics.print("Crashed!", system.winWidth * 0.1, system.winHeight * 0.3, 0, system.winWidth / 99, system.winWidth / 99)
    end

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
    end
    if love.keyboard.isDown("right") and system.launch == "left" then
        flow = flow + love.math.random(0.8, 1.1)
        system.launch = "right"
    elseif love.keyboard.isDown("left") and system.launch == "right" then
        flow = flow + love.math.random(0.8, 1.2)
        system.launch = "left"
    end


end

return gameLevel01