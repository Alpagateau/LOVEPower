local love = require("love")

local original_poll = love.event.poll

love.event.poll = function()
    return function()
        local n, a, b, c, d, e, f = original_poll()
        if n then
            -- If the event name itself or an argument is a userdata
            if type(n) == "userdata" or type(a) == "userdata" then
                print("--- Rogue Userdata Detected ---")
                print("Event Name type:", type(n), n)
                print("Arg 1 type:", type(a), a)
                
                -- Try to extract a metatable name
                local mt = getmetatable(n or a)
                if mt then
                    print("Metatable __name:", mt.__name)
                end
                
                -- Print the traceback to see how love.run is calling it
                print(debug.traceback()) 
            end
            return n, a, b, c, d, e, f
        end
    end
end

function love.createhandlers()
    print("[LUA] Create Handlers <default>")
    love.handlers = {
        quit = function()
		    end,
        lowmemory = function()
            if love.lowmemory then love.lowmemory() end
            collectgarbage()
            collectgarbage()
        end,
        mousemoved = function(x, y, dx, dy, touch)
        end
    }
    setmetatable(love.handlers, {__index = function(_self, name)
            print("Unknown event: " .. tostring(name).."("..")")
        end
    })
end

function Info(s)
  print("[CALLBACKS INFO] "..s)
end

function love.run()
    print("love run")
    if love.load then love.load(love.arg.parseGameArguments(arg), arg) end
    if love.timer then love.timer.step() end
    local dt = 0

    while true do
      print("loop")
        if love.event then
            love.event.pump()
            while true do
                local success, name, a, b, c, d, e, f = pcall(love.event.poll)
                if not success then Info("<ERROR> "..name) end
                if not name then break end
                if name == "quit" then
                    if not love.quit or not love.quit() then
                        return a or 0
                    end
                else

                    love.handlers[name](a, b, c, d, e, f)
                end
            end
            if love.timer then dt = love.timer.step() end
--            Info("wiimote")
            print("love update")
            if love.update then love.update(dt) end
--            Info("update")

            print("love graphics")
            if love.graphics and love.graphics.isActive() then
                love.graphics.origin()
                --Info("Origin")
                love.graphics.clear(love.graphics.getBackgroundColor()) -- TODO: Figure out why this freezes the game
                --Info("Clear")
                -- until then, render a rectangle
                local lastColor = {love.graphics.getColor()}
                love.graphics.setColor(love.graphics.getBackgroundColor())
                love.graphics.rectangle("fill", 0, 0, love.graphics.getWidth(), love.graphics.getHeight())
                love.graphics.setColor(unpack(lastColor))
                print("=== (DRAW) ===")
                if love.draw then love.draw() end
                print("=== (PRESENT) ===")
                love.graphics.present()
                print("==(END)==")
            end

            love.timer.sleep(0.001)
            collectgarbage("step")
        end
    end
end

local debug, print, tostring, error = debug, print, tostring, error

function love.threaderror(t, err)
	error("Thread error ("..tostring(t)..")\n\n"..err, 0)
end

local function error_printer(msg, layer)
	print((debug.traceback("Error: " .. tostring(msg), 1+(layer or 1)):gsub("\n[^\n]+$", "")))
end

local function writeLog(msg)
    local f = io.open("sd://love_error_lua.log", "a")
    if f then
        f:write(msg, "\n")
        f:close()
    end
end

function love.errhand(err)
    local msg = tostring(err) .. "\n" ..
        (debug and debug.traceback and debug.traceback("", 2) or "")
    writeLog(msg)

    love.audio.stop()
    love.graphics.setFont(love.graphics.newFont(14))

    while true do
        love.graphics.origin()
        love.graphics.setColor(89/255, 157/255, 220/255)
        love.graphics.print("Error:", 10, 10)
        -- get a table of all the lines
        local lines = {}
        for line in msg:gmatch("([^\n]*)\n?") do
            table.insert(lines, line)
        end
        local y = 30
        for i, line in ipairs(lines) do
            love.graphics.print(line, 20, y)
            y = y + 20
        end
        love.graphics.present()

        love.event.pump()
        while true do
            local name, a, b, c, d, e, f = love.event.poll()
            if not name then break end
            if name == "quit" then
                return
            end
        end
    end
end
