--! file: main.lua



function love.load()
    buildingImage = love.graphics.newImage("Assets/isometric_office_5.png")
    buildingImageWidth = buildingImage:getWidth()
    buildingImageHeight = buildingImage:getHeight()
    background = love.graphics.newImage("Assets/marsmountain.png")

    love.physics.setMeter(64)
    world = love.physics.newWorld(0, 3.72*64, true)
    world:setCallbacks(beginContact, endContact, preSolve, postSolve)
  
    objects = {} 
    objects.ground = {}
    objects.ground.body = love.physics.newBody(world, love.graphics.getPixelWidth()/2, love.graphics.getPixelHeight() - 75)
    objects.ground.shape = love.physics.newRectangleShape(love.graphics.getPixelWidth(), 150)
    objects.ground.fixture = love.physics.newFixture(objects.ground.body, objects.ground.shape)
  
    objects.tower = {}
    objects.tower.body = love.physics.newBody(world, love.math.random(0, love.graphics.getPixelWidth()), 0, "dynamic")
    objects.tower.shape = love.physics.newRectangleShape(buildingImageWidth, buildingImageHeight + 75)
    objects.tower.images = buildingImage
    objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)
    objects.tower.fixture:setRestitution(0.3) 
    objects.tower.fixture:setFriction(0.98)
  

    love.graphics.setBackgroundColor(0.92, 0.70, 0.60)
      end
  
  
  function love.update(dt)
    world:update(dt) 

    if love.keyboard.isDown("right") then
      objects.tower.body:applyForce(400, 0)
    elseif love.keyboard.isDown("left") then
      objects.tower.body:applyForce(-400, 0)
    end
    if love.keyboard.isDown("up") then
        objects.tower.body:applyForce(0, -3000)
    end
  end
  
  function love.draw()
    love.graphics.draw(background, 0, 0)
    love.graphics.setColor(0.53, 0.39, 0.32)
    love.graphics.polygon("fill", objects.ground.body:getWorldPoints(objects.ground.shape:getPoints()))
  
    love.graphics.setColor(1.0, 1.0, 1.0)
    love.graphics.draw(buildingImage, objects.tower.body:getX(), objects.tower.body:getY() )


  end

function beginContact()
    local x, y = objects.tower.body:getLinearVelocity()
    print (y)
    print(objects.tower.body:getAngularVelocity())
    if y < 100 then
        print"landed"
    else 
        print"Boom!"
    end
end