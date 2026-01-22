#include "love/modules/filesystem/filesystem.hpp"
#include <cstdio>
#define SOL_ALL_SAFETIES_ON 1
#ifndef USE_LUAJIT
    #define SOL_LUAJIT 0
#endif

#include "love/love.hpp"

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>

void init_debug() {
    // Initialize FAT for SD card access
    if (!fatInitDefault()) {
        // If FAT fails, try minimal video debug
        VIDEO_Init();
        CON_InitEx(NULL, 40, 30, 10, 10);
        printf("FAT init failed, using console\n");
    } else {
        // Write to SD card
        FILE* f = fopen("sd:/lovepower.log", "w");
        if (f) {
            fprintf(f, "LOVEPower starting...\n");
            fclose(f);
        }
    }
}

int main(int argc, char** argv) {

    init_debug();
    FILE* f = fopen("sd:lovepower.log", "a");
    fprintf(f, "%s:%d : Beginning with %d args\n", __FILE__, __LINE__, argc);
    for(int i = 0; i < argc; i++)
    {
        fprintf(f, "%d : %s, ", i, argv[i]);
    }
    fprintf(f, "\n");
    fclose(f);
    try {
        return love::initialize(argc, argv);
    } catch (const std::exception &e) {
        love::logError(std::string("Fatal exception: ") + e.what());
    } catch (...) {
        love::logError("Fatal unknown exception");
    }
    return -1;
}
