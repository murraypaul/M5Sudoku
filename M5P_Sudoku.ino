#include "M5EPD.h"

#include "SudokuState.h"

M5EPD_Canvas Canvas(&M5.EPD);

void setup() 
{
    M5.begin();
    M5.TP.SetRotation(90);
    M5.EPD.SetRotation(90);
    M5.EPD.Clear(true);

    Canvas.createCanvas(540, 960);

  SudokuState state;
//state.GenerateFromString("53  7    6  195    98    6 8   6   34  8 3  17   2   6 6    28    419  5    8  79"); // Propagate only
//state.GenerateFromString("4       7  2 8 53    75   9  587  626 392  8  9  65     7        6  72      917 3"); // Propagate only
//state.GenerateFromString("    68 3 19       8 31  2  4   51 6 7   2   4    7 8   1   5  7  4       5  3 1  "); // Propagate and guess
  state.GenerateFromString("    7  9  46 3     2 5        1  2  3 7  4   81    5  5   63       1  37        6"); // Propagate and guess
  state.Dump();
  log_d("Is Valid: %c", state.Valid()?'Y':'N');
  log_d("Is Solved: %c", state.Solved()?'Y':'N');
  state.Propagate();
  state.Dump();
  log_d("Is Valid: %c", state.Valid()?'Y':'N');
  log_d("Is Solved: %c", state.Solved()?'Y':'N');
  if( !state.Solved() )
  {
    state.SolveByGuessing();
    state.Dump();
    log_d("Is Valid: %c", state.Valid()?'Y':'N');
    log_d("Is Solved: %c", state.Solved()?'Y':'N');
  }
}

void loop() {
}
