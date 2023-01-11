--! file: entity.lua
Entity = Object:extend()

function Entity:new(x, y, image_path, a, sx, sy)
    self.x = x
    self.y = y
    self.angle = a
    self.scalex = sx
    self.scaley = sy
    self.image = love.graphics.newImage(image_path)
    self.width = self.image:getWidth()
    self.height = self.image:getHeight()
end

function Entity:update(dt)

end

function Entity:draw()
    love.graphics.draw(self.image, self.x, self.y, self.angle, self.scalex, self.scaley)
end