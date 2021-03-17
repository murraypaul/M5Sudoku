#include "M5EPD.h"

#include "DisplayManager.h"

#include "SudokuState.h"

M5EPD_Canvas Canvas(&M5.EPD);

SudokuState CurrentState;
Point<uint8_t>  CurrentSquare;
String      LastValidation = "";

void myloop(void*);

void setup() 
{
  BaseDisplayManager.Init(true);

//CurrentState.GenerateFromString("53  7    6  195    98    6 8   6   34  8 3  17   2   6 6    28    419  5    8  79"); // Propagate only
//CurrentState.GenerateFromString("4       7  2 8 53    75   9  587  626 392  8  9  65     7        6  72      917 3"); // Propagate only
//CurrentState.GenerateFromString("    68 3 19       8 31  2  4   51 6 7   2   4    7 8   1   5  7  4       5  3 1  "); // Propagate and guess
//CurrentState.GenerateFromString("    7  9  46 3     2 5        1  2  3 7  4   81    5  5   63       1  37        6"); // Propagate and guess
/*  CurrentState.Dump();
  log_d("Is Valid: %c", CurrentState.Valid()?'Y':'N');
  log_d("Is Solved: %c", CurrentState.Solved()?'Y':'N');
  CurrentState.Propagate();
  CurrentState.Dump();
  log_d("Is Valid: %c", CurrentState.Valid()?'Y':'N');
  log_d("Is Solved: %c", CurrentState.Solved()?'Y':'N');
  if( !CurrentState.Solved() )
  {
    CurrentState.SolveByGuessing();
    CurrentState.Dump();
    log_d("Is Valid: %c", CurrentState.Valid()?'Y':'N');
    log_d("Is Solved: %c", CurrentState.Solved()?'Y':'N');
  }*/

  SudokuState::CheckHasSave();
  log_d("HasSave: %c", SudokuState::HasSave()?'Y':'N');
  if( SudokuState::HasSave() )
    CurrentState.Load();

  BaseDisplayManager.draw();

//  disableCore0WDT();
//  disableCore1WDT();

//  xTaskCreate(
//                    myloop,          /* Task function. */
//                    "MyLook",        /* String with name of task. */
//                    32*1024,            /* Stack size in bytes. */
//                    NULL,             /* Parameter passed as input of the task */
//                    3,                /* Priority of the task. */
//                    NULL);            /* Task handle. */ 
}

void loop() {
//  static uint32_t lastCheck = millis();
//    auto ts = millis();
//    if( ts - 1000 > lastCheck )
//    {
//      log_d("Ping");
//      lastCheck = ts;
//    }
//  vTaskDelay(0);
    BaseDisplayManager.doLoop();
//  yield();
}

void myloop(void*) {
//  static uint32_t lastCheck = millis();
  while(true)
  {
//    auto ts = millis();
//    if( ts - 1000 > lastCheck )
//    {
//      log_d("Pong");
//      lastCheck = ts;
//    }
    BaseDisplayManager.doLoop();
    yield();
  }
}
