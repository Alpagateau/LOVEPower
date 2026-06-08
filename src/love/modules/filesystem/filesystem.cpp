#include <dirent.h>
#include <sol/sol.hpp>
#include <string>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sys/dirent.h>

#include "filesystem.hpp"
extern "C" {
    #include <lua.h>
}

namespace love {
    namespace filesystem {
        void __init(sol::state &luastate, int argc, char **argv) {
            createDirectories("sd://LOVEPower");
            std::filesystem::current_path("sd://LOVEPower");

            createDirectories("game");
            createDirectories("save");
        }

        void createDirectories(const std::string& path) {
            std::filesystem::path dir(path);
            if (!std::filesystem::exists(dir)) {
                std::filesystem::create_directories(dir);
            }
        }

        std::string getFilePath(const std::string& file) {
            std::string path;

            if (doesPreferSaveDirectory) {
                if (std::filesystem::exists("save/" + file)) {
                    path = "save/" + file;
                } else {
                    path = "game/" + file;
                }
            } else {
                if (std::filesystem::exists("game/" + file)) {
                    path = "game/" + file;
                } else {
                    path = "save/" + file;
                }
            }

            return path;
        }

        void init(std::string identity) { // Lua accesible
            // identity is the new path to use for save and game files
            if (identity.empty()) {
                identity = "sd://LOVEPower";
            }
            std::filesystem::current_path(identity);

            createDirectories("game");
            createDirectories("save");
        }

        bool fileExists(const std::string& file) {
            std::string path = getFilePath(file);
            return std::filesystem::exists(path);
        }

        sol::protected_function load(const std::string& file, sol::this_state state) {
            sol::state_view luastate(state);

            return luastate.load_file(getFilePath(file)).get<sol::protected_function>();
        }

        sol::table getInfo(const std::string& file, sol::this_state s) {
            sol::state_view lua(s);
            sol::table info = lua.create_table();

            std::string path = getFilePath(file);
            if (!std::filesystem::exists(path)) {
                return info; // return empty table instead of nil
            }

            info["filetype"] = std::filesystem::is_directory(path) ? "directory" : "file";
            info["size"] = static_cast<double>(std::filesystem::file_size(path));

            auto ftime = std::filesystem::last_write_time(path);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now());
            info["modtime"] = static_cast<double>(std::chrono::system_clock::to_time_t(sctp));

            return info;
        }

        bool exists(const std::string& file, sol::this_state lua) {
            std::string path = getFilePath(file);
            return std::filesystem::exists(path);
        }

        void preferSaveDirectory(const bool preferSave) {
            doesPreferSaveDirectory = preferSave;
        }

        int getDirectoryItems(lua_State* L)
        {
          DIR* dir;
          struct dirent* ent;

          const char* path = lua_tostring(L, -1);
          lua_pop(L, 1);
          printf("Listing directory : %s\n", path);

          printf("Create an empty table\n");
          lua_newtable(L);
          
          int idx = 1;
          if((dir = opendir(path)) != NULL)
          {
            while((ent = readdir(dir)) != NULL)
            {
              printf("> %s\n", ent->d_name);
              lua_pushnumber(L, idx++);
              lua_pushstring(L, ent->d_name);
              lua_settable(L, -3);
            }
          }
          else
          {
            printf("Couldn't open the directory\n");
          }

          return 1;
        }
        
    }
}

int luaopen_love_filesystem(lua_State *L)  {

    printf("<== MODULE LOVE FS ==>\n");
    sol::state_view luastate(L);

    luastate["love"]["filesystem"] = luastate.create_table_with(
        "init", love::filesystem::init,
        "load", love::filesystem::load,
        "getInfo", love::filesystem::getInfo,
        //"newFile", love::filesystem::newFile,
        "getDirectoryItems", love::filesystem::getDirectoryItems,
        "exists", love::filesystem::exists,
        "preferSaveDirectory", love::filesystem::preferSaveDirectory
    );

    return 1;
}
