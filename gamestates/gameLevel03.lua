--! file: stage03.lua

Gamestate = require 'libs.hump.gamestate'

local gameLevel03 = {}
function gameLevel03:init()
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
    objects.tower.shape = love.physics.newRectangleShape(objects.tower.width / 2, objects.tower.height / 2, objects.tower.width, objects.tower.height, 0)
    objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)
    objects.tower.fixture:setRestitution(0.3) 
    objects.tower.fixture:setFriction(0.98)



    bgFill = {objects.ground.body:getWorldPoints(objects.ground.shape:getPoints())}
    for i = #bgFill, #bgFill-1, -1 do
        bgFill[i] = nul
    end

end

function gameLevel03:update(dt)
    mars:update(dt) 
    gameLevel03:input()
    gameLevel03:dampen()
    edge(objects.tower.body:getX(), objects.tower.body:getY())
end
  
function gameLevel03:draw()
    love.graphics.setColor(system.BGcolorR, system.BGcolorG, system.BGcolorB)
    love.graphics.draw(objects.ground.background, 0, 0)
    love.graphics.setColor(0.53, 0.39, 0.32)
    love.graphics.polygon("line", objects.ground.body:getWorldPoints(objects.ground.shape:getPoints()))
    local triangles = love.math.triangulate(bgFill)

    for i, triangle in ipairs(triangles) do 
        love.graphics.polygon("fill", triangle)
    end


    love.graphics.setColor(1.0, 1.0, 1.0)
    love.graphics.draw(objects.tower.image, objects.tower.body:getX(), objects.tower.body:getY(), objects.tower.body:getAngle() )
    love.graphics.polygon("line", objects.tower.body:getWorldPoints(objects.tower.shape:getPoints()))

end

function gameLevel03:genLandscape()

    local ground = {000,000, 001,002} --generate random landscape


    local slices = love.math.random(18,30)
    for i = 1, slices, 1 do
        x = i * (system.winWidth / slices)
        table.insert(ground, x)
        --y = ground[#ground - 1] + 10
        y = love.math.random(200, system.winHeight)/4
        table.insert(ground, y)
    end


    local groundReturn = {system.winWidth,love.math.random(0, system.winHeight)/2, system.winWidth,500, 000,500}
    for i,v in ipairs(groundReturn) do
        table.insert(ground, v)
    end

    --for i,v in ipairs(ground) do print(i,v) end

    

    return ground
end

function gameLevel03:beginContact()
    local x, y = objects.tower.body:getLinearVelocity()
    if debugMode then
        print (y)
        print(objects.tower.body:getAngularVelocity())
        if y < 100 then
            print"landed"
        else 
            print"Boom!"
            gameLevel03:crash()
        end
    end
end

function gameLevel03:dampen()

end


function gameLevel03:keypressed(key, scancode, isrepeat)
    if debugMode then
        if key == "s" then
        score = score + 1
        print(score)
        end
    end
    if key == "escape" then
        love.event.quit()
    elseif key == "space" then
        love.event.quit()
    end
end

function gameLevel03:input()
    if love.keyboard.isDown("right") then
        objects.tower.body:applyTorque(objects.tower.strengthTorque)
      elseif love.keyboard.isDown("left") then
        objects.tower.body:applyTorque(-objects.tower.strengthTorque)
      end
      if love.keyboard.isDown("up") then
          objects.tower.body:applyForce(objects.tower.strengthMain * math.cos(objects.tower.body:getAngle() - 1.57), objects.tower.strengthMain * math.sin(objects.tower.body:getAngle() - 1.57))
      end
      if love.keyboard.isDown("a") then
          objects.tower.body:applyForce(objects.tower.strengthSide * math.cos(objects.tower.body:getAngle() + 3.14), objects.tower.strengthSide * math.sin(objects.tower.body:getAngle() + 3.14))
      elseif love.keyboard.isDown("d") then
          objects.tower.body:applyForce(objects.tower.strengthSide * math.cos(objects.tower.body:getAngle() + 0), objects.tower.strengthSide * math.sin(objects.tower.body:getAngle() + 0))
    end
end


function gameLevel03:crash()
    --if objects.audio.crash then
    --    objects.audio.crash:rewind()
    --end
    -- objects.audio.crash:play()
    playSound(objects.audio.crash)
end


return gameLevel03