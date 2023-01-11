--! file: building.lua
Building = Entity:extend()

function Building:new(x, y, a, sx, sy)
    Building.super.new(self, x, y, "assets/martin8bit.png", a, sx, sy)
end