#include "joystick.h"
#include "joystick_wrapper.h"

extern "C" int luaopen_love_joystick(lua_State *L) {
    printf("<== MODULE LOVE JOYSTICK ==>\n");
    sol::state_view luastate(L);

    // 1. Register the "Joystick" custom object type in Lua
    luastate.new_usertype<love::joystick::love_joystick_t>(
        "Joystick",
        sol::no_constructor, // Prevent Lua from creating raw joysticks with Joystick.new()
        
        // Standard Love2D API Methods
        "isConnected", &love::joystick::isConnected,
        "getButtonCount", &love::joystick::getButtonCount,
        "getAxisCount", &love::joystick::getAxisCound, // Maps to your custom spelling
        
        "getName", [](love::joystick::love_joystick_t* j) {
            return j->name;
        },

        "getAxes", [](love::joystick::love_joystick_t* j, sol::this_state s) {
            sol::state_view lua(s);
            return sol::make_object(lua, love::joystick::getAxes(j));
        },

        "getAxis", [](love::joystick::love_joystick_t* j, int luaAxisIdx) {
            // Convert Lua 1-based indexing to C++ 0-based indexing safely
            return love::joystick::getAxis(j, luaAxisIdx - 1);
        },

        "isDown", [](love::joystick::love_joystick_t* j, std::string buttonName) {
            return love::joystick::isDown(j, buttonName);
        },

        // --- Custom Wii/Wiimote Engine Extensions ---

        "setOrientation", [](love::joystick::love_joystick_t* j, std::string orientation) {
            std::transform(orientation.begin(), orientation.end(), orientation.begin(), ::tolower);
            if (orientation == "horizontal" || orientation == "sideways") {
                love::joystick::setOrientation(j, love::joystick::LOVE_ORIENTATION_HORIZONTAL);
            } else {
                love::joystick::setOrientation(j, love::joystick::LOVE_ORIENTATION_VERTICAL);
            }
        },

        "getOrientation", [](love::joystick::love_joystick_t* j) {
            auto current = love::joystick::getOrientation(j);
            return (current == love::joystick::LOVE_ORIENTATION_HORIZONTAL) ? "horizontal" : "vertical";
        },

        "setMouseMode", [](love::joystick::love_joystick_t* j, bool enable) {
            love::joystick::setMouseMode(j, enable);
        },

        "isMouseModeEnabled", [](love::joystick::love_joystick_t* j) {
            return love::joystick::isMouseModeEnabled(j);
        },

        "isGamepad", love::joystick::isGamepad,
        "isGamepadDown", love::joystick::isGamepadDown,
        "getGamepadAxis", love::joystick::getGamepadAxis
    );

    // 2. Register the module-level functions under love.joystick
    luastate["love"]["joystick"] = luastate.create_table_with(
        "getJoystickCount", love::joystick::getJoystickCound,
        "loadGamepadMappings" , love::joystick::loadGamepadMappings,
        "getJoysticks", [](sol::this_state s) {
            sol::state_view lua(s);
            sol::table luaArray = lua.create_table();
            
            // Gather the persistent pointer tracking list from your backend
            auto activeJoysticks = love::joystick::getJoysticks();
            
            // Populate a clean, standard 1-indexed Lua array table
            for (size_t i = 0; i < activeJoysticks.size(); ++i) {
                luaArray[i + 1] = activeJoysticks[i]; // Sol2 automatically treats this as the "Joystick" usertype
            }
            
            return luaArray;
        }
    );

    return 1;
}
