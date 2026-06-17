#ifndef LOVE_JOYSTICK_H
#define LOVE_JOYSTICK_H

#include <string>
#include <vector>
#include "../../modules/wiimote/classes/wiimoteController.hpp"
#include "../../modules/wiimote/classes/balanceBoard.hpp"

namespace love{
namespace joystick{

typedef enum {
  LOVE_JOYSTICK_WIIMOTE,
  LOVE_JOYSTICK_WIIMOTE_NUNCHUNK,
  LOVE_JOYSTICK_WIIMOTE_CLASSIC,
  LOVE_JOYSTICK_BALANCE_BOARD
} love_joystick_type_t;

// Custom Enum to handle sideways vs vertical remote orientation
typedef enum {
  LOVE_ORIENTATION_VERTICAL,
  LOVE_ORIENTATION_HORIZONTAL
} love_orientation_t;

typedef struct { 
  love_joystick_type_t kind;
  std::string name;
  int id;                         // Physical index (1 to 4)
  love_orientation_t orientation; // Tracking orientation state
  bool mouseMode;                 // Track whether IR controls the mouse pointer
  union { 
    love::wiimote::WiimoteController* controller;
    love::wiimote::BalanceBoard* board;
  };
} love_joystick_t;

std::vector<love_joystick_t*> getJoysticks();
int getJoystickCound(); // Kept your original spelling to match your codebase
std::string getGamepadMappingString();
void close(love_joystick_t* j);
bool open (int j);

std::vector<double> getAxes(love_joystick_t* j);
double getAxis(love_joystick_t* t, int idx);
int getAxisCound(love_joystick_t* t);
int getButtonCount(love_joystick_t* t);

bool isConnected(love_joystick_t* t);
bool isDown(love_joystick_t* t, std::string button);

void setOrientation(love_joystick_t* t, love_orientation_t orientation);
love_orientation_t getOrientation(love_joystick_t* t);
void setMouseMode(love_joystick_t* t, bool enable);
bool isMouseModeEnabled(love_joystick_t* t);

bool isGamepad(love_joystick_t* t);
bool isGamepadDown(love_joystick_t* t, std::string button);
double getGamepadAxis(love_joystick_t* t, std::string axis);
void loadGamepadMappings(std::string filename_or_string);

}}

#endif
