--! file: objects.lua

system = {}
system.winWidth = love.graphics.getPixelWidth()
system.winHeight = love.graphics.getPixelHeight()
system.level01over = false
system.level02over = false
system.level03over = false
system.BGScale = 1
system.BGcolorR = 1
system.BGcolorG = 1
system.BGcolorB = 1
system.score01 = 10000
system.score02 = 0
system.score03 = 0
system.score = 0
system.landed = false
system.landedTimer = 2
system.debounce = 0
system.itemsDestroyed = 0


activePeeps = {}

objects = {} 
objects.image = {}
objects.image.fireball = {}
objects.image.fireball.tex = love.graphics.newImage("assets/towerFireball.png")
local width = objects.image.fireball.tex:getWidth()
local height = objects.image.fireball.tex:getHeight() 
objects.image.fireball.frames = {}
local frame_width = 107
local frame_height = 147
maxFrames = 4
for i=0,0 do
    for j=0,3 do
        table.insert(objects.image.fireball.frames, love.graphics.newQuad(j * frame_width, i * frame_height, frame_width, frame_height, width, height))
        if #objects.image.fireball.frames == maxFrames then
            break
        end 
    end
end
objects.image.fireball.currentFrame = 1

objects.image.explosion = {}
objects.image.explosion.tex = love.graphics.newImage("assets/explosion3.png")
local width = objects.image.explosion.tex:getWidth()
local height = objects.image.explosion.tex:getHeight() 
objects.image.explosion.frames = {}
local frame_width = 107
local frame_height = 147
maxFrames = 4
for i=0,0 do
    for j=0,3 do
        table.insert(objects.image.explosion.frames, love.graphics.newQuad(j * frame_width, i * frame_height, frame_width, frame_height, width, height))
        if #objects.image.explosion.frames == maxFrames then
            break
        end 
    end
end
objects.image.explosion.currentFrame = 1




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
objects.tower.strengthTorque = 2000
objects.tower.strengthMain = 2000
objects.tower.strengthSide = 300
objects.tower.fire = false
objects.tower.empty = false
objects.tower.crashed = false


objects.spacePeep = {}
objects.spacePeep.image = love.graphics.newImage("assets/chick8bit.png")
objects.spacePeep.width = objects.spacePeep.image:getWidth()
objects.spacePeep.height = objects.spacePeep.image:getHeight()

objects.items = {}


objects.audio = {}
objects.audio.crash = love.audio.newSource("assets//audio/explosion-fuzzy1.ogg", "static")
objects.audio.fire = love.audio.newSource("assets/audio/qubodupFireLoop.ogg", "static")
objects.audio.launch = love.audio.newSource("assets/audio/rocket_launch.ogg", "static")
objects.audio.mainTheme = love.audio.newSource("assets/audio/Abandoned Steel Mill.ogg", "static")
objects.audio.itemBreak = love.audio.newSource("assets/audio/rock_break.ogg", "static")