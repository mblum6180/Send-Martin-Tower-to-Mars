--! file: stage02.lua

Gamestate = require 'libs.hump.gamestate'

local gameLevel02 = {}
function gameLevel02:init()
    love.graphics.reset()
    objects.audio.fire:setLooping(true)
    objects.audio.fire:setVolume(0.16)
    playSound(objects.audio.fire,'play',true)
    junkTimer = 0
    scroll = 0
    scrollSpeed = 42
    scrollTower = 0
    system.score02 = system.score01

    space:setCallbacks(beginContact, endContact, preSolve, postSolve)

    love.graphics.setBackgroundColor(0.92, 0.70, 0.60)


    objects.tower.body = love.physics.newBody(space, love.math.random(system.winWidth * 0.2, system.winWidth * 0.8), system.winHeight * 0.95, "dynamic")
    objects.tower.body:setLinearDamping(0.9)
    objects.tower.shape = love.physics.newRectangleShape(objects.tower.width / 2, objects.tower.height / 2, objects.tower.width * system.scaling, objects.tower.height * system.scaling, 0)
    objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)
    objects.tower.fixture:setRestitution(0.3) 
    objects.tower.fixture:setFriction(0.0)


    gameLevel02:genItems(0)


end

function gameLevel02:update(dt)
    space:update(dt) 

    objects.image.fireball.currentFrame = objects.image.fireball.currentFrame + 10 * dt
    if objects.image.fireball.currentFrame >= 4 then
        objects.image.fireball.currentFrame = 1
    end


    junkTimer = junkTimer + 1 *system.scaling * dt
    if junkTimer > love.math.random(0.3,0.5) then
        gameLevel02:genItems(#objects.items+1) --timer for junk
        junkTimer = 0
    end
    for i in ipairs (objects.items) do
        --print(-objects.items[i].body:getY(), scroll)
        if -objects.items[i].body:getY() < scroll - system.winHeight then
             table.remove(objects.items, i)

        end
        if -objects.items[i].red == 0 then
            playSound(objects.audio.itemBreak,'stop')
            playSound(objects.audio.itemBreak,'play')
            table.remove(objects.items, i)
            system.score02 = system.score02 - 500
            system.itemsDestroyed = system.itemsDestroyed + 1
           --print (score)
       end
    end


    system.BGScale = system.BGScale + 0.005 * dt
    if objects.tower.body:getY() < -2500 then
        --objects.tower.fixture:destroy()
        system.level02over = true
    end


    if system.level02over and system.crashed == false then
        playSound(objects.audio.fire,'stop')
        Gamestate.switch(gameLevelGoal02)
    end
    if system.crashed == false then
        scrollTower = scrollTower + (scrollSpeed / 4) * dt
        objects.tower.body:setY(system.winHeight * 0.95 + -scroll - scrollTower) -- lock tower in place
        objects.tower.body:setAngle(0)
    end



    if system.crashed == false then
        if love.keyboard.isDown("right") then
        objects.tower.body:applyForce(400, 0)
        system.score02 = system.score02 - 2 * dt
        elseif love.keyboard.isDown("left") then
        objects.tower.body:applyForce(-400, 0)
        system.score02 = system.score02 - 2 * dt
        end

        system.score02 = system.score02 - 1 * dt
    end


    edge(objects.tower.body:getX(), objects.tower.body:getY())

    scroll = scroll + (scrollSpeed * dt)
    --print(scroll)

    if system.score02 <= 0 and system.crashed == false then
        system.score02 = 0
        playSound(objects.audio.problem,'play',false)
        system.crashed = true
        playSound(objects.audio.fire,'stop')
    end

end
  
function gameLevel02:draw()
    love.graphics.setColor(system.BGcolorR, system.BGcolorG, system.BGcolorB) -- draw Backgroud
    love.graphics.draw(objects.ground.background02, 0, 0, 0, system.BGScale * system.scaling, (system.winHeight / objects.ground.background02Height))

    love.graphics.translate(0, scroll)
  



    love.graphics.draw(objects.tower.image, objects.tower.body:getX(), objects.tower.body:getY(), objects.tower.body:getAngle(), system.scaling) -- Draw Tower
    if debugMode then
        love.graphics.polygon("line", objects.tower.body:getWorldPoints(objects.tower.shape:getPoints()))
    end



    for i,v in ipairs (objects.items) do -- Draw Space junk
        --print(i,v)
        love.graphics.setColor(objects.items[i].red, 1.0, 1.0)

        love.graphics.draw(objects.items[i].image, objects.items[i].body:getX(), objects.items[i].body:getY(), objects.items[i].body:getAngle(), objects.items[i].div, 1, objects.items[i].widthDiv, 0 )
     
        if debugMode then
            dx , dy = objects.items[i].body:getWorldPoints(objects.items[i].shape:getPoint())
            love.graphics.circle("line", dx, dy, 10)
        end
    end



    love.graphics.setColor(1.0, 0.0, 0.0, 1)
    love.graphics.print(math.floor(system.score02), system.winWidth * 0.1, system.winHeight * 0.1 + -scroll, 0, system.winWidth / 150, system.winWidth / 150)


    if system.crashed == false then
        love.graphics.setColor(1.0, 1.0, 1.0, 1) -- Draw Flames
        love.graphics.draw(objects.image.fireball.tex, objects.image.fireball.frames[math.floor(objects.image.fireball.currentFrame)], 
        objects.tower.body:getX() - objects.tower.width/4, 
        objects.tower.body:getY() + objects.tower.height * 0.9, objects.tower.body:getAngle(), 1, 1)
    end

    if system.crashed == true then
        love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)  -- Draw Crashed
        love.graphics.print("Out of Fuel!", system.winWidth * 0.1, system.winHeight * 0.3  + -scroll, 0, system.winWidth / 99, system.winWidth / 99)
    end
end

function gameLevel02:beginContact(obj1,obj2)
    if debugMode then
        print(obj1,obj2)
    end
    if obj1:getUserData() == null and system.crashed == false then

        if obj2:getUserData() then
            playSound(objects.audio.itemBreak,'stop')
            playSound(objects.audio.itemBreak,'play', true)
            obj2:getUserData().red = obj2:getUserData().red - .5
            system.score02 = system.score02 - 100
        end
    end


end

function gameLevel02:keypressed(key, scancode, isrepeat)
    if debugMode then
        if key == "q" then
            gameLevel02:genItems(1)
            print("MORE!")
        end
    end
    if key == "escape" then
        love.event.quit()
    --elseif key == "space" then
    --    Gamestate.switch(gameLevel03)
    end


end
function gameLevel02:genItems(id)
    index = id
    id = {}
    id.image = objects.spacePeep.image
    id.width = id.image:getWidth()
    id.height = id.image:getHeight()
    id.red = 1
    id.green = 1
    id.blue = 1
    id.body = love.physics.newBody(space, love.math.random(0, system.winWidth), (0 - scroll - id.height), "dynamic")
    id.shape = love.physics.newCircleShape(id.width / 2)
    id.fixture = love.physics.newFixture(id.body, id.shape, 1)
    id.fixture:setUserData(id)
    id.body:setAngle(love.math.random(0,6.283185))
    table.insert(objects.items, index, id)
end



return gameLevel02