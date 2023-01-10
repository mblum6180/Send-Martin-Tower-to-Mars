--! file: building.lua
Building = Entity:extend()

function Building:new(x, y)
    Building.super.new(self, x, y, "assets/martin8bit.png")
end