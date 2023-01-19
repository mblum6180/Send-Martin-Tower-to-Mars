--! file: objects.lua

activePeeps = {}

objects = {} 

objects.ground = {}
objects.ground.background01 = love.graphics.newImage("assets/CityBG8bit.png") -- Background image
objects.ground.background02 = love.graphics.newImage("assets/spaceBG.png") -- Background image
objects.ground.background03 = love.graphics.newImage("assets/marsmountain8bit.png") -- Background image
objects.ground.box = {
    -20,-100, --1
    system.winWidth + 20,-100, --2
    system.winWidth + 20, system.winHeight, --3
    system.winWidth + 30, system.winHeight, --4
    system.winWidth + 30, -110,  --5
    -20, -110, --6
    -20, system.winHeight,--7
    -10, system.winHeight}--8

objects.tower = {}
objects.tower.image = love.graphics.newImage("assets/martin8bit.png")
objects.tower.width = objects.tower.image:getWidth()
objects.tower.height = objects.tower.image:getHeight()
objects.tower.strengthMain = 2000
objects.tower.strengthSide = 300
objects.tower.strengthTorque = 4000

objects.fire = {}
objects.fire.image = love.graphics.newImage("assets/Fireball1.png")
objects.fire.width = objects.fire.image:getWidth()
objects.fire.height = objects.fire.image:getHeight()

objects.spacePeep = {}
objects.spacePeep.image = love.graphics.newImage("assets/chick8bit.png")
objects.spacePeep.width = objects.spacePeep.image:getWidth()
objects.spacePeep.height = objects.spacePeep.image:getHeight()

objects.audio = {}
objects.audio.crash = love.audio.newSource("assets/explosion-fuzzy1.wav", "static")