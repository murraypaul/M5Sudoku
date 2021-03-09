#include "M5EPD.h"

#include "DisplayManager.h"

#include "SudokuState.h"

M5EPD_Canvas Canvas(&M5.EPD);

SudokuState CurrentState;
Point<uint8_t>  CurrentSquare;
String      LastValidation = "";

void setup() 
{
  BaseDisplayManager.Init(true);

//CurrentState.GenerateFromString("53  7    6  195    98    6 8   6   34  8 3  17   2   6 6    28    419  5    8  79"); // Propagate only
//CurrentState.GenerateFromString("4       7  2 8 53    75   9  587  626 392  8  9  65     7        6  72      917 3"); // Propagate only
//CurrentState.GenerateFromString("    68 3 19       8 31  2  4   51 6 7   2   4    7 8   1   5  7  4       5  3 1  "); // Propagate and guess
  CurrentState.GenerateFromString("    7  9  46 3     2 5        1  2  3 7  4   81    5  5   63       1  37        6"); // Propagate and guess
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

  BaseDisplayManager.draw();
}

void loop() {
  BaseDisplayManager.doLoop();
}
