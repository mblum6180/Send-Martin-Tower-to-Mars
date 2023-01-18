--! file: objects.lua

objects = {} 

objects.ground = {}
objects.ground.background = love.graphics.newImage("assets/marsmountain8bit.png")
objects.ground.box = {0,-100, system.winWidth,-100, system.winWidth,-110, 0,-110}

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

objects.audio = {}
objects.audio.crash = love.audio.newSource("assets/explosion-fuzzy1.wav", "static")