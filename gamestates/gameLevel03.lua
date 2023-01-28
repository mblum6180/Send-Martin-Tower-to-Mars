--! file: stage03.lua

Gamestate = require 'libs.hump.gamestate'

local gameLevel03 = {}
function gameLevel03:init()
    love.graphics.reset()
    objects.audio.fire:setVolume(1.0)
    system.score03 = system.score02
    bonus = 9000
    landingSpeed = nil

    objects.ground.landscape = gameLevel03:genLandscape()

    mars:setCallbacks(beginContact, endContact, preSolve, postSolve)

    love.graphics.setBackgroundColor(0.92, 0.70, 0.60)
    objects.ground.body = love.physics.newBody(mars, 0, love.graphics.getPixelHeight() * 0.5)
    objects.ground.shape = love.physics.newChainShape(true, objects.ground.landscape, "static")
    objects.ground.fixture = love.physics.newFixture(objects.ground.body, objects.ground.shape)

    objects.ground.bodyBox = love.physics.newBody(mars, 0, 0)
    objects.ground.shapeBox = love.physics.newChainShape(true, objects.ground.box, "static")
    objects.ground.fixtureBox = love.physics.newFixture(objects.ground.bodyBox, objects.ground.shapeBox)


    objects.tower.body = love.physics.newBody(mars, love.math.random(150, system.winWidth - 150), 0, "dynamic")
    objects.tower.body:setLinearDamping(0.6)
    objects.tower.shape = love.physics.newRectangleShape(objects.tower.width / 2, objects.tower.height / 2, objects.tower.width, objects.tower.height, 0)
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
    if objects.tower.crashed == false then
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

    if objects.tower.crashed == false and system.winner == false then
        if system.landed then 
            if system.landedTimer <= 0 then 
                system.landedTimer = 0
            else 
            system.landedTimer = system.landedTimer - 1 * dt
            end
            if system.landedTimer == 0 then
                system.winner = true
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
    love.graphics.setColor(system.BGcolorR, system.BGcolorG, system.BGcolorB)
    love.graphics.draw(objects.ground.background03, 0, 0)
    love.graphics.setColor(0.53, 0.39, 0.32)
    love.graphics.polygon("line", objects.ground.body:getWorldPoints(objects.ground.shape:getPoints()))
    
    
    local triangles = love.math.triangulate(bgFill) --  Draw Mountains 
    for i, triangle in ipairs(triangles) do 
        love.graphics.polygon("fill", triangle)
    end

    love.graphics.setColor(1.0, 1.0, 1.0) -- Draw Tower
    love.graphics.draw(objects.tower.image, objects.tower.body:getX(), objects.tower.body:getY(), objects.tower.body:getAngle() )
    if debugMode then
        love.graphics.polygon("line", objects.tower.body:getWorldPoints(objects.tower.shape:getPoints()))
    end
    if objects.tower.fire then -- draw fireball
        love.graphics.setColor(1.0, 1.0, 1.0, bgAlpha)
        love.graphics.draw(objects.image.fireball.tex, objects.image.fireball.frames[math.floor(objects.image.fireball.currentFrame)], 
        objects.tower.body:getX(), objects.tower.body:getY(), objects.tower.body:getAngle(),
        1,1,
        15,-120)
    end

    love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)   -- Print Score
    love.graphics.print(math.floor(system.score03), system.winWidth * 0.1, system.winHeight * 0.1, 0, system.winWidth / 150, system.winWidth / 150)

    if system.winner == true then
        love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)  -- Draw Winner
        love.graphics.print("Landed!", system.winWidth * 0.1, system.winHeight * 0.3, 0, system.winWidth / 99, system.winWidth / 99)
        love.graphics.print("Landing Bonus "..bonus, system.winWidth * 0.1, system.winHeight * 0.55, 0, system.winWidth / 200, system.winWidth / 200)

    end
    love.graphics.setColor(0.63, 0.49, 0.42) --   Draw Foreground Mountains --must be last
    love.graphics.translate(0, system.winHeight * 0.04)
    for i, triangle in ipairs(triangles) do 
        love.graphics.polygon("fill", triangle)
    end



end

function gameLevel03:genLandscape()

    local ground = {000,000, 001,200} --generate random landscape


    local slices = love.math.random(18,30)
    for i = 1, slices, 1 do
        x = i * love.math.random(system.winWidth / slices -1, system.winWidth / slices)
        table.insert(ground, x)

        local platform = love.math.random(1, 100) --creates level platforms to land on
        if platform < 10 then
            y = ground[#ground - 1]
        else
            --y = ground[#ground - 1] + 10 --v1
            --y = love.math.random(200, system.winHeight)/4 --v2
            --y = love.math.random(ground[#ground - 1] - (system.winWidth / slices), 300) -- v3
            y = love.math.random(ground[#ground - 1] - love.math.random(0,100), 300) -- v4
        end
            

        table.insert(ground, y)
    end


    local groundReturn = {system.winWidth,love.math.random(0, system.winHeight)/2, system.winWidth,500, 000,500}
    for i,v in ipairs(groundReturn) do
        table.insert(ground, v)
    end

    --for i,v in ipairs(ground) do print(i,v) end

    

    return ground
end

function love.keypressed(key, code)
    Gamestate.keypressed(key, code) -- pass multiple arguments
end


function gameLevel03:beginContact(obj1,obj2)
    local x, y = objects.tower.body:getLinearVelocity()
    --print (y)
    --print(objects.tower.body:getAngularVelocity())
    if landingSpeed == nil then
        landingSpeed = y
        print(landingSpeed)
    end
    if y < 100 then
        gameLevel03:landed()

    else 
        print"Boom!"
        gameLevel03:crash()

    end
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
    elseif key == "space" then
        Gamestate.switch(gameLevelGoal03)
    end
end

function gameLevel03:input()

    if objects.tower.empty == false then
        if love.keyboard.isDown("right") then
            objects.tower.body:applyTorque(objects.tower.strengthTorque)
        elseif love.keyboard.isDown("left") then
            objects.tower.body:applyTorque(-objects.tower.strengthTorque)
        end
        if love.keyboard.isDown("up") then
            objects.tower.body:applyForce(objects.tower.strengthMain * math.cos(objects.tower.body:getAngle() - 1.57), objects.tower.strengthMain * math.sin(objects.tower.body:getAngle() - 1.57))
            objects.tower.fire = true
        else objects.tower.fire = false
        end
        if love.keyboard.isDown("a") then
            objects.tower.body:applyForce(objects.tower.strengthSide * math.cos(objects.tower.body:getAngle() + 3.14), objects.tower.strengthSide * math.sin(objects.tower.body:getAngle() + 3.14))
        elseif love.keyboard.isDown("d") then
            objects.tower.body:applyForce(objects.tower.strengthSide * math.cos(objects.tower.body:getAngle() + 0), objects.tower.strengthSide * math.sin(objects.tower.body:getAngle() + 0))
        end
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
    print("landed")
    system.landed = true

end


return gameLevel03