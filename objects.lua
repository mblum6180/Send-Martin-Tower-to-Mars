--! file: objects.lua


background = love.graphics.newImage("assets/marsmountain8bit.png")


objects = {} 
objects.building = {}
objects.building.image = love.graphics.newImage("assets/martin8bit.png")
objects.building.image.width = objects.building.image:getWidth()
objects.building.image.height = objects.building.image:getHeight()

objects.ground = {}
objects.ground.body = love.physics.newBody(world, love.graphics.getPixelWidth()/2, love.graphics.getPixelHeight() - 75)
objects.ground.shape = love.physics.newRectangleShape(love.graphics.getPixelWidth(), 150)
objects.ground.fixture = love.physics.newFixture(objects.ground.body, objects.ground.shape)

objects.tower = {}
objects.tower.body = love.physics.newBody(world, love.math.random(0, love.graphics.getPixelWidth()), 0, "dynamic")
objects.tower.shape = love.physics.newRectangleShape(building.width, building.height + 75)
objects.tower.images = buildingImage
objects.tower.fixture = love.physics.newFixture(objects.tower.body, objects.tower.shape, 1)
objects.tower.fixture:setRestitution(0.3) 
objects.tower.fixture:setFriction(0.98)

