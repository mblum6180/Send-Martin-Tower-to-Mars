--! file: stage03.lua

Gamestate = require 'libs.hump.gamestate'

gameLevel03 = {}
function gameLevel03:enter()
    love.graphics.reset()
    objects.audio.fire:setVolume(1.0)
    system.score03 = system.score02
    landingSpeed = 0
    objects.ground.landscape = {}
    objects.ground.landscape = gameLevel03:genLandscape()

    mars:setCallbacks(beginContact, endContact, preSolve, postSolve)

    love.graphics.setBackgroundColor(0.92, 0.70, 0.60)
    objects.ground.body = love.physics.newBody(mars, 0, system.winHeight * 0.5)
    objects.ground.shape = love.physics.newChainShape(true, objects.ground.landscape, "static")
    objects.ground.fixture = love.physics.newFixture(objects.ground.body, objects.ground.shape)

    objects.ground.bodyBox = love.physics.newBody(mars, 0, 0)
    objects.ground.shapeBox = love.physics.newChainShape(true, objects.ground.box, "static")
    objects.ground.fixtureBox = love.physics.newFixture(objects.ground.bodyBox, objects.ground.shapeBox)


    objects.tower.body = love.physics.newBody(mars, love.math.random(150, system.winWidth - 150), 0, "dynamic")
    objects.tower.body:setLinearDamping(0.6)
    objects.tower.shape = love.physics.newRectangleShape(objects.tower.width / 2 * system.scaling, objects.tower.height / 2 * system.scaling,
         objects.tower.width * system.scaling, objects.tower.height * system.scaling, 0)
    objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)
    objects.tower.fixture:setRestitution(0.3) 
    objects.tower.fixture:setFriction(0.98)
    objects.tower.fixture:setUserData(1)


    bgFill = {objects.ground.body:getWorldPoints(objects.ground.shape:getPoints())}
    for i = #bgFill, #bgFill-1, -1 do
        bgFill[i] = nul
    end

end

function gameLevel03:update(dt)
    mars:update(dt) 
    if objects.tower.crashed == false and objects.tower.empty == false then
        gameLevel03:input()
    end
    edge(objects.tower.body:getX(), objects.tower.body:getY())

    
    
    if system.score03 <= 0 then 
        system.score03 = 0
        objects.tower.empty = true
        objects.tower.fire = false
    end
    if system.landedTimer == 0 then 
        objects.tower.empty = true
    end

    if system.winner == false then
        if system.landed then 
            if system.landedTimer <= 0 then 
                system.landedTimer = 0
            else 
            system.landedTimer = system.landedTimer - 1 * dt
            end
            if system.landedTimer == 0 then
                system.winner = true
                if system.winner == true and objects.tower.crashed == false then
                    playSound(objects.audio.landed,'play',false)
                end
                system.bonus = math.abs(math.deg(objects.tower.body:getAngle())) --calculate bonus
                if system.bonus >= 45 then
                    system.bonus = 0
                elseif system.bonus < 45 then
                    system.bonus = (44 - system.bonus) * (100 - landingSpeed)
                end

                --print(landingSpeed)

            end
        end
        if objects.tower.fire then -- Fire
            if objects.tower.empty == false then
                system.score03 = system.score03 - 200 * dt
                    objects.image.fireball.currentFrame = objects.image.fireball.currentFrame + 25 * dt
                if objects.image.fireball.currentFrame >= 4 then
                    objects.image.fireball.currentFrame = 1
                end
                playSound(objects.audio.fire,'play',false)
            end
        else 
            playSound(objects.audio.fire,'pause',false)
        end
    end
    
end
  
function gameLevel03:draw()
    love.graphics.setColor(system.BGcolorR, system.BGcolorG, system.BGcolorB) -- draw Background
    love.graphics.draw(objects.ground.background03, 0, 0, 0,  system.scaling, (system.winHeight / objects.ground.background03Height)) -- set scaling


    love.graphics.setColor(0.53, 0.39, 0.32)
    love.graphics.polygon("line", objects.ground.body:getWorldPoints(objects.ground.shape:getPoints()))
    
    
    local triangles = love.math.triangulate(bgFill) --  Draw Mountains 
    for i, triangle in ipairs(triangles) do 
        love.graphics.polygon("fill", triangle)
    end

    love.graphics.setColor(1.0, 1.0, 1.0) -- Draw Tower
    love.graphics.draw(objects.tower.image, objects.tower.body:getX(), objects.tower.body:getY(), objects.tower.body:getAngle(), system.scaling, system.scaling, 0, 0 )
    if debugMode then
        love.graphics.polygon("line", objects.tower.body:getWorldPoints(objects.tower.shape:getPoints()))
    end
    if objects.tower.fire then -- draw fireball
        love.graphics.setColor(1.0, 1.0, 1.0, 0.7)
        love.graphics.draw(objects.image.fireball.tex, objects.image.fireball.frames[math.floor(objects.image.fireball.currentFrame)], 
        objects.tower.body:getX(), objects.tower.body:getY(), objects.tower.body:getAngle(),
        system.scaling, system.scaling,
        15,-120)
    end

    love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)   -- Print Score
    love.graphics.setFont(scoreFont)
    love.graphics.print(math.floor(system.score03), system.winWidth * 0.1, system.winHeight * 0.1, 0)

    if system.winner == true and objects.tower.crashed == false then
        love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)  -- Draw Winner
        love.graphics.setFont(screenFont)
        love.graphics.print("Landed!", system.winWidth * 0.1, system.winHeight * 0.3, 0)
        love.graphics.print("Landing Bonus "..math.ceil(system.bonus), system.winWidth * 0.1, system.winHeight * 0.55, 0)

    end
    if system.winner == true and objects.tower.crashed == true then
        love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)  -- Draw Crashed
        love.graphics.setFont(screenFont)
        love.graphics.print("Crashed!", system.winWidth * 0.1, system.winHeight * 0.3, 0)
    
    end
    love.graphics.setColor(0.63, 0.49, 0.42) --   Draw Foreground Mountains --must be last
    love.graphics.translate(0, system.winHeight * 0.04)
    for i, triangle in ipairs(triangles) do 
        love.graphics.polygon("fill", triangle)
    end



end

function gameLevel03:genLandscape()  --Height Width

    local ground = {000,000, 001,love.math.random(system.winHeight * 0.1,system.winHeight * 0.3),} --generate random landscape


    local slices = love.math.random(18,30)
    for i = 1, slices, 1 do
        x = i * love.math.random(system.winWidth / slices -1, system.winWidth / slices)
        table.insert(ground, x)

        local platform = love.math.random(1, 100) --creates level platforms to land on
        if platform < 10 then
            y = ground[#ground - 1], 500

        else
            y = love.math.random(ground[#ground - 1] - love.math.random(0,system.winHeight * 0.1), system.winHeight * 0.32) -- v4 The first value is the last element in the array "ground" minus a random number between 0 and 100. The second value is 300. The random number generated will be stored in the variable "y".
        end
            

        table.insert(ground, y)
    end


    local groundReturn = {system.winWidth,love.math.random(0, system.winHeight)/2, system.winWidth,system.winHeight, 000,system.winHeight}
    for i,v in ipairs(groundReturn) do
        table.insert(ground, v)
    end

 
    return ground
end



function gameLevel03:beginContact(obj1,obj2)
    local x, y = objects.tower.body:getLinearVelocity()
    --print (y)
    --print(objects.tower.body:getAngularVelocity())
    if landingSpeed == nil then
        landingSpeed = y
        --print(landingSpeed)
    end
    if y < 100 then
        gameLevel03:landed()

    else 
        --print"Boom!"
        gameLevel03:crash()

    end
end

function gameLevel03:endContact(obj1,obj2)
    --print("not on Ground")
    system.landed = false
end

function gameLevel03:keypressed(key, scancode, isrepeat)
    if debugMode then
        if key == "s" then
        print("s")
        end
        if key == 'i' then
            print(objects.tower.fixture:getUserData())
        end
    end
    if key == "escape" then
        love.event.quit()
    elseif key == "space" or key =="left" or key =="right" or key =="down" or key =="up" then
        if system.winner == true then
        --print(system.winner)
        Gamestate.switch(gameLevelGoal03)
        end
    end
    if key == "m" and objects.audio.mainTheme:isPlaying() then
        playSound(objects.audio.mainTheme,'pause',false) -- pause BG music
    elseif key == "m" then
        playSound(objects.audio.mainTheme,'play',false) -- unpause BG music
    end
end

function gameLevel03:input()
    if love.keyboard.isDown("right") then
        objects.tower.body:applyTorque(objects.tower.strengthTorque)
    elseif love.keyboard.isDown("left") then
        objects.tower.body:applyTorque(-objects.tower.strengthTorque)
    end
    if love.keyboard.isDown("up") or system.moveGas then
        objects.tower.body:applyForce(objects.tower.strengthMain * math.cos(objects.tower.body:getAngle() - 1.57), objects.tower.strengthMain * math.sin(objects.tower.body:getAngle() - 1.57))
        objects.tower.fire = true
    else objects.tower.fire = false
        objects.tower.body:applyForce(objects.tower.strengthMain / 10 * math.cos(objects.tower.body:getAngle() - 1.57), objects.tower.strengthMain / 10 * math.sin(objects.tower.body:getAngle() - 1.57))
    end
    if love.keyboard.isDown("a") or system.moveLeft then
        objects.tower.body:applyForce(objects.tower.strengthSide * math.cos(objects.tower.body:getAngle() + 3.14), objects.tower.strengthSide * math.sin(objects.tower.body:getAngle() + 3.14))
    elseif love.keyboard.isDown("d") or system.moveRight then
        objects.tower.body:applyForce(objects.tower.strengthSide * math.cos(objects.tower.body:getAngle() + 0), objects.tower.strengthSide * math.sin(objects.tower.body:getAngle() + 0))
    end
end

function gameLevel03:touchpressed(id, x, y, dx, dy, pressure)
    print(id, x, y, pressure)
    if x < system.winWidth * 0.3 then
        system.moveLeft = true
    elseif x > system.winWidth * 0.7 then
        system.moveRight = true
    else
        system.moveGas = true
    end
    if system.winner == true then
        --print(system.winner)
        Gamestate.switch(gameLevelGoal03)
    end
end

function gameLevel03:touchreleased(id, x, y, pressure)
    if x < system.winWidth * 0.3 then
        system.moveLeft = false
    elseif x > system.winWidth * 0.7 then
        system.moveRight = false
    else
        system.moveGas = false
    end
end

function gameLevel03:crash()
    if objects.tower.crashed == false then
        system.score03 = system.score03 - 5000
    end
    objects.tower.fire = false
    objects.tower.crashed = true
    playSound(objects.audio.crash,'stop')
    playSound(objects.audio.crash,'play', true)
end

function gameLevel03:landed()
    --print("landed")
    system.landed = true

end

function gameLevel03:leave()
    print("hello")
    bodies = mars:getBodies()
    for i, body in ipairs(bodies) do
        body:destroy()
         --print("BOOOOOM")
    end
end

return gameLevel03