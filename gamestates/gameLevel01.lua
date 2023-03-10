--! file: gameLevel01.lua

--Launch

Gamestate = require 'libs.hump.gamestate'

gameLevel01 = {}

function gameLevel01:enter()
    love.graphics.reset()
    launch = false
    timer = 1
    bgAlpha = 0
    bgFadein = 1
    countDown = 10
    flow = 0
    towerScaling = 3


    objects.ground.body = love.physics.newBody(earth, system.winWidth/2, system.winHeight * 0.95)
    objects.ground.shape = love.physics.newRectangleShape(system.winWidth, system.winHeight * 0.25)
    objects.ground.fixture = love.physics.newFixture(objects.ground.body, objects.ground.shape)

    objects.tower.body = love.physics.newBody(earth, system.winWidth * 0.681, system.winHeight * 0.6, "dynamic")
    objects.tower.shape = love.physics.newRectangleShape(objects.tower.width / 2 * system.scaling, objects.tower.height / 2 * system.scaling, objects.tower.width * towerScaling * system.scaling, objects.tower.height * towerScaling * system.scaling, 0)    
    objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)

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
            objects.tower.body:applyForce(0, -28000)
            --objects.tower.body:setY(objects.tower.body:getY() - 256 * dt)
        end
        if system.score01 > 0 then
            system.score01 = system.score01 - 200 * dt
        else 
            system.score01 = 0
        end
        objects.image.fireball.currentFrame = objects.image.fireball.currentFrame + 10 * dt
        if objects.image.fireball.currentFrame >= 4 then
            objects.image.fireball.currentFrame = 1
        end
        love.system.vibrate(0.2)
    end

    if launch and system.score01 <= 0 and objects.tower.body:getY() > 266 then
        launch = false
        system.score01 = 0
        system.crashed = true
    end
    
    if system.level01over then
        Gamestate.switch(gameLevelGoal01)
    end


    
    if countDown > 0 and system.crashed == false then
        countDown = countDown - 1 * dt
        if countDown < 0 then
            countDown = 0
        end
        if math.floor(countDown) == 9 then
            playSound(objects.audio.countdown,'play',false)
        end
        flow = flow - (flow * 3.2) * dt * (system.level * 0.8)
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
    love.graphics.setColor(1.0, 1.0, 1.0, bgAlpha) -- draw Backgroud
    love.graphics.draw(objects.ground.background01, 0, 0, 0,  system.scaling, (system.winHeight / objects.ground.background01Height))
    love.graphics.setColor(1.0, 1.0, 1.0, 1)

    love.graphics.setColor(0.149, 0.361, 0.259, 0.4)
    love.graphics.polygon("fill", objects.ground.body:getWorldPoints(objects.ground.shape:getPoints()))
    if debugMode then
        love.graphics.setColor(1, 1, 1, 1)
        love.graphics.polygon("line", objects.ground.body:getWorldPoints(objects.ground.shape:getPoints()))
    end
  
    love.graphics.setColor(1.0, 1.0, 1.0, bgAlpha)  
    love.graphics.draw(objects.tower.image, objects.tower.body:getX(), objects.tower.body:getY(), 
        objects.tower.body:getAngle(), system.scaling * towerScaling, system.scaling * towerScaling,
        objects.tower.width / towerScaling, objects.tower.height/towerScaling) -- Draw Tower
    if debugMode then
        love.graphics.polygon("line", objects.tower.body:getWorldPoints(objects.tower.shape:getPoints()))
    end

    love.graphics.setColor(1.0, 0.9, 0.9, bgAlpha)  -- Draw Bar 
    love.graphics.rectangle("line", system.winWidth * 0.1, system.winHeight * 0.84, system.winWidth * 0.8, system.winHeight * 0.1)
    love.graphics.setColor(1.0, 0.1, 0.1, bgAlpha)
    love.graphics.rectangle("fill", system.winWidth * 0.1, system.winHeight * 0.84,  math.min(flow * system.winWidth * 0.1, system.winWidth * 0.8), system.winHeight * 0.1)

    love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)  -- Draw Score
    love.graphics.setFont(scoreFont)
    love.graphics.print(math.floor(system.score01), system.winWidth * 0.1, system.winHeight * 0.1, 0, system.scaling,  system.scaling)

    love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)  -- Draw CountDown
    love.graphics.setFont(countDownFont)
    love.graphics.print(math.floor(countDown), system.winWidth * 0.1, system.winHeight * 0.65, 0,  system.scaling,  system.scaling)

    if countDown == 0 and system.crashed == false then
        love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)  -- Draw Launch
        love.graphics.setFont(screenFont)
        love.graphics.print("Launch!", system.winWidth * 0.1, system.winHeight * 0.3, 0,  system.scaling,  system.scaling)
    end

    if countDown == 0 and system.crashed == true then
        love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)  -- Draw Crashed
        love.graphics.setFont(screenFont)
        love.graphics.print("Crashed!\n\nPress to play agian.", system.winWidth * 0.1, system.winHeight * 0.3, 0, system.scaling,  system.scaling)
    end

    if launch then 
        love.graphics.setColor(1.0, 1.0, 1.0, 0.98)
        love.graphics.draw(objects.image.fireball.tex, objects.image.fireball.frames[math.floor(objects.image.fireball.currentFrame)], 
        objects.tower.body:getX() - objects.tower.width /2 * towerScaling * system.scaling, objects.tower.body:getY() + objects.tower.height /2 * towerScaling * system.scaling,
         0,
         towerScaling * system.scaling, towerScaling * system.scaling
         )
    end
end

function gameLevel01:keypressed(key, scancode, isrepeat)
     if key == "escape" then
        love.event.quit()
    end
    if love.keyboard.isDown("right") and system.launch == "left" then
        flow = flow + love.math.random(8, 16) / 10
        system.launch = "right"
    elseif love.keyboard.isDown("left") and system.launch == "right" then
        flow = flow + love.math.random(8, 16) / 10
        system.launch = "left"
    end
    if key == "m" and objects.audio.mainTheme:isPlaying() then
        playSound(objects.audio.mainTheme,'pause',false) -- pause BG music
    elseif key == "m" then
        playSound(objects.audio.mainTheme,'play',false) -- unpause BG music
    end
    if  key == "space" and countDown == 0 and system.crashed == true then
        reset()
        system.score01 = 0
        Gamestate.switch(gameIntro02)
    end
    
end

function gameLevel01:mousepressed(x, y, istouch)
    if x < system.winWidth * 0.3  and system.launch == "left" then
      flow = flow + love.math.random(8, 16) / 10
      system.launch = "right"
    elseif x > system.winWidth * 0.7  and system.launch == "right" then
      flow = flow + love.math.random(8, 16) / 10
      system.launch = "left"
    end
    if countDown == 0 and system.crashed == true then
        reset()
        system.score01 = 0
        Gamestate.switch(gameIntro02)
    end
end


function gameLevel01:leave()
    bodies = earth:getBodies()
    for i, body in ipairs(bodies) do
        body:destroy()
         --print("BOOOOOM")
    end

end

return gameLevel01