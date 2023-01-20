--! file: stage02.lua

Gamestate = require 'libs.hump.gamestate'

local gameLevel02 = {}
function gameLevel02:init()
    
    junkTimer = 0
    scroll = 0
    scrollSpeed = 42
    scrollTower = 0
    space:setCallbacks(beginContact, endContact, preSolve, postSolve)

    love.graphics.setBackgroundColor(0.92, 0.70, 0.60)


    objects.tower.body = love.physics.newBody(space, love.math.random(system.winWidth * 0.2, system.winWidth * 0.8), system.winHeight * 0.9, "dynamic")
    objects.tower.body:setLinearDamping(0.9)
    objects.tower.shape = love.physics.newRectangleShape(objects.tower.width / 2, objects.tower.height / 2, objects.tower.width, objects.tower.height, 0)
    objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)
    objects.tower.fixture:setRestitution(0.3) 
    objects.tower.fixture:setFriction(0.0)


    objects.spacePeep.body = love.physics.newBody(space, love.math.random(system.winWidth * 0.2, system.winWidth * 0.8), system.winHeight * 0.1, "static")
    objects.spacePeep.shape = love.physics.newRectangleShape(objects.spacePeep.width / 2, objects.spacePeep.height / 2, objects.spacePeep.width, objects.spacePeep.height, 0)
    objects.spacePeep.fixture = love.physics.newFixture(objects.spacePeep.body, objects.spacePeep.shape, 1)
    gameLevel02:genItems(0)


end

function gameLevel02:update(dt)
    space:update(dt) 

    junkTimer = junkTimer + 1 * dt
    if junkTimer > 0.8 then
        gameLevel02:genItems(#objects.items+1) --timer for junk
        junkTimer = 0
    end
    for i,v in ipairs (objects.items) do
        print(objects.items[i].body:getY())
        if objects.items[i].body:getY() < -500 then
             table.remove(objects.items, i)
             print("remove",i)
        end
    end


    system.BGScale = system.BGScale + 0.005 * dt
    if objects.tower.body:getY() < -2000 then
        --objects.tower.fixture:destroy()
        system.level02over = true
    end


    if system.level02over then
        Gamestate.switch(gameLevelGoal02)
    end

    scrollTower = scrollTower + (scrollSpeed / 4) * dt
    objects.tower.body:setY(system.winHeight * 0.7 + -scroll - scrollTower) -- lock tower in place
    objects.tower.body:setAngle(0)

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

    scroll = scroll + (scrollSpeed * dt)
    --print(scroll)
end
  
function gameLevel02:draw()
    love.graphics.setColor(system.BGcolorR, system.BGcolorG, system.BGcolorB)
    love.graphics.draw(objects.ground.background02, 0, 0, 0, system.BGScale)

    love.graphics.translate(0, scroll)
  
    love.graphics.setColor(1.0, 1.0, 1.0)
    love.graphics.draw(objects.spacePeep.image, objects.spacePeep.body:getX(), objects.spacePeep.body:getY() ) -- Draw Space peep
    if debugMode then
        love.graphics.polygon("line", objects.spacePeep.body:getWorldPoints(objects.spacePeep.shape:getPoints()))
    end

    love.graphics.draw(objects.tower.image, objects.tower.body:getX(), objects.tower.body:getY(), objects.tower.body:getAngle()) -- Draw Tower
    if debugMode then
        love.graphics.polygon("line", objects.tower.body:getWorldPoints(objects.tower.shape:getPoints()))
    end

    for i,v in ipairs (objects.items) do
        print(i,v)
        love.graphics.draw(objects.items[i].image, objects.items[i].body:getX(), objects.items[i].body:getY() ) -- Draw Space junk
        if debugMode then
            love.graphics.polygon("line", objects.items[i].body:getWorldPoints(objects.items[i].shape:getPoints()))
        end
    end


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


    id.body = love.physics.newBody(space, love.math.random(0, system.winWidth), (0 - scroll - id.height), "static")
    id.shape = love.physics.newRectangleShape(id.width / 2, id.height / 2, id.width, id.height, 0)
    id.fixture = love.physics.newFixture(id.body, id.shape, 1)

    table.insert(objects.items, index, id)
end



return gameLevel02