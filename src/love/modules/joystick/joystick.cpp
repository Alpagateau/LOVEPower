#include "joystick.h"
#include "../wiimote/wiimote.hpp"
#include <sstream>
#include <algorithm>
#include <map>
#include <fstream>
#include <wiiuse/wiiuse.h>

namespace love{
namespace joystick {

// Maintain persistent joystick structures so configuration state doesn't wipe out
static std::vector<love_joystick_t> global_joysticks;
static std::map<std::string, std::string> customMappings;
static bool initialized = false;

static void initJoystickPool() {
  if (initialized) return;
  global_joysticks.clear();

  for(int i = 0; i < 4; i++) {
    love_joystick_t j = {}; 
    j.id = i + 1;
    j.controller = love::wiimote::getWiimote(i + 1);
    j.kind = LOVE_JOYSTICK_WIIMOTE;
    j.orientation = LOVE_ORIENTATION_VERTICAL; // Default
    j.mouseMode = false;                       // Default
    std::ostringstream ss;
    ss << "Wiimote(" << i+1 <<")";
    j.name = ss.str();
    global_joysticks.push_back(j);
  }

  love_joystick_t b = {};
  b.id = 5;
  b.kind = LOVE_JOYSTICK_BALANCE_BOARD;
  b.name = "BalanceBoard";
  b.board = love::wiimote::getBalanceBoard();
  b.orientation = LOVE_ORIENTATION_VERTICAL;
  b.mouseMode = false;
  global_joysticks.push_back(b);

  initialized = true;
}

std::vector<love_joystick_t*> getJoysticks()
{
  initJoystickPool();
  std::vector<love_joystick_t*> active_joysticks;

  for(auto& j : global_joysticks) {
    if (j.kind != LOVE_JOYSTICK_BALANCE_BOARD && j.controller && j.controller->isConnected()) {
      // Dynamic expansion type tracking matching your original pattern
      if(j.controller->hasNunchuk()) 
        j.kind = LOVE_JOYSTICK_WIIMOTE_NUNCHUNK;
      else if(j.controller->hasClassic())
        j.kind = LOVE_JOYSTICK_WIIMOTE_CLASSIC;
      else
        j.kind = LOVE_JOYSTICK_WIIMOTE;

      active_joysticks.push_back(&j);
    } 
    else if (j.kind == LOVE_JOYSTICK_BALANCE_BOARD && j.board && j.board->isConnected()) {
      active_joysticks.push_back(&j);
    }
  }
  return active_joysticks;
}

int getJoystickCound()
{
  int count = 0;
  for(int i = 0; i < 4; i++) {
    if(love::wiimote::getWiimote(i+1)->isConnected()) count++;
  }
  if(love::wiimote::getBalanceBoard()->isConnected()) count++;
  return count;
}

void close(love_joystick_t* j) { (void)j; }
bool open(int j) { return love::wiimote::getWiimote(j)->isConnected(); }

double getAxis(love_joystick_t* t, int idx)
{
  // Reuse your existing getAxes logic to fetch all axis values
  std::vector<double> axes = getAxes(t);
  
  // Safe bounds check before accessing the vector index
  if (idx >= 0 && idx < static_cast<int>(axes.size()))
  {
    return axes[idx];
  }
  
  return 0.0;
}

std::vector<double> getAxes(love_joystick_t* j)
{
  std::vector<double> axes;
  
  if(j->kind == LOVE_JOYSTICK_WIIMOTE || j->kind == LOVE_JOYSTICK_WIIMOTE_NUNCHUNK)
  {
    // Nunchuk expansion populates its analog stick axes first
    if (j->kind == LOVE_JOYSTICK_WIIMOTE_NUNCHUNK) {
      axes.push_back(j->controller->getNunchukJoystickX());
      axes.push_back(j->controller->getNunchukJoystickY());
    }

    double x = 0;
    double y = 0;

    // Intercept and rotate the raw D-Pad vectors if the Wiimote is held horizontally
    if (j->orientation == LOVE_ORIENTATION_HORIZONTAL) {
      if(j->controller->checkButton("down"))  x += 1; // Physical Down points Right
      if(j->controller->checkButton("up"))    x -= 1; // Physical Up points Left
      if(j->controller->checkButton("right")) y += 1; // Physical Right points Up
      if(j->controller->checkButton("left"))  y -= 1; // Physical Left points Down
    } else {
      if(j->controller->checkButton("right")) x += 1;
      if(j->controller->checkButton("left"))  x -= 1;
      if(j->controller->checkButton("up"))    y += 1;
      if(j->controller->checkButton("down"))  y -= 1; 
    }
    axes.push_back(x);
    axes.push_back(y);
  }
  else if(j->kind == LOVE_JOYSTICK_WIIMOTE_CLASSIC)
  {
    axes.push_back(j->controller->getClassicLeftJoystickX());
    axes.push_back(j->controller->getClassicLeftJoystickY());
    axes.push_back(j->controller->getClassicRightJoystickX());
    axes.push_back(j->controller->getClassicRightJoystickY());
    
    double x = 0;
    double y = 0;
    if(j->controller->checkButton("classic_right")) x += 1;
    if(j->controller->checkButton("classic_left"))  x -= 1;
    if(j->controller->checkButton("classic_up"))    y += 1;
    if(j->controller->checkButton("classic_down"))  y -= 1; 
    axes.push_back(x);
    axes.push_back(y);
  }
  else if(j->kind == LOVE_JOYSTICK_BALANCE_BOARD) 
  { 
    axes.push_back(j->board->getBalanceX());
    axes.push_back(j->board->getCenterOfBalanceY());
  }
  return axes;
}

bool isDown(love_joystick_t* t, std::string button)
{
  if(t->kind == LOVE_JOYSTICK_BALANCE_BOARD) return false;
  
  std::string lowerButton = button;
  std::transform(lowerButton.begin(), lowerButton.end(), lowerButton.begin(), ::tolower);

  // If running sideways layout, translate the logical direction demands into rotated hardware clicks
  if (t->orientation == LOVE_ORIENTATION_HORIZONTAL) {
    if (lowerButton == "up")    return t->controller->checkButton("right");
    if (lowerButton == "down")  return t->controller->checkButton("left");
    if (lowerButton == "left")  return t->controller->checkButton("up");
    if (lowerButton == "right") return t->controller->checkButton("down");
    
    // Optional comfortable remapping: make 2 act as Main Action (A) and 1 act as Cancel (B)
    if (lowerButton == "a")     return t->controller->checkButton("2");
    if (lowerButton == "b")     return t->controller->checkButton("1");
  }

  return t->controller->checkButton(button);
}

// --- Configuration Setters and Getters ---
void setOrientation(love_joystick_t* t, love_orientation_t orientation) { t->orientation = orientation; }
love_orientation_t getOrientation(love_joystick_t* t) { return t->orientation; }
void setMouseMode(love_joystick_t* t, bool enable) { t->mouseMode = enable; }
bool isMouseModeEnabled(love_joystick_t* t) { return t->mouseMode; }

int getAxisCound(love_joystick_t* t) {
  switch(t->kind) {
    case LOVE_JOYSTICK_WIIMOTE:           return 2;
    case LOVE_JOYSTICK_BALANCE_BOARD:     return 2;
    case LOVE_JOYSTICK_WIIMOTE_NUNCHUNK:  return 4;
    case LOVE_JOYSTICK_WIIMOTE_CLASSIC:   return 8;
    default:                              return 0;
  }
}

int getButtonCount(love_joystick_t* t) {
  switch(t->kind) {
    case LOVE_JOYSTICK_WIIMOTE:           return 11;
    case LOVE_JOYSTICK_BALANCE_BOARD:     return 0;
    case LOVE_JOYSTICK_WIIMOTE_NUNCHUNK:  return 13;
    case LOVE_JOYSTICK_WIIMOTE_CLASSIC:   return 15;
    default:                              return 0;
  }
}

bool isConnected(love_joystick_t* t) {
  if(t->kind == LOVE_JOYSTICK_BALANCE_BOARD) return t->board->isConnected();
  return t->controller->isConnected();
}

bool isGamepad(love_joystick_t *t)
{
  return t->kind != LOVE_JOYSTICK_BALANCE_BOARD;
}

static std::string getPhysicalButtonName(love_joystick_type_t kind, std::string gpButton)
{
    std::string button = gpButton;
    std::transform(button.begin(), button.end(), button.begin(), ::tolower);

    // Look up custom overrides first (e.g., "classic_a" mapped differently)
    std::string lookupKey = std::to_string(kind) + "_" + button;
    if (customMappings.find(lookupKey) != customMappings.end()) {
        return customMappings[lookupKey];
    }

    // Fallback to Sane Hardware Defaults
    if (kind == LOVE_JOYSTICK_WIIMOTE_CLASSIC) {
        if (button == "a") return "classic_a";
        if (button == "b") return "classic_b";
        if (button == "x") return "classic_x";
        if (button == "y") return "classic_y";
        if (button == "back") return "classic_minus";
        if (button == "start") return "classic_plus";
        if (button == "guide") return "classic_home";
        if (button == "dpaddup" || button == "up") return "classic_up";
        if (button == "dpaddown" || button == "down") return "classic_down";
        if (button == "dpadleft" || button == "left") return "classic_left";
        if (button == "dpadright" || button == "right") return "classic_right";
        if (button == "leftshoulder") return "classic_l";
        if (button == "rightshoulder") return "classic_r";
    } 
    else if (kind == LOVE_JOYSTICK_WIIMOTE_NUNCHUNK) {
        if (button == "a") return "a";
        if (button == "b") return "b";
        if (button == "x") return "1";
        if (button == "y") return "2";
        if (button == "back") return "minus";
        if (button == "start") return "plus";
        if (button == "guide") return "home";
        if (button == "dpaddup" || button == "up") return "up";
        if (button == "dpaddown" || button == "down") return "down";
        if (button == "dpadleft" || button == "left") return "left";
        if (button == "dpadright" || button == "right") return "right";
        if (button == "leftshoulder") return "c";
    } 
    else if (kind == LOVE_JOYSTICK_WIIMOTE) {
        // Standard vertical/default Wiimote binding map
        if (button == "a") return "a";
        if (button == "b") return "b";
        if (button == "x") return "1";
        if (button == "y") return "2";
        if (button == "back") return "minus";
        if (button == "start") return "plus";
        if (button == "guide") return "home";
        if (button == "dpaddup" || button == "up") return "up";
        if (button == "dpaddown" || button == "down") return "down";
        if (button == "dpadleft" || button == "left") return "left";
        if (button == "dpadright" || button == "right") return "right";
    }

    return button; // If no match found, fallback directly to the string provided
}

bool isGamepadDown(love_joystick_t* t, std::string button)
{
    if (!isGamepad(t)) return false;
    std::string physicalName = getPhysicalButtonName(t->kind, button);
    return t->controller->checkButton(physicalName);
}

double getGamepadAxis(love_joystick_t* t, std::string axis)
{
    if (!isGamepad(t)) return 0.0;
    
    std::transform(axis.begin(), axis.end(), axis.begin(), ::tolower);

    if (t->kind == LOVE_JOYSTICK_WIIMOTE_CLASSIC) {
        if (axis == "leftx")  return t->controller->getClassicLeftJoystickX();
        if (axis == "lefty")  return t->controller->getClassicLeftJoystickY();
        if (axis == "rightx") return t->controller->getClassicRightJoystickX();
        if (axis == "righty") return t->controller->getClassicRightJoystickY();
        if (axis == "triggerleft")  return t->controller->getClassicLeftShoulder();
        if (axis == "triggerright") return t->controller->getClassicRightShoulder();
    } 
    else if (t->kind == LOVE_JOYSTICK_WIIMOTE_NUNCHUNK) {
        if (axis == "leftx") return t->controller->getNunchukJoystickX();
        if (axis == "lefty") return t->controller->getNunchukJoystickY();
        // Nunchuk Z button is digital but commonly acts as a trigger analog fallback
        if (axis == "triggerleft") return t->controller->checkButton("z") ? 1.0 : 0.0;
    }

    return 0.0;
}

void loadGamepadMappings(std::string filename_or_string)
{
    // Allows loading simple mapping configuration lines
    // Expected format per line: controller_type,gamepad_button:hardware_button
    // Example: 2,a:classic_b  (Where 2 is LOVE_JOYSTICK_WIIMOTE_CLASSIC)
    
    std::istream* stream;
    std::ifstream file(filename_or_string);
    std::stringstream sstream;

    if (file.is_open()) {
        stream = &file;
    } else {
        sstream << filename_or_string;
        stream = &sstream;
    }

    std::string line;
    while (std::getline(*stream, line)) {
        // Remove spaces
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line.empty() || line[0] == '#') continue;

        size_t commaPos = line.find(',');
        size_t colonPos = line.find(':');
        if (commaPos != std::string::npos && colonPos != std::string::npos) {
            std::string kindStr = line.substr(0, commaPos);
            std::string gpButton = line.substr(commaPos + 1, colonPos - (commaPos + 1));
            std::string hwButton = line.substr(colonPos + 1);

            std::string lookupKey = kindStr + "_" + gpButton;
            customMappings[lookupKey] = hwButton;
        }
    }
}

}}
