--! file: objects.lua

objects = {} 

objects.ground = {}
objects.ground.body = love.physics.newBody(mars, system.winWidth/2, love.graphics.getPixelHeight() - 75)
objects.ground.shape = love.physics.newRectangleShape(system.winWidth, 150)
objects.ground.fixture = love.physics.newFixture(objects.ground.body, objects.ground.shape)


objects.tower = {}
objects.tower.image = love.graphics.newImage("assets/martin8bit.png")
objects.tower.width = objects.tower.image:getWidth()
objects.tower.height = objects.tower.image:getHeight()
objects.tower.body = love.physics.newBody(mars, love.math.random(0, system.winWidth), 0, "dynamic")
objects.tower.shape = love.physics.newRectangleShape(objects.tower.width, objects.tower.height + 75)
objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)
objects.tower.fixture:setRestitution(0.3) 
objects.tower.fixture:setFriction(0.98)