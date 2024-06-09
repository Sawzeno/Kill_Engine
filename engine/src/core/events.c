#include  "events.h"
#include  "containers/darray.h"
#include  "kmemory.h"
#include  "defines.h"

typedef struct registeredEvent registeredEvent;
typedef struct eventCodeEntry eventCodeEntry;
typedef struct eventSystemState eventSystemState;

struct registeredEvent{
  void* listener;
  pfnOnEvent  callback;
};

struct eventCodeEntry{
  registeredEvent* events;
};

#define MAX_MESSAGE_CODES 128

struct eventSystemState{
  //lookup table for event codes
  eventCodeEntry registered[MAX_MESSAGE_CODES];
};

static u8 areEventsInitialized  = false;
static eventSystemState state;

u8  initializeEvents(){
  if(areEventsInitialized == true){
    return false;
  }
 
  areEventsInitialized = false;
  kzeroMemory(&state , sizeof(state));
  areEventsInitialized = true;
  
  UINFO("EVENT SUBSYSTEM INITIALIZED !");
  return true;
}

void  shutdownEvents(){
  UINFO("EVENT SUBSYSTEM SHUTDOWN !");
  for(u16 i = 0 ; i < MAX_MESSAGE_CODES ; ++i){
    if(state.registered[i].events != 0){
      _darrayDestroy(state.registered[i].events);
      state.registered[i].events  = 0;
    }
  }
}

u8 eventRegister(u16 code, void* listener, pfnOnEvent onEvent) {
    if(areEventsInitialized == false) {
        return false;
    }

    if(state.registered[code].events == 0) {
        state.registered[code].events = darrayCreate(registeredEvent);
    }

    u64 registeredCount = darrayLength(state.registered[code].events);
    for(u64 i = 0; i < registeredCount; ++i) {
        if(state.registered[code].events[i].listener == listener) {
            UREPORT("TODO");
            return false;
        }
    }

    // If at this point, no duplicate was found. Proceed with registration.
    registeredEvent event;
    event.listener = listener;
    event.callback = onEvent;
    darrayPush(state.registered[code].events, event);

    return true;
}

u8 eventUnregister(u16 code, void* listener, pfnOnEvent onEvent) {
    if(areEventsInitialized == false) {
        return false;
    }

    // On nothing is registered for the code, boot out.
    if(state.registered[code].events == 0) {
        // TODO: warn
        return false;
    }

    u64 registeredCount = darrayLength(state.registered[code].events);
    for(u64 i = 0; i < registeredCount; ++i) {
        registeredEvent e = state.registered[code].events[i];
        if(e.listener == listener && e.callback == onEvent) {
            // Found one, remove it
            registeredEvent popped_event;
            darrayPopAt(state.registered[code].events, i, &popped_event);
            return true;
        }
    }

    // Not found.
    return false;
}

u8 eventFire(u16 code, void* sender, eventContext context) {
    if(areEventsInitialized == false) {
        return false;
    }

    // If nothing is registered for the code, boot out.
    if(state.registered[code].events == 0) {
        return false;
    }

    u64 registeredCount = darrayLength(state.registered[code].events);
    for(u64 i = 0; i < registeredCount; ++i) {
        registeredEvent e = state.registered[code].events[i];
        if(e.callback(code, sender, e.listener, context)) {
            // Message has been handled, do not send to other listeners.
            return true;
        }
    }

    // Not found.
    return false;
}
