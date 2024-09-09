#include  "containers/hashtable.h"
#include  "testmanager.h"

typedef struct Test{
  bool  Bvalue;
  f32   Fvalue;
  u64   Uvalue;

}Test;

#define CHECK_DESTROY(hashtablePtr)            \
EXPECTED_AS(0, (hashtablePtr)->memory);        \
EXPECTED_AS(0, (hashtablePtr)->elementCount);  \
EXPECTED_AS(0, (hashtablePtr)->elementSize);   \

u8 shouldCreateAndDestroy(){
  Hashtable hashtable;
  u64 elementSize = sizeof(u64);
  u64 elementCount= 6;
  u64 memory[6] = {0};

  hashtableCreate(elementSize, elementCount, memory,false, &hashtable);
  NOT_EXPECTED_AS(0, hashtable.memory);
  EXPECTED_AS(sizeof(u64),hashtable.elementSize);
  EXPECTED_AS(6, hashtable.elementCount);

  hashtableDestroy(&hashtable);

  CHECK_DESTROY(&hashtable)
  return true;
}

u8 shouldSetAndGet(){
  Hashtable hashtable = {0};
  u64 elementSize = sizeof(u64);
  u64 elementCount= 8;
  u64 memory[8] = {0};

  hashtableCreate(elementSize, elementCount, memory, false, &hashtable);

  u64 testval1 = 23;
  hashtableSet(&hashtable, "test1", &testval1);
  u64 gettestval1 = 0;
  hashtableGet(&hashtable, "test1", &gettestval1);
  EXPECTED_AS(testval1, gettestval1);

  hashtableDestroy(&hashtable);
  CHECK_DESTROY(&hashtable)
  return true;
}

u8 shouldSetAndGetPtr(){
  Hashtable hashtable = {0};
  u64 elementSize = sizeof(Test*);
  u64 elementCount= 16;
  u64 memory[16]  = {0};

  hashtableCreate(elementSize, elementCount, memory, true, &hashtable );
  Test t;
  Test* setptr = &t;
  setptr->Bvalue  = true;
  setptr->Uvalue  = 63;
  setptr->Fvalue  = 3.1415f;

  hashtableSetPtr(&hashtable, "test", (void**)&setptr);
  Test* getptr = {0};
  hashtableGetPtr(&hashtable, "test", (void**)&getptr);

  EXPECTED_AS(setptr->Bvalue, getptr->Bvalue);
  EXPECTED_FLOAT_AS(setptr->Fvalue, getptr->Fvalue);
  EXPECTED_AS(setptr->Uvalue, getptr->Uvalue);

  hashtableDestroy(&hashtable);
  CHECK_DESTROY(&hashtable)
  return true;
} 

u8 shouldSetAndGetNonexistant(){
  Hashtable hashtable = {0};
  u64 elementSize = sizeof(u64);
  u64 elementCount= 16;
  u64 memory[16]  = {0};

  hashtableCreate(elementSize, elementCount, memory, false, &hashtable);

  u64 setval = 23;
  hashtableSet(&hashtable, "test1", &setval);
  u64 getval = 0;
  hashtableGet(&hashtable, "test2", &getval);

  EXPECTED_AS( 0, getval);

  hashtableDestroy(&hashtable);
  CHECK_DESTROY(&hashtable)
  return true;
}

u8 shouldSetAndGetNonexistantPtr(){
  Hashtable hashtable = {0};
  u64 elementSize = sizeof(Test*);
  u64 elementCount= 16;
  u64 memory[16]  = {0};

  hashtableCreate(elementSize, elementCount, memory, true, &hashtable );

  Test t;
  Test* setptr = &t;
  setptr->Bvalue  = true;
  setptr->Uvalue  = 63;
  setptr->Fvalue  = 3.1415f;
  hashtableSetPtr(&hashtable, "test", (void**)&setptr);
  Test* getptr = NULL;
  hashtableGetPtr(&hashtable, "nonexistant", (void**)&getptr);

  EXPECTED_AS( 0, getptr);

  hashtableDestroy(&hashtable);
  CHECK_DESTROY(&hashtable);
  return true;
}

u8 shouldSetAndUnsetPtr(){
  Hashtable hashtable = {0};
  u64 elementSize = sizeof(Test*);
  u64 elementCount= 16;
  u64 memory[16]  = {0};

  hashtableCreate(elementSize, elementCount, memory, true, &hashtable );

  Test t;
  Test* setptr = &t;
  setptr->Bvalue  = true;
  setptr->Uvalue  = 63;
  setptr->Fvalue  = 3.1415f;
  bool result = false;

  result = hashtableSetPtr(&hashtable, "test", (void**)&setptr);
  EXPECTED_AS(true, result);

  Test* getptr = NULL;
  hashtableGetPtr(&hashtable, "test", (void**)&getptr);

  EXPECTED_AS(setptr->Bvalue, getptr->Bvalue);
  EXPECTED_FLOAT_AS(setptr->Fvalue, getptr->Fvalue);
  EXPECTED_AS(setptr->Uvalue, getptr->Uvalue);
  // unset it
  result = hashtableSetPtr(&hashtable, "test", 0);
  IS_TRUE(result);
  // should no longer be found
  Test*  getunsetptr = NULL;
  result = hashtableGetPtr(&hashtable, "test", (void**)&getunsetptr);
  IS_FALSE(result);

  hashtableDestroy(&hashtable);
  CHECK_DESTROY(&hashtable);
  return true;
}

u8 tryCallNonPtrOnPtrTable(){
  Hashtable hashtable = {0};
  u64 elementSize = sizeof(Test*);
  u64 elementCount= 16;
  u64 memory[16]  = {0};

  hashtableCreate(elementSize, elementCount, memory, true, &hashtable );
  Test t;
  Test* setptr = &t;
  setptr->Bvalue  = true;
  setptr->Uvalue  = 63;
  setptr->Fvalue  = 3.1415f;

  bool result;
  result = hashtableSet(&hashtable, "test", (void**)&setptr);
  IS_FALSE(result);

  Test* getptr = {0};
  result = hashtableGet(&hashtable, "test", (void**)&getptr);
  IS_FALSE(result);

  hashtableDestroy(&hashtable);
  CHECK_DESTROY(&hashtable)
  return true;

}

u8 tryCallPtrOnNonPtrTable(){
  Hashtable hashtable = {0};
  u64 elementSize = sizeof(Test*);
  u64 elementCount= 16;
  u64 memory[16]  = {0};

  hashtableCreate(elementSize, elementCount, memory, false, &hashtable );
  Test t;
  Test* setptr = &t;
  setptr->Bvalue  = true;
  setptr->Uvalue  = 63;
  setptr->Fvalue  = 3.1415f;

  bool result;
  result = hashtableSetPtr(&hashtable, "test", (void**)&setptr);
  IS_FALSE(result);

  Test* getptr = {0};
  result = hashtableGetPtr(&hashtable, "test", (void**)&getptr);
  IS_FALSE(result);

  hashtableDestroy(&hashtable);
  CHECK_DESTROY(&hashtable)

  return true;
}

u8 SetGetUpdatePtrType(){
  Hashtable hashtable = {0};
  u64 elementSize = sizeof(Test*);
  u64 elementCount= 16;
  u64 memory[16]  = {0};

  hashtableCreate(elementSize, elementCount, memory, true, &hashtable );

  Test original;
  Test* setoriginalptr    = &original;
  setoriginalptr->Bvalue  = true;
  setoriginalptr->Uvalue  = 63;
  setoriginalptr->Fvalue  = 3.1415f;

  Test update;
  Test* setupdateptr    = &update;
  setupdateptr->Bvalue  = false;
  setupdateptr->Uvalue  = 99;
  setupdateptr->Fvalue  = 6.2890f;

  Test* getptr = NULL;

  hashtableSetPtr(&hashtable, "test", (void**)&setoriginalptr);
  hashtableGetPtr(&hashtable, "test", (void**)&getptr);

  EXPECTED_AS(setoriginalptr->Bvalue, getptr->Bvalue);
  EXPECTED_FLOAT_AS(setoriginalptr->Fvalue, getptr->Fvalue);
  EXPECTED_AS(setoriginalptr->Uvalue, getptr->Uvalue);


  hashtableSetPtr(&hashtable, "test", (void**)&setupdateptr);
  hashtableGetPtr(&hashtable, "test", (void**)&getptr);

  EXPECTED_AS(setupdateptr->Bvalue, getptr->Bvalue);
  EXPECTED_FLOAT_AS(setupdateptr->Fvalue, getptr->Fvalue);
  EXPECTED_AS(setupdateptr->Uvalue, getptr->Uvalue);

  hashtableDestroy(&hashtable);
  CHECK_DESTROY(&hashtable)

  return true;
}
int main(void){
  testManagerInit();   

  testManagerRegisterTest(shouldCreateAndDestroy, "create and destroy");
  testManagerRegisterTest(shouldSetAndGet, "set and get");
  testManagerRegisterTest(shouldSetAndGetPtr, "set and get ptr values");
  testManagerRegisterTest(shouldSetAndGetNonexistant, "get nonexistant value");
  testManagerRegisterTest(shouldSetAndGetNonexistantPtr, "get nonexistant ptr value");
  testManagerRegisterTest(shouldSetAndUnsetPtr, "set and unset value");
  testManagerRegisterTest(tryCallNonPtrOnPtrTable, "NonPtr funcs in Ptr Types table");
  testManagerRegisterTest(tryCallPtrOnNonPtrTable, "Ptr funcs in NonPtr Types table");
  testManagerRegisterTest(SetGetUpdatePtrType, "set, get and update a ptr value");
  testManagerRunTests();
  return 0;
}


