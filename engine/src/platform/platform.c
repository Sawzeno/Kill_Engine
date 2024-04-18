#include  "platform.h"
#include  "core/input.h"
#include  "defines.h"

#include  <X11/X.h>
#include  <xcb/xcb.h>
#include  <xcb/xproto.h>
#include  <X11/keysym.h>
#include  <X11/XKBlib.h>
#include  <X11/Xlib.h>
#include  <X11/Xlib-xcb.h>
#include  <sys/time.h>

#include  <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <string.h>
#include  <bits/time.h>
#include  <time.h>

typedef struct internalState internalState;

struct internalState{
  Display*          display;
  xcb_connection_t* connection;
  xcb_window_t      window;
  xcb_screen_t*     screen;
  xcb_atom_t        wmProtocols;
  xcb_atom_t        wmDeleteWin;
};

u8  translateKeycode(u32 xKeycode);

u8  startPlatform(platformState* platState,
                  const char* appliactionName,
                  i32 x,
                  i32 y,
                  i32 width,
                  i32 height){
  //create internal state
  platState->internalState  = malloc(sizeof(internalState));
  MEMERR(platState->internalState)
  internalState*  state = (internalState*)(platState->internalState);

  //connect to Xserver
  state->display  = XOpenDisplay(NULL);

  //Turn off keyrepeats
  XAutoRepeatOff(state->display);

  //Retrieve the connection from display
  state->connection = XGetXCBConnection(state->display);
  if(xcb_connection_has_error(state->connection)){
    UFATAL("Failed to connect to X server via XCB");
    return false;
  }

  //get data from the Xserver
  const struct xcb_setup_t *setup = xcb_get_setup(state->connection);

  //loop through screens using iterator
  u8 screenP  = 0;
  xcb_screen_iterator_t it  = xcb_setup_roots_iterator (setup);
  for(i32 s = screenP ; s > 0 ; --s){
    xcb_screen_next(&it);
  }
  //after looping till the screen assign it 
  state->screen = it.data;

  //ask for a new ID for the window
  state->window = xcb_generate_id(state->connection);

  //Register event types
  //EVENT_MASK is always required , BACK_PIXEL is to blackout the screen 
  u32 eventMask  = XCB_CW_EVENT_MASK | XCB_CW_BACK_PIXEL;

  //listen for keyboard and mouse buttons
  u32 eventValues=
    XCB_EVENT_MASK_EXPOSURE       | XCB_EVENT_MASK_BUTTON_PRESS   |
    XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
    XCB_EVENT_MASK_ENTER_WINDOW   | XCB_EVENT_MASK_LEAVE_WINDOW   |
    XCB_EVENT_MASK_KEY_PRESS      | XCB_EVENT_MASK_KEY_RELEASE;
  //values to be sent over to XCB
  u32 valueList[] = {state->screen->black_pixel , eventValues};

  //create the window
  xcb_create_window (state->connection,                             /* Connection          */
                     0,                                             /* depth               */
                     state->window,                                 /* window Id           */
                     state->screen->root,                           /* parent window       */
                     x, y,                                          /* x, y                */
                     width, height,                                 /* width, height       */
                     0,                                             /* border_width        */
                     XCB_WINDOW_CLASS_INPUT_OUTPUT,                 /* class               */
                     state->screen->root_visual,                    /* visual              */
                     eventMask, valueList);                         /* masks */

  //change title of window
  xcb_change_property(state->connection,
                      XCB_PROP_MODE_REPLACE,
                      state->window,
                      XCB_ATOM_WM_NAME,
                      XCB_ATOM_STRING,
                      8,
                      strlen(appliactionName),
                      appliactionName);
  //notify at deleteing the window
  xcb_intern_atom_cookie_t  wmDeleteCookie    = xcb_intern_atom(state->connection,
                                                                0,
                                                                strlen("WM_DELETE_WINDOW"),
                                                                "WM_DELETE_WINDOW");
  xcb_intern_atom_cookie_t  wmProtocolsCookie = xcb_intern_atom(state->connection,
                                                                0,
                                                                strlen("WM_PROTOCOLS"),
                                                                "WM_PROTOCOLS");
  xcb_intern_atom_reply_t   *wmDeleteReply    = xcb_intern_atom_reply(state->connection,
                                                                      wmDeleteCookie,
                                                                      NULL);
  xcb_intern_atom_reply_t   *wmProtocolsReply = xcb_intern_atom_reply(state->connection,
                                                                      wmProtocolsCookie,
                                                                      NULL);
  state->wmDeleteWin  = wmDeleteReply->atom;
  state->wmProtocols  = wmProtocolsReply->atom;

  xcb_change_property(state->connection,
                      XCB_PROP_MODE_REPLACE,
                      state->window,
                      wmProtocolsReply->atom,
                      4,
                      32,
                      1,
                      &wmDeleteReply->atom);

  xcb_map_window(state->connection, state->window);

  i32 streamResult  = xcb_flush(state->connection);
  if(streamResult <=  0 ){
    UFATAL("an error occured when flushing the stream %d" , streamResult);
    return false;
  }

  UINFO("WINDOWING SUBSYSTEM INITIALIZED")
  return true;
}

void  shutdownPlatform    (platformState* platState){
  UINFO("WINDOWING SUBSYTEM SHUTDOWN");

  internalState*  state = (internalState*)(platState->internalState);

  UTRACE("RESTORING STATES");
  //turn repeat keys back ON
  XAutoRepeatOn(state->display);

  UTRACE("DESTROYING WINDOW");
  //destroy window
  xcb_destroy_window(state->connection, state->window);
}

u8    platformPumpMessages(platformState* platState){

  internalState*  state = (internalState*)(platState->internalState);
  xcb_generic_event_t* event;
  xcb_client_message_event_t* cm;

  u8 quitFlagged  = false;

  //poll for events untill NULL returned
  while(event != 0){
    event = xcb_poll_for_event(state->connection);
    UTRACE("EVENT POLLED")
    if(event  ==  0){
      break;
    }

    switch (event->response_type & ~0x80) {
      case XCB_KEY_PRESS:
      case XCB_KEY_RELEASE: {
        UTRACE("PLATFORM KEY PRESSED OR RELEASED")
        xcb_key_release_event_t* keyboardEvent = (xcb_key_release_event_t*)event;
        u8  pressed         = event->response_type  == XCB_KEY_PRESS;
        xcb_keycode_t code  = keyboardEvent->detail;
        KeySym keysym       = XkbKeycodeToKeysym(state->display, (KeyCode)code, 0, code & ShiftMask ? 1 : 0);
        keys key            = translateKeycode(keysym);
        inputProcessKey(key, pressed);
      } break;

      case  XCB_BUTTON_PRESS:
      case  XCB_BUTTON_RELEASE:{
        UTRACE("PLATFROM MOUSE PRESSED OR RELEASED")
        xcb_button_press_event_t* mouseEvent =   (xcb_button_press_event_t*)event;
        u8  pressed = event->response_type  ==  XCB_BUTTON_PRESS;
        buttons mouseButton = BUTTON_MAX_BUTTONS;
        switch(mouseEvent->detail){
          case XCB_BUTTON_INDEX_1 :
            mouseButton = BUTTON_LEFT;
          case XCB_BUTTON_INDEX_2 :
            mouseButton = BUTTON_MIDDLE;
          case XCB_BUTTON_INDEX_3 :
            mouseButton = BUTTON_LEFT;
        }

        if(mouseButton != BUTTON_MAX_BUTTONS){
          UTRACE("PLATFORM MAX BUTTONS PRESSED")
          inputProcessButton(mouseButton, pressed);
        }
      }break;
      case  XCB_MOTION_NOTIFY:{
        UTRACE("PLATFORM MOTION NOTIFY")
        xcb_motion_notify_event_t* moveEvent = (xcb_motion_notify_event_t*)event;
        inputProcessMouseMove(moveEvent->event_x, moveEvent->event_y);
        break;
      }
      case  XCB_CONFIGURE_NOTIFY:
        //resizing
        break;

      case  XCB_CLIENT_MESSAGE:{
        UTRACE("PLATFORM CLIENT NOTIFY")
        cm  = (xcb_client_message_event_t*)event;
        if(cm->data.data32[0] ==  state->wmDeleteWin){
          quitFlagged = true;
        }
      }break;
      default:
        //something else
        break;
    }

    free(event);
  }

  return !quitFlagged;
}

void  platformSleep(u64 ms){
  if(ms >= 1000){
    sleep(ms/1000);
  } 
  usleep((ms%1000) * 1000);
}

f64   platformGetAbsoluteTime(){
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return  now.tv_sec + now.tv_nsec * 0.000000001;
}

u8  translateKeycode(u32 xKeycode){
  switch (xKeycode) {
    case XK_BackSpace:
      return KEY_BACKSPACE;
    case XK_Return:
      return KEY_ENTER;
    case XK_Tab:
      return KEY_TAB;
    //case XK_Shift: return KEY_SHIFT;
    //case XK_Control: return KEY_CONTROL;

    case XK_Pause:
      return KEY_PAUSE;
    case XK_Caps_Lock:
      return KEY_CAPITAL;

    case XK_Escape:
      return KEY_ESCAPE;

    // Not supported
    // case : return KEY_CONVERT;
    // case : return KEY_NONCONVERT;
    // case : return KEY_ACCEPT;

    case XK_Mode_switch:
      return KEY_MODECHANGE;

    case XK_space:
      return KEY_SPACE;
    case XK_Prior:
      return KEY_PRIOR;
    case XK_Next:
      return KEY_NEXT;
    case XK_End:
      return KEY_END;
    case XK_Home:
      return KEY_HOME;
    case XK_Left:
      return KEY_LEFT;
    case XK_Up:
      return KEY_UP;
    case XK_Right:
      return KEY_RIGHT;
    case XK_Down:
      return KEY_DOWN;
    case XK_Select:
      return KEY_SELECT;
    case XK_Print:
      return KEY_PRINT;
    case XK_Execute:
      return KEY_EXECUTE;
    // case XK_snapshot: return KEY_SNAPSHOT; // not supported
    case XK_Insert:
      return KEY_INSERT;
    case XK_Delete:
      return KEY_DELETE;
    case XK_Help:
      return KEY_HELP;

    case XK_Meta_L:
      return KEY_LWIN;  // TODO: not sure this is right
    case XK_Meta_R:
      return KEY_RWIN;
    // case XK_apps: return KEY_APPS; // not supported

    // case XK_sleep: return KEY_SLEEP; //not supported

    case XK_KP_0:
      return KEY_NUMPAD0;
    case XK_KP_1:
      return KEY_NUMPAD1;
    case XK_KP_2:
      return KEY_NUMPAD2;
    case XK_KP_3:
      return KEY_NUMPAD3;
    case XK_KP_4:
      return KEY_NUMPAD4;
    case XK_KP_5:
      return KEY_NUMPAD5;
    case XK_KP_6:
      return KEY_NUMPAD6;
    case XK_KP_7:
      return KEY_NUMPAD7;
    case XK_KP_8:
      return KEY_NUMPAD8;
    case XK_KP_9:
      return KEY_NUMPAD9;
    case XK_multiply:
      return KEY_MULTIPLY;
    case XK_KP_Add:
      return KEY_ADD;
    case XK_KP_Separator:
      return KEY_SEPARATOR;
    case XK_KP_Subtract:
      return KEY_SUBTRACT;
    case XK_KP_Decimal:
      return KEY_DECIMAL;
    case XK_KP_Divide:
      return KEY_DIVIDE;
    case XK_F1:
      return KEY_F1;
    case XK_F2:
      return KEY_F2;
    case XK_F3:
      return KEY_F3;
    case XK_F4:
      return KEY_F4;
    case XK_F5:
      return KEY_F5;
    case XK_F6:
      return KEY_F6;
    case XK_F7:
      return KEY_F7;
    case XK_F8:
      return KEY_F8;
    case XK_F9:
      return KEY_F9;
    case XK_F10:
      return KEY_F10;
    case XK_F11:
      return KEY_F11;
    case XK_F12:
      return KEY_F12;
    case XK_F13:
      return KEY_F13;
    case XK_F14:
      return KEY_F14;
    case XK_F15:
      return KEY_F15;
    case XK_F16:
      return KEY_F16;
    case XK_F17:
      return KEY_F17;
    case XK_F18:
      return KEY_F18;
    case XK_F19:
      return KEY_F19;
    case XK_F20:
      return KEY_F20;
    case XK_F21:
      return KEY_F21;
    case XK_F22:
      return KEY_F22;
    case XK_F23:
      return KEY_F23;
    case XK_F24:
      return KEY_F24;

    case XK_Num_Lock:
      return KEY_NUMLOCK;
    case XK_Scroll_Lock:
      return KEY_SCROLL;

    case XK_KP_Equal:
      return KEY_NUMPAD_EQUAL;

    case XK_Shift_L:
      return KEY_LSHIFT;
    case XK_Shift_R:
      return KEY_RSHIFT;
    case XK_Control_L:
      return KEY_LCONTROL;
    case XK_Control_R:
      return KEY_RCONTROL;
    // case XK_Menu: return KEY_LMENU;
    case XK_Menu:
      return KEY_RMENU;

    case XK_semicolon:
      return KEY_SEMICOLON;
    case XK_plus:
      return KEY_PLUS;
    case XK_comma:
      return KEY_COMMA;
    case XK_minus:
      return KEY_MINUS;
    case XK_period:
      return KEY_PERIOD;
    case XK_slash:
      return KEY_SLASH;
    case XK_grave:
      return KEY_GRAVE;

    case XK_a:
    case XK_A:
      return KEY_A;
    case XK_b:
    case XK_B:
      return KEY_B;
    case XK_c:
    case XK_C:
      return KEY_C;
    case XK_d:
    case XK_D:
      return KEY_D;
    case XK_e:
    case XK_E:
      return KEY_E;
    case XK_f:
    case XK_F:
      return KEY_F;
    case XK_g:
    case XK_G:
      return KEY_G;
    case XK_h:
    case XK_H:
      return KEY_H;
    case XK_i:
    case XK_I:
      return KEY_I;
    case XK_j:
    case XK_J:
      return KEY_J;
    case XK_k:
    case XK_K:
      return KEY_K;
    case XK_l:
    case XK_L:
      return KEY_L;
    case XK_m:
    case XK_M:
      return KEY_M;
    case XK_n:
    case XK_N:
      return KEY_N;
    case XK_o:
    case XK_O:
      return KEY_O;
    case XK_p:
    case XK_P:
      return KEY_P;
    case XK_q:
    case XK_Q:
      return KEY_Q;
    case XK_r:
    case XK_R:
      return KEY_R;
    case XK_s:
    case XK_S:
      return KEY_S;
    case XK_t:
    case XK_T:
      return KEY_T;
    case XK_u:
    case XK_U:
      return KEY_U;
    case XK_v:
    case XK_V:
      return KEY_V;
    case XK_w:
    case XK_W:
      return KEY_W;
    case XK_x:
    case XK_X:
      return KEY_X;
    case XK_y:
    case XK_Y:
      return KEY_Y;
    case XK_z:
    case XK_Z:
      return KEY_Z;

    default:
      return 0;
  }
}
