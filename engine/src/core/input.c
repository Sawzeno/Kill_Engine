#include  "input.h"

#include  "logger.h"
#include  "events.h"
#include  "kmemory.h"
#include  "defines.h"

typedef struct  KeyboardState{
  u8    keys[KEYS_MAX_KEYS];
}KeyboardState;

typedef struct  MouseState{
  i16   x;
  i16   y;
  u8    buttons[BUTTON_MAX_BUTTONS];
}MouseState;

typedef struct  InputSystemState{
  KeyboardState keyboardCurrent;
  KeyboardState keyboardPrevious;
  MouseState    mouseCurrent;
  MouseState    mousePrevious;
}InputSystemState;

static InputSystemState* inputSystemStatePtr;

void testInput(char c){
  UDEBUG("prev : %d  current : %d", inputSystemStatePtr->keyboardPrevious.keys[c], inputSystemStatePtr->keyboardCurrent.keys[c]);
}
// #ifndef LOG_DEBUG_ENABLED 
char* getKeyName(u32 xKeycode);
// #endif

bool initializeInput(u64* memoryRequirement, void* state){
  TRACEFUNCTION;
  KDEBUG("memoryRequirement :%"PRIu64" state : %p",*memoryRequirement, state);
  *memoryRequirement  = sizeof(InputSystemState);
  if(state  ==  NULL){
    KDEBUG("INPUT SUBSYTEM  : STATE PASSED AS NULL");
    return true;
  }

  inputSystemStatePtr = state;
  KINFO("INPUT SUBSYSTEM INITIALIZED");
  return true;
}

void shutdownInput(){
TRACEFUNCTION;
  KINFO("INPUT SUBSYTEM SHUTDOWN");
  inputSystemStatePtr = NULL;
}

void inputUpdate(f64 deltaTime){
TRACEEVENT;
  if(inputSystemStatePtr  ==  NULL){
    KERROR("INPUT NOT INITIALIZED");
    return;
  }
  kcopyMemory(&inputSystemStatePtr->keyboardPrevious, &inputSystemStatePtr->keyboardCurrent,  sizeof(KeyboardState), __FUNCTION__);
  kcopyMemory(&inputSystemStatePtr->mousePrevious,    &inputSystemStatePtr->mouseCurrent,     sizeof(MouseState), __FUNCTION__);
}

void inputProcessKey(keys key, u8 pressed){
TRACEEVENT;
  //updates the current value ,if wasnt pressed and is pressed then set its to that and vice versa
  EDEBUG("%s : %s",getKeyName(key), pressed == true ? "pressed" : "released");
  if(inputSystemStatePtr->keyboardCurrent.keys[key] != pressed){
    inputSystemStatePtr->keyboardCurrent.keys[key]   = pressed;

    EventContext context;
    context.data.u16[0] = key;
    eventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, NULL, context);
  }
}

void inputProcessButton(buttons button, u8 pressed){
TRACEEVENT;
  if(inputSystemStatePtr->mouseCurrent.buttons[button] != pressed){
    inputSystemStatePtr->mouseCurrent.buttons[button]  = pressed;

    EventContext context;
    context.data.u16[0] = button;
    eventFire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED ,NULL, context);
  }
}

void inputProcessMouseMove(i16 x, i16 y){
TRACEEVENT;
  EDEBUG("X : %"PRIu64" Y : %"PRIu64"",x,y);
  if(inputSystemStatePtr->mouseCurrent.x != x || inputSystemStatePtr->mouseCurrent.y != y){

    inputSystemStatePtr->mouseCurrent.x  = x;
    inputSystemStatePtr->mouseCurrent.y  = y;

    EventContext context;
    context.data.u16[0]   = x;
    context.data.u16[1]   = y;

    eventFire(EVENT_CODE_MOUSE_MOVED, NULL,  context);
  }
}

void inputProcessMouseWheel(i8 z_delta){
TRACEEVENT;
  EventContext context;
  context.data.u16[0] = z_delta;
  eventFire(EVENT_CODE_MOUSE_WHEEL, NULL, context);
}

u8 inputIsKeyDown(keys key){
  TRACEEVENT;
  return inputSystemStatePtr->keyboardCurrent.keys[key] = true;
}

u8 inputIsKeyUp(keys key){
  TRACEEVENT;
  return inputSystemStatePtr->keyboardCurrent.keys[key] == false;
}
u8 inputWasKeyDown(keys key){
  TRACEEVENT;
  return inputSystemStatePtr->keyboardPrevious.keys[key] == true;
}

u8 inputWasKeyUp(keys key){
  TRACEEVENT;
  return inputSystemStatePtr->keyboardPrevious.keys[key] == false;
}

u8 inputIsMouseDown(buttons button){
  TRACEEVENT;
  return inputSystemStatePtr->mouseCurrent.buttons[button] == true;
}

u8 inputIsMouseUp(buttons button){
  TRACEEVENT;
  return inputSystemStatePtr->mouseCurrent.buttons[button] == false;
}
u8 inputWasMouseDown(buttons button){
  TRACEEVENT;
  return inputSystemStatePtr->mousePrevious.buttons[button] == true;
}

u8 inputWasMouseUp(buttons button){
  TRACEEVENT;
  return inputSystemStatePtr->mousePrevious.buttons[button] == false;
}

void inputGetMousePos(i32 *x, i32 *y){
  TRACEEVENT;
  *x  = inputSystemStatePtr->mouseCurrent.x;
  *y  = inputSystemStatePtr->mouseCurrent.y;
}
void inputGetPrevMousePos(i32 *x, i32 *y){
  TRACEEVENT;
  *x  = inputSystemStatePtr->mousePrevious.x;
  *y  = inputSystemStatePtr->mousePrevious.y;
}

#if LOG_DEBUG_ENABLED == 1 
char* getKeyName(u32 key){
switch (key) {
    
      case KEY_BACKSPACE: return "KEY_BACKSPACE";
    
      case KEY_ENTER: return "KEY_ENTER";
    
      case KEY_TAB: return "KEY_TAB";

    
      case KEY_PAUSE: return "KEY_PAUSE";
    
      case KEY_CAPITAL: return "KEY_CAPITAL";

    
      case KEY_ESCAPE: return "KEY_ESCAPE";

    
      case KEY_MODECHANGE: return "KEY_MODECHANGE";

    
      case KEY_SPACE: return "KEY_SPACE";
    
      case KEY_PRIOR: return "KEY_PRIOR";
    
      case KEY_NEXT: return "KEY_NEXT";
    
      case KEY_END: return "KEY_END";
    
      case KEY_HOME: return "KEY_HOME";
    
      case KEY_LEFT: return "KEY_LEFT";
    
      case KEY_UP: return "KEY_UP";
    
      case KEY_RIGHT: return "KEY_RIGHT";
    
      case KEY_DOWN: return "KEY_DOWN";
    
      case KEY_SELECT: return "KEY_SELECT";
    
      case KEY_PRINT: return "KEY_PRINT";
    
      case KEY_EXECUTE: return "KEY_EXECUTE";
    
      case KEY_INSERT: return "KEY_INSERT";
    
      case KEY_DELETE: return "KEY_DELETE";
    
      case KEY_HELP: return "KEY_HELP";

    
      case KEY_LWIN:  // TODO: not sure this is right; return "KEY_LWIN:  // TODO not sure this is right;";
    
      case KEY_RWIN: return "KEY_RWIN";
    
      case KEY_NUMPAD0: return "KEY_NUMPAD0";
    
      case KEY_NUMPAD1: return "KEY_NUMPAD1";
    
      case KEY_NUMPAD2: return "KEY_NUMPAD2";
    
      case KEY_NUMPAD3: return "KEY_NUMPAD3";
    
      case KEY_NUMPAD4: return "KEY_NUMPAD4";
    
      case KEY_NUMPAD5: return "KEY_NUMPAD5";
    
      case KEY_NUMPAD6: return "KEY_NUMPAD6";
    
      case KEY_NUMPAD7: return "KEY_NUMPAD7";
    
      case KEY_NUMPAD8: return "KEY_NUMPAD8";
    
      case KEY_NUMPAD9: return "KEY_NUMPAD9";
    
      case KEY_MULTIPLY: return "KEY_MULTIPLY";
    
      case KEY_ADD: return "KEY_ADD";
    
      case KEY_SEPARATOR: return "KEY_SEPARATOR";
    
      case KEY_SUBTRACT: return "KEY_SUBTRACT";
    
      case KEY_DECIMAL: return "KEY_DECIMAL";
    
      case KEY_DIVIDE: return "KEY_DIVIDE";
    
      case KEY_F1: return "KEY_F1";
    
      case KEY_F2: return "KEY_F2";
    
      case KEY_F3: return "KEY_F3";
    
      case KEY_F4: return "KEY_F4";
    
      case KEY_F5: return "KEY_F5";
    
      case KEY_F6: return "KEY_F6";
    
      case KEY_F7: return "KEY_F7";
    
      case KEY_F8: return "KEY_F8";
    
      case KEY_F9: return "KEY_F9";
    
      case KEY_F10: return "KEY_F10";
    
      case KEY_F11: return "KEY_F11";
    
      case KEY_F12: return "KEY_F12";
    
      case KEY_F13: return "KEY_F13";
    
      case KEY_F14: return "KEY_F14";
    
      case KEY_F15: return "KEY_F15";
    
      case KEY_F16: return "KEY_F16";
    
      case KEY_F17: return "KEY_F17";
    
      case KEY_F18: return "KEY_F18";
    
      case KEY_F19: return "KEY_F19";
    
      case KEY_F20: return "KEY_F20";
    
      case KEY_F21: return "KEY_F21";
    
      case KEY_F22: return "KEY_F22";
    
      case KEY_F23: return "KEY_F23";
    
      case KEY_F24: return "KEY_F24";

    
      case KEY_NUMLOCK: return "KEY_NUMLOCK";
    
      case KEY_SCROLL: return "KEY_SCROLL";

    
      case KEY_NUMPAD_EQUAL: return "KEY_NUMPAD_EQUAL";

    
      case KEY_LSHIFT: return "KEY_LSHIFT";
    
      case KEY_RSHIFT: return "KEY_RSHIFT";
    
      case KEY_LCONTROL: return "KEY_LCONTROL";
    
      case KEY_CONTROL: return "KEY_CONTROL";
    
      case KEY_LALT: return "KEY_LALT";
    
      case KEY_RALT: return "KEY_RALT";

    
      case KEY_SEMICOLON: return "KEY_SEMICOLON";
    
      case KEY_PLUS: return "KEY_PLUS";
    
      case KEY_COMMA: return "KEY_COMMA";
    
      case KEY_MINUS: return "KEY_MINUS";
    
      case KEY_PERIOD: return "KEY_PERIOD";
    
      case KEY_SLASH: return "KEY_SLASH";
    
      case KEY_GRAVE: return "KEY_GRAVE";

    
    
      case KEY_A: return "KEY_A";
    
    
      case KEY_B: return "KEY_B";
    
    
      case KEY_C: return "KEY_C";
    
    
      case KEY_D: return "KEY_D";
    
    
      case KEY_E: return "KEY_E";
    
    
      case KEY_F: return "KEY_F";
    
    
      case KEY_G: return "KEY_G";
    
    
      case KEY_H: return "KEY_H";
    
    
      case KEY_I: return "KEY_I";
    
    
      case KEY_J: return "KEY_J";
    
    
      case KEY_K: return "KEY_K";
    
    
      case KEY_L: return "KEY_L";
    
    
      case KEY_M: return "KEY_M";
    
    
      case KEY_N: return "KEY_N";
    
    
      case KEY_O: return "KEY_O";
    
    
      case KEY_P: return "KEY_P";
    
    
      case KEY_Q: return "KEY_Q";
    
    
      case KEY_R: return "KEY_R";
    
    
      case KEY_S: return "KEY_S";
    
    
      case KEY_T: return "KEY_T";
    
    
      case KEY_U: return "KEY_U";
    
    
      case KEY_V: return "KEY_V";
    
    
      case KEY_W: return "KEY_W";
    
    
      case KEY_X: return "KEY_X";
    
    
      case KEY_Y: return "KEY_Y";
    
    
      case KEY_Z: return "KEY_Z";

    default:
      return 0;
  }
}
#endif

