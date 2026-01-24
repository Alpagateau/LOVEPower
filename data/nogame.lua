local love = require("love")

function love.nogame()
    -- TODO: Make this pretty?
    print("No game found")
    function love.draw()
        love.graphics.setBackgroundColor(1, 0, 0)
        love.graphics.setColor(1, 1, 1)
        love.graphics.print("No game found. Please add a main.lua file to sd:/LOVEPower/game/", 10, 10)
    end
end
