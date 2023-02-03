-- Configuration
function love.conf(t)
    t.title = "Send Martin Tower to Mars!" -- The title of the window the game is in (string)
	t.version = "11.3"         -- The LÖVE version this game was made for (string)
	t.window.icon = "assets/martinIcon.png"
	t.window.width = 1600        
	t.window.height = 720
    t.window.resizable = false
    t.modules.joystick = false 
    t.modules.mouse = false 

end