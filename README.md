# LÖVEPower

A "port" of LÖVE2D to the Wii. Based heavily off of [WiiLove](https://github.com/HTV04/WiiLove), then improved by [GuglioIsStupid](https://github.com/GuglioIsStupid).

PR's are heavily appreaciated! I'm still new to this stuff

Want support? Join the [Discord Server](https://discord.gg/wqkQxKNNu5)

# Compiling

1. Download, compile and install [Lua for Wii](https://wiibrew.org/wiki/Lua_for_Wii). You'll need to modify the default LUA_PATH and add 'sd://LOVEPower/game/?.lua'
2. (Optional) Download and install [libmii](https://wiibrew.org/wiki/Libmii).
3. Download, compile and install [SDL-Wii](https://github.com/GuglioIsStupid/sdl-wii).
4. Compile with "make". For some reason multiple threads don't work(?). I'll look into it in the near future.

Modified libraries will be available in source code for ease of user compiling

# TODO
- Implement require semantics
- Implement Mii data (love.mii) (Via libmii)
- Implement transforms (love.graphics)
- Fix freezing with some graphical functions (love.graphics)
- Make an app launcher for multiple lua scripts. (no lua needed)
- Test the API (using https://github.com/ellraiser/love-test)

Not in a specific order.

# Supported
- love.wiimote
- love.graphics
- love.filesystem
- love.timer
- love.math
- love.event
- love.audio
- love.physics (Via Box2d, direct port from LÖVE2D)

A wiki containing supported functions will be created in the future, for now you can look inside the modules CPP files as your wiki.

# Unsupported
Any keyboard, mouse, touch and joystick functions

# NOTICE
A lot of this code is directly taken from LÖVE2D, big thanks to the LÖVE team for making such an amazing framework!

This project is not affiliated with the LÖVE team in any way.
