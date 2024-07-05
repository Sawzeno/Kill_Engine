#include "errors.h"

#include  "core/events.h"
#include  "core/logger.h"
#include  <unistd.h>
#include  <signal.h>
#include  <execinfo.h>
#include  <dlfcn.h>


void handle_sigill(int sig, siginfo_t* info, void* context);
void handle_sigsegv(int sig);
void handle_sigint(int sig);

bool initErrors(){


  stack_t signalStack;
  signalStack.ss_size = SIGSTKSZ;
  signalStack.ss_sp   = malloc(signalStack.ss_size);
  if(signalStack.ss_sp == NULL){
    UFATAL("COULD NOT ALLOCATE MEMORY FOR SIGNAL STACK !!");
    return false;
  }
  signalStack.ss_flags= SS_ONSTACK;
  if(sigaltstack(&signalStack, NULL) == -1){
    UERROR("COULD NOT INITIALIZE SIGNALSTACK !");
    return false;
  }

  struct sigaction sigactINT;
  sigemptyset(&sigactINT.sa_mask);
  sigactINT.sa_handler= handle_sigint;
  sigactINT.sa_flags = SA_ONSTACK;

  struct sigaction sigactSEGV;
  sigemptyset(&sigactSEGV.sa_mask);
  sigactSEGV.sa_handler= handle_sigsegv;
  sigactSEGV.sa_flags = SA_ONSTACK;

  struct sigaction sigactILL;
  sigemptyset(&sigactILL.sa_mask);
  sigactILL.sa_sigaction = handle_sigill;
  sigactILL.sa_flags  = SA_SIGINFO ;

  if( 
    sigaction(SIGINT, &sigactINT, NULL) == -1 ||
    sigaction(SIGSEGV,&sigactSEGV,NULL) == -1 ||
    sigaction(SIGILL, &sigactILL, NULL) == -1
  ){
    UERROR("COULD NOT SET UP SIGNAL HANDLERS");
    exit(EXIT_FAILURE);
  }
 
  return true;
}

void sendSignal( int sig, const char* str, bool e){
  union sigval sv;

  Uinfo* info = calloc(1, sizeof(Uinfo));
  if(info != NULL){
    info->str  = str;
    info->e    = e;
    sv.sival_ptr  = (void*)info;
  }else{
    UFATAL("could not even allocate memory for error info");
  }
  if (sigqueue(getpid(), sig, sv) == -1) {
    UFATAL("could not call sigqueue");
    exit(EXIT_FAILURE);
  }
}

void handle_sigill(int sig, siginfo_t* info, void* context){
  (void)context;
  if(sig  ==  SIGILL){
    if(info == NULL){
      UFATAL("SIGILL raised");
      shutdownLogging();
      exit(SIGILL);
    }else{
      Uinfo* uinfo  = (Uinfo*)(info->si_value.sival_ptr);

      UFATAL("SIGILL raised : %s", uinfo->str);
      if(uinfo->e){
        UFATAL("EXITING!");
        EventContext  data  = {};
        eventFire(EVENT_CODE_APPLICATION_QUIT , 0 , data);
      }else{
        return;
      }
    }
  }
}

void handle_sigint(int sig){
  if(sig == SIGINT){
  UFATAL("SIGINT RESCIEVED SHUTTING DOWN APPLICATION");
  EventContext  data  = {};
  eventFire(EVENT_CODE_APPLICATION_QUIT , 0 , data);
  UFATAL("EXITED SUCCESFULLY");
  }
}

void handle_sigsegv(int sig) {
  if (sig == SIGSEGV) {
    void* buffer[100];
    size_t size = backtrace(buffer, 100);
    char** symbols = backtrace_symbols(buffer, size);

    if (symbols != NULL) {
      for (size_t i = 0; i < size; i++) {
        UERROR("Backtrace: %s", symbols[i]);
      }
      free(symbols);
    } else {
      UERROR("Could not backtrace symbols");
    }
    UFATAL("SIGSEGV: TERMINATING...");
    shutdownLogging();  
    exit(EXIT_SUCCESS);
  }
}

