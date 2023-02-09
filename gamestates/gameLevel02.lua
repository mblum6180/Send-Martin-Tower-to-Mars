--! file: stage02.lua

Gamestate = require 'libs.hump.gamestate'

gameLevel02 = {}

function gameLevel02:enter()
    love.graphics.reset()
    objects.audio.fire:setLooping(true)
    objects.audio.fire:setVolume(0.16)
    playSound(objects.audio.fire,'play',true)
    junkTimer = 0
    scroll = 0
    scrollSpeed = 42
    scrollTower = 0
    system.score02 = system.score01
    objects.items = {}
    space:setCallbacks(beginContact, endContact, preSolve, postSolve)

    love.graphics.setBackgroundColor(0.92, 0.70, 0.60)


    objects.tower.body = love.physics.newBody(space, love.math.random(system.winWidth * 0.2, system.winWidth * 0.8), system.winHeight * 0.95, "dynamic")
    objects.tower.body:setLinearDamping(0.9)
    info = {
        name = "tower",
        color = {1, 1, 1},
        scale = 1
    }
    objects.tower.body:setUserData(info)
    objects.tower.shape = love.physics.newRectangleShape(objects.tower.width / 2 * system.scaling, objects.tower.height / 2 * system.scaling, objects.tower.width * system.scaling, objects.tower.height * system.scaling, 0)
    objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)
    objects.tower.fixture:setRestitution(0.3) 
    objects.tower.fixture:setFriction(0.0)


end

function gameLevel02:update(dt)
    space:update(dt) 

    objects.image.fireball.currentFrame = objects.image.fireball.currentFrame + 10 * dt
    if objects.image.fireball.currentFrame >= 4 then
        objects.image.fireball.currentFrame = 1
    end


    junkTimer = junkTimer + 1 *system.scaling * dt
    gameLevel02:genItems() --timer for junk


    bodies = space:getBodies() -- Junk cleanup 
    for i, body in ipairs(bodies) do
        local userData = body:getUserData()
        local x, y = body:getPosition()
        if userData.name == "junk" then
            if -y < scroll - system.winHeight * 1.1 then
                body:destroy()
                print("BOOOOOM")
            end
        end
        if userData.color[1] <= 0 then
            body:destroy()
            print("BOOOOOM")
        end
    end


    for i in ipairs (objects.items) do
        --print(-objects.items[i].body:getY(), scroll)
        if objects.items[i].body:getY() < scroll - system.winHeight * 1.1 then
             --table.remove(objects.items, i)
             objects.items[i].body:destroy()

        end
        if objects.items[i].red == 0 then
            playSound(objects.audio.itemBreak,'stop')
            playSound(objects.audio.itemBreak,'play')
            system.score02 = system.score02 - 500 *  objects.items[i].scale
            --table.remove(objects.items, i)
            objects.items[i].body:destroy()
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
        if Gamestate.current() == gameLevel02 then
            objects.tower.body:setY(system.winHeight * 0.95 + -scroll - scrollTower) -- lock tower in place
            objects.tower.body:setAngle(0)
        end
    end



    if system.crashed == false and Gamestate.current() == gameLevel02 then
        if love.keyboard.isDown("right") then
        objects.tower.body:applyForce(400, 0)
        system.score02 = system.score02 - 2 * dt
        elseif love.keyboard.isDown("left") then
        objects.tower.body:applyForce(-400, 0)
        system.score02 = system.score02 - 2 * dt
        end

        system.score02 = system.score02 - 1 * dt
    end

    if Gamestate.current() == gameLevel02 then
        edge(objects.tower.body:getX(), objects.tower.body:getY())
    end
   

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
    love.graphics.draw(objects.ground.background02, 0, 0, 0, system.BGScale * system.scaling)

    love.graphics.translate(0, scroll)
  



    love.graphics.draw(objects.tower.image, objects.tower.body:getX(), objects.tower.body:getY(), 
        objects.tower.body:getAngle(), system.scaling, system.scaling, 0, 0) -- Draw Tower
    if debugMode then
        love.graphics.polygon("line", objects.tower.body:getWorldPoints(objects.tower.shape:getPoints()))
    end


    bodies = space:getBodies()
    for i, body in ipairs(bodies) do
        local userData = body:getUserData()
        if userData.name == "junk" then
            local x, y = body:getPosition()
            local angle = body:getAngle()
            love.graphics.draw(objects.spacePeep.image, x, y)

            if debugMode then
        --        love.graphics.circle("line", objects.items[i].body:getX(), objects.items[i].body:getY(), objects.items[i].shape:getRadius())
                --dr = objects.items[i].body:getWorldPoints(objects.items[i].shape:getRadius())
                --dx , dy = objects.items[i].body:getWorldPoints(objects.items[i].shape:getPoint())
                love.graphics.circle("line", x, y, 10)
            end
        end
    end
    


    --for i,v in ipairs (junk) do -- Draw Space junk
    --    print(i,v)
    --    love.graphics.setColor(objects.items[i].red, 1.0, 1.0)

    --    love.graphics.draw(objects.items[i].image, objects.items[i].body:getX(), objects.items[i].body:getY(), objects.items[i].body:getAngle(), objects.items[i].scale, objects.items[i].scale, objects.items[i].width / 2, objects.items[i].height / 2)
     
    --    if debugMode then
    --        love.graphics.circle("line", objects.items[i].body:getX(), objects.items[i].body:getY(), objects.items[i].shape:getRadius())
            --dr = objects.items[i].body:getWorldPoints(objects.items[i].shape:getRadius())
            --dx , dy = objects.items[i].body:getWorldPoints(objects.items[i].shape:getPoint())
            --love.graphics.circle("line", dx, dy, dr)
    --    end
    --end



    love.graphics.setColor(1.0, 0.0, 0.0, 1)
    love.graphics.setFont(scoreFont)
    love.graphics.print(math.floor(system.score02), system.winWidth * 0.1, system.winHeight * 0.1 + -scroll, 0)


    if system.crashed == false then
        love.graphics.setColor(1.0, 1.0, 1.0, 0.7) -- Draw Flames
        love.graphics.draw(objects.image.fireball.tex, objects.image.fireball.frames[math.floor(objects.image.fireball.currentFrame)], 
        objects.tower.body:getX() - objects.tower.width/4  * system.scaling, 
        objects.tower.body:getY() + objects.tower.height * 0.9  * system.scaling, objects.tower.body:getAngle(), system.scaling, system.scaling)
    end

    if system.crashed == true then
        love.graphics.setColor(1.0, 0.0, 0.0, bgAlpha)  -- Draw Crashed
        love.graphics.setFont(screenFont)
        love.graphics.print("Out of Fuel!", system.winWidth * 0.1, system.winHeight * 0.3  + -scroll, 0)
    end
end

function gameLevel02:beginContact(obj1,obj2)
    if debugMode then
        print(obj1,obj2, obj1:getBody():getUserData().name, obj2:getBody():getUserData().name)

    end
    if obj1:getBody():getUserData().name == "tower" and system.crashed == false then
            playSound(objects.audio.itemBreak,'stop')
            playSound(objects.audio.itemBreak,'play', true)
            obj2:getBody():getUserData().color[1] = obj2:getBody():getUserData().color[1] - .5
            system.score02 = system.score02 - 50 -- *  obj1:getUserData().scale
            print("Bang1")
    end

    if obj1:getBody():getUserData().name =="tower" and system.crashed == false  then
        playSound(objects.audio.itemBreak,'stop')
        playSound(objects.audio.itemBreak,'play', true)
        --obj2:getUserData().red = obj2:getUserData().red - .5
        system.score02 = system.score02 - 50 -- *  obj2:getUserData().scale
        print("Bang2")
    end



end

function gameLevel02:keypressed(key, scancode, isrepeat)
    if debugMode then
        if key == "s" then
            objects.items = {}
        end
    end
    if key == "escape" then
        love.event.quit()
    --elseif key == "space" then
    --    Gamestate.switch(gameLevel03)
    end


end
function gameLevel02:genItems()
    if junkTimer > love.math.random(0.3,8)  and Gamestate.current() == gameLevel02 then -- 0.3,0.8
        local image = objects.spacePeep.image
        local width = image:getWidth()
        local height = image:getHeight()
        local red = 1
        local green = 1
        local blue = 1
        local scale = love.math.random(0.8,1.5)
        local body = love.physics.newBody(space, love.math.random(0, system.winWidth), (0 - scroll - height), "dynamic")
        local shape = love.physics.newCircleShape(width / 2 * scale)
        local fixture = love.physics.newFixture(body, shape, 1)
        info = {
            name = "junk",
            color = {1, 1, 1},
            scale = 1
        }
        body:setUserData(info)
        --print(body:getUserData())
        body:setAngle(love.math.random(0,6.283185)) --6.283185
        body:setAngularVelocity(love.math.random(-1.2,1.2))
        body:applyForce(love.math.random(-200,200), love.math.random(-200, 200)) --apply force to give junk movement
        junkTimer = 0
    end
end


function gameLevel02:leave()
    --objects.tower.body:destroy()

    bodies = space:getBodies()
    for i, body in ipairs(bodies) do
        body:destroy()
         print("BOOOOOM")

    end
end

return gameLevel02