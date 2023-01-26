--! file: stage02.lua

Gamestate = require 'libs.hump.gamestate'

local gameLevel02 = {}
function gameLevel02:init()
    love.graphics.reset()
    junkTimer = 0
    scroll = 0
    scrollSpeed = 42
    scrollTower = 0
    system.score02 = system.score01

    space:setCallbacks(beginContact, endContact, preSolve, postSolve)

    love.graphics.setBackgroundColor(0.92, 0.70, 0.60)


    objects.tower.body = love.physics.newBody(space, love.math.random(system.winWidth * 0.2, system.winWidth * 0.8), system.winHeight * 0.95, "dynamic")
    objects.tower.body:setLinearDamping(0.9)
    objects.tower.shape = love.physics.newRectangleShape(objects.tower.width / 2, objects.tower.height / 2, objects.tower.width, objects.tower.height, 0)
    objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)
    objects.tower.fixture:setRestitution(0.3) 
    objects.tower.fixture:setFriction(0.0)


    gameLevel02:genItems(0)


end

function gameLevel02:update(dt)
    space:update(dt) 

    if system.score02 <= 0 then 
        system.score02 = 0
        objects.tower.crashed = true
    end

    objects.image.fireball.currentFrame = objects.image.fireball.currentFrame + 10 * dt
    if objects.image.fireball.currentFrame >= 4 then
        objects.image.fireball.currentFrame = 1
    end


    junkTimer = junkTimer + 1 * dt
    if junkTimer > 0.3 then
        gameLevel02:genItems(#objects.items+1) --timer for junk
        junkTimer = 0
    end
    for i in ipairs (objects.items) do
        --print(-objects.items[i].body:getY(), scroll)
        if -objects.items[i].body:getY() < scroll - system.winHeight then
             table.remove(objects.items, i)
             --print("remove",i)
             system.score = system.score + 1
            --print (score)
        end
        if -objects.items[i].red == 0 then
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


    if system.level02over then
        Gamestate.switch(gameLevelGoal02)
    end

    scrollTower = scrollTower + (scrollSpeed / 4) * dt
    objects.tower.body:setY(system.winHeight * 0.95 + -scroll - scrollTower) -- lock tower in place
    objects.tower.body:setAngle(0)

    if love.keyboard.isDown("right") then
      objects.tower.body:applyForce(400, 0)
    elseif love.keyboard.isDown("left") then
      objects.tower.body:applyForce(-400, 0)
    end


    edge(objects.tower.body:getX(), objects.tower.body:getY())

    scroll = scroll + (scrollSpeed * dt)
    --print(scroll)
end
  
function gameLevel02:draw()
    love.graphics.setColor(system.BGcolorR, system.BGcolorG, system.BGcolorB)
    love.graphics.draw(objects.ground.background02, 0, 0, 0, system.BGScale)

    love.graphics.translate(0, scroll)
  



    love.graphics.draw(objects.tower.image, objects.tower.body:getX(), objects.tower.body:getY(), objects.tower.body:getAngle()) -- Draw Tower
    if debugMode then
        love.graphics.polygon("line", objects.tower.body:getWorldPoints(objects.tower.shape:getPoints()))
    end



    for i,v in ipairs (objects.items) do
        --print(i,v)
        love.graphics.setColor(objects.items[i].red, 1.0, 1.0)
        love.graphics.draw(objects.items[i].image, objects.items[i].body:getX(), objects.items[i].body:getY(), objects.items[i].body:getAngle()  ) -- Draw Space junk
        if debugMode then
            love.graphics.polygon("line", objects.items[i].body:getWorldPoints(objects.items[i].shape:getPoints()))
        end
    end



    love.graphics.setColor(1.0, 0.0, 0.0, 1)
    love.graphics.print(math.floor(system.score02), system.winWidth * 0.1, system.winHeight * 0.1 + -scroll, 0, system.winWidth / 150, system.winWidth / 150)

    love.graphics.setColor(1.0, 1.0, 1.0, 1)
    love.graphics.draw(objects.image.fireball.tex, objects.image.fireball.frames[math.floor(objects.image.fireball.currentFrame)], 
    objects.tower.body:getX() - objects.tower.width/4, 
    objects.tower.body:getY() + objects.tower.height * 0.9, objects.tower.body:getAngle(), 1, 1)



end

function gameLevel02:beginContact(obj1,obj2)
    if debugMode then
        print(obj1,obj2)
    end
    if obj1:getUserData() == null then

        if obj2:getUserData() then
            playSound(objects.audio.itemBreak,'play', true)
            obj2:getUserData().red = obj2:getUserData().red - .5
            system.score02 = system.score02 - 100
        end
    end


end

function gameLevel02:keypressed(key, scancode, isrepeat)
    if debugMode then
        if key == "s" then
        system.score = system.score + 1
        print(system.score)
        end
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
    id.image = love.graphics.newImage("assets/chick8bit.png")
    id.width = id.image:getWidth()
    id.height = id.image:getHeight()
    id.red = 1
    id.green = 1
    id.blue = 1

    id.body = love.physics.newBody(space, love.math.random(0, system.winWidth), (0 - scroll - id.height), "dynamic")
    id.shape = love.physics.newRectangleShape(id.width / 2, id.height / 2, id.width, id.height, 0)
    id.fixture = love.physics.newFixture(id.body, id.shape, 1)
    id.fixture:setUserData(id)
    table.insert(objects.items, index, id)
end



return gameLevel02