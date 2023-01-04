-- Configuration
function love.conf(t)
    t.title = "Send Martin Tower to Mars!" -- The title of the window the game is in (string)
	t.version = "11.3"         -- The LÖVE version this game was made for (string)
	t.window.icon = "Assets/isometric_office_5.png"
	t.window.width = 1280        
	t.window.height = 720
    t.window.resizable = false
    t.window.highdpi = true
    t.window.usedpiscale = true
end