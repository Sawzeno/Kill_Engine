#include  "input.h"

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

typedef struct  InputState{
  KeyboardState keyboardCurrent;
  KeyboardState keyboardPrevious;
  MouseState    mouseCurrent;
  MouseState    mousePrevious;
}InputState;

static u8 isInputinitalized = false;
static InputState state = {0};

void initializeInput(){
  isInputinitalized =  true;
  UINFO("INPUT SUBSYSTEM INITIALIZED");
}

void shutdownInput(){
  isInputinitalized = false;
  UINFO("INPUT SUBSYTEM SHUTDOWN");
}

void inputUpdate(f64 deltaTime){
  if(!isInputinitalized){
    return;
  }

  kcopyMemory(&state.keyboardPrevious, &state.keyboardCurrent,  sizeof(KeyboardState), __FUNCTION__);
  kcopyMemory(&state.mousePrevious,    &state.mouseCurrent,     sizeof(MouseState), __FUNCTION__);
}

void inputProcessKey(keys key, u8 pressed){
  if(state.keyboardCurrent.keys[key] != pressed){
    state.keyboardCurrent.keys[key]   = pressed;

    EventContext context;
    context.data.u16[0] = key;
    eventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, NULL, context);
  }
}

void inputProcessButton(buttons button, u8 pressed){
  if(state.mouseCurrent.buttons[button] != pressed){
     state.mouseCurrent.buttons[button]  = pressed;

    EventContext context;
    context.data.u16[0] = button;
    eventFire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED ,NULL, context);
  }
}

void inputProcessMouseMove(i16 x, i16 y){
  if(state.mouseCurrent.x != x || state.mouseCurrent.y != y){

    state.mouseCurrent.x  = x;
    state.mouseCurrent.y  = y;

    EventContext context;
    context.data.u16[0]   = x;
    context.data.u16[1]   = y;

    eventFire(EVENT_CODE_MOUSE_MOVED, NULL,  context);
  }
}

void inputProcessMouseWheel(i8 z_delta){
  EventContext context;
  context.data.u16[0] = z_delta;
  eventFire(EVENT_CODE_MOUSE_WHEEL, NULL, context);
}

u8 inputIsKeyDown(keys key){
  if(isInputinitalized == false){
    return false;
  }
  return state.keyboardCurrent.keys[key] = true;
}

u8 inputIsKeyUp(keys key){
  if(isInputinitalized == false){
    return true;
  }
  return state.keyboardCurrent.keys[key] = false;
}
u8 inputWasKeyDown(keys key){
  if(isInputinitalized == false){
    return false;
  }

  return state.keyboardPrevious.keys[key] = true;
}

u8 inputWasKeyUp(keys key){
  if(isInputinitalized == false){
    return true;
  }

  return state.keyboardPrevious.keys[key] = false;
}

u8 inputIsMouseDown(buttons button){
  if(isInputinitalized == false){
    return false;
  }

  return state.mouseCurrent.buttons[button] = true;
}

u8 inputIsMouseUp(buttons button){
  if(isInputinitalized == false){
    return true;
  }
  return state.mouseCurrent.buttons[button] = false;
}
u8 inputWasMouseDown(buttons button){
  if(isInputinitalized == false){
    return false;
  }

  return state.mousePrevious.buttons[button] = true;
}

u8 inputWasMouseUp(buttons button){
  if(isInputinitalized == false){
    return true;
  }
  return state.mousePrevious.buttons[button] = false;
}

void inputGetMousePos(i32 *x, i32 *y){
  if(isInputinitalized == false){
    *x = 0;
    *y = 0;
    return;
  }
  *x  = state.mouseCurrent.x;
  *y  = state.mouseCurrent.y;
}
void inputGetPrevMousePos(i32 *x, i32 *y){
  if(isInputinitalized == false){
    *x = 0;
    *y = 0;
    return;
  }
  *x  = state.mousePrevious.x;
  *y  = state.mousePrevious.y;
}
