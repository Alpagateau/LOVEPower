#include <FreeTypeGX/FreeTypeGX.h>
#include <filesystem>
#include <sol/sol.hpp>
#include <vector>
#include <stdio.h>
#include "../../../modules/filesystem/filesystem.hpp"
#include <grrlib.h>

#include "font.hpp"

#include "Vera_ttf.h"

#define DEFAULT_FONT_SIZE 12

namespace love {
namespace graphics {


void Font::_createFont(const uint8_t *newFont, int dataSize, int size) {
  font = new FreeTypeGX();
  font->loadFont(newFont, dataSize, size);
}

Font::Font() {
  _createFont(Vera_ttf, static_cast<int>(Vera_ttf_size), DEFAULT_FONT_SIZE);
}

Font::Font(int size) {
  _createFont(Vera_ttf, static_cast<int>(Vera_ttf_size), size);
}

Font::Font(std::string file) {
  uint8_t *data = nullptr;
  int dataSize = GRRLIB_LoadFile(("sd://LOVEPower/" + filesystem::getFilePath(file)).c_str(), &data);
  if (dataSize <= 0) {
    switch(dataSize){
      case 0:
        throw std::runtime_error("Empty File: " + filesystem::getFilePath(file));
        break;
      case -1:
        throw std::runtime_error("File Not Found: " + filesystem::getFilePath(file));
        break;
      case -2:
        throw std::runtime_error("OutOfMemory: " + filesystem::getFilePath(file));
        break;
      case -3: 
        throw std::runtime_error("FileReadError: " + filesystem::getFilePath(file));
        break;
    }
  }
  _createFont(data, dataSize, DEFAULT_FONT_SIZE);
}

Font::Font(std::string file, int size) {
  uint8_t *data = nullptr;
  std::string f = "sd:/LOVEPower/" + filesystem::getFilePath(file);
  printf("[FONT] file %s exists ? : %d\n", f.c_str(), std::filesystem::exists(f.c_str()));
  int dataSize = GRRLIB_LoadFile(f.c_str(), &data);
  if (dataSize <= 0) {
    switch(dataSize){
      case 0:
        throw std::runtime_error("Empty File: " + filesystem::getFilePath(file));
        break;
      case -1:
        throw std::runtime_error("File Not Found: " + filesystem::getFilePath(file));
        break;
      case -2:
        throw std::runtime_error("OutOfMemory: " + filesystem::getFilePath(file));
        break;
      case -3: 
        throw std::runtime_error("FileReadError: " + filesystem::getFilePath(file));
        break;
    }
  }
  _createFont(data, dataSize, size);
}

int Font::getWidth(std::string text) {
  std::vector<wchar_t> wide = utf8_to_wchar_vec(text);
  return font->getWidth(wide.data());
}

int Font::getHeight() {
  std::vector<wchar_t> wide = utf8_to_wchar_vec("");
  return font->getHeight(wide.data());
}
} // namespace graphics
} // namespace love
