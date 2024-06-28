#include  "events.h"
#include  "containers/darray.h"
#include  "defines.h"
#include  "logger.h"

typedef struct RegisteredEvent  RegisteredEvent;
typedef struct EventCodeEntry   EventCodeEntry;
typedef struct EventSystemState EventSystemState;

struct RegisteredEvent{
  void* listener;
  pfnOnEvent  callback;
};

struct EventCodeEntry{
  RegisteredEvent* events;
};

#define MAX_MESSAGE_CODES 128

struct EventSystemState{
  //lookup table for event codes
  EventCodeEntry registered[MAX_MESSAGE_CODES];
};

static EventSystemState* eventSystemStatePtr;

bool  initializeEvents(u64* memoryRequirement, void* state ){
TRACEEVENT;
EDEBUG("memoryRequirement :%"PRIu64" state : %p",*memoryRequirement, state);

  *memoryRequirement = sizeof(EventSystemState);
  if(state == NULL){
    EDEBUG("EVENT SUBSYSTEM : STATE PASSED AS NULL");
    return true;
  } 
  eventSystemStatePtr = state;

  KINFO("EVENT SUBSYSTEM INITIALIZED !");
  return true;
}

void  shutdownEvents(){
  KINFO("EVENT SUBSYSTEM SHUTDOWN !");
  for(u16 i = 0 ; i < MAX_MESSAGE_CODES ; ++i){
    if(eventSystemStatePtr->registered[i].events != 0){
      DARRAY_DESTROY(eventSystemStatePtr->registered[i].events);
      eventSystemStatePtr->registered[i].events  = 0;
    }
  }
  eventSystemStatePtr = NULL;
}

u8 eventRegister(u16 code, void* listener, pfnOnEvent onEvent) {
  TRACEMEMORY;
  EDEBUG("code : %"PRIu16" listener : %p onEvent : %p",code,listener,onEvent);
  if(eventSystemStatePtr  ==  NULL){
    return false;
  }
  if(eventSystemStatePtr->registered[code].events == 0) {
    eventSystemStatePtr->registered[code].events = DARRAY_CREATE(RegisteredEvent);
  }

  u64 registeredCount = DARRAY_LENGTH(eventSystemStatePtr->registered[code].events);
  for(u64 i = 0; i < registeredCount; ++i) {
    if(eventSystemStatePtr->registered[code].events[i].listener == listener) {
      UREPORT("TODO");
      return false;
    }
  }

  // If at this point, no duplicate was found. Proceed with registration.
  RegisteredEvent event;
  event.listener = listener;
  event.callback = onEvent;
  DARRAY_PUSH(eventSystemStatePtr->registered[code].events, event);

  return true;
}

u8 eventUnregister(u16 code, void* listener, pfnOnEvent onEvent) {
  TRACEEVENT;
  EDEBUG("code : %"PRIu16" listener : %p onEvent : %p",code,listener,onEvent);
  if(eventSystemStatePtr  ==  NULL){
    return false;
  }
  // On nothing is registered for the code, boot out.
  if(eventSystemStatePtr->registered[code].events == 0) {
    // TODO: warn
    return false;
  }

  u64 registeredCount = DARRAY_LENGTH(eventSystemStatePtr->registered[code].events);
  for(u64 i = 0; i < registeredCount; ++i) {
    RegisteredEvent e = eventSystemStatePtr->registered[code].events[i];
    if(e.listener == listener && e.callback == onEvent) {
      // Found one, remove it
      RegisteredEvent popped_event;
      DARRAY_POPAT(eventSystemStatePtr->registered[code].events, i, &popped_event);
      return true;
    }
  }

  // Not found.
  return false;
}

u8 eventFire(u16 code, void* sender, EventContext context) {
  TRACEEVENT;
  EDEBUG("code : %"PRIu16" sender : %p context : %p",code,sender,context);
  if(eventSystemStatePtr  ==  NULL){
    return false;
  }
  // If nothing is registered for the code, boot out.
  if(eventSystemStatePtr->registered[code].events == 0) {
    return false;
  }
  u64 registeredCount = DARRAY_LENGTH(eventSystemStatePtr->registered[code].events);
  for(u64 i = 0; i < registeredCount; ++i) {
    RegisteredEvent e = eventSystemStatePtr->registered[code].events[i];
    if(e.callback(code, sender, e.listener, context)) {
      // Message has been handled, do not send to other listeners.
      return true;
    }
  }
  // Not found.
  return false;
}
