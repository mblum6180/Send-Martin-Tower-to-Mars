--! file: stage03.lua

Gamestate = require 'libs.hump.gamestate'

local gameLevel03 = {}
function gameLevel03:init()
    objects.ground.landscape = gameLevel03:genLandscape()
    background = love.graphics.newImage("assets/marsmountain8bit.png") -- Background image

    mars:setCallbacks(beginContact, endContact, preSolve, postSolve)

    love.graphics.setBackgroundColor(0.92, 0.70, 0.60)
    objects.ground.body = love.physics.newBody(mars, 0, love.graphics.getPixelHeight() * 0.5)
    objects.ground.shape = love.physics.newChainShape(true, objects.ground.landscape, "static")
    objects.ground.fixture = love.physics.newFixture(objects.ground.body, objects.ground.shape)

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
    love.graphics.draw(background, 0, 0)
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

    local ground = {000,000, 001,002, --hack to test random landscape
    100,love.math.random(0, system.winHeight)/2,
    200,love.math.random(0, system.winHeight)/2, 
    250,love.math.random(0, system.winHeight)/2, 
    300,love.math.random(0, system.winHeight)/2, 
    400,love.math.random(0, system.winHeight)/2, 
    500,love.math.random(0, system.winHeight)/2, 
    600,love.math.random(0, system.winHeight)/2,
    700,love.math.random(0, system.winHeight)/2,
    800,love.math.random(0, system.winHeight)/2, 
    900,love.math.random(0, system.winHeight)/2, 
    1000,love.math.random(0, system.winHeight)/2, 
    1100,love.math.random(0, system.winHeight)/2,
    1200,love.math.random(0, system.winHeight)/2
    }
    table.insert(ground, system.winWidth)
    table.insert(ground, love.math.random(0, system.winHeight)/2)
    table.insert(ground, system.winWidth)
    table.insert(ground, 500)
    table.insert(ground, 000)
    table.insert(ground, 500)

    for i,v in ipairs(ground) do print(i,v) end

    

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