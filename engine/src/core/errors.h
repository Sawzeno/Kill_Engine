#pragma once

#include  <stdbool.h>
#include  <signal.h>

typedef struct Uinfo Uinfo;

struct  Uinfo{
  const char* str;
  bool        e;
};

bool  initErrors();
void  sendSignal        (int sig, const char* str, bool e);
