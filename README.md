# M5Sudoku
Sudoku application for M5Paper

Can generate uniquely solveable Sudoku puzzles, targetting a given number of clues, to act as a difficulty setting.

At any point, can validate that the puzzle is still solvable.

Can mark squares as either a known value, or a set of possible values.

Can save and reload current state to EEPROM.

Will automatically shutdown and save state after 5 minutes of inactivity.

Notes:
- It may not be possible to generate a uniquely solveable puzzle of the given numbers of clues in the time requested
- In this situation, the puzzle with the lowest number of clues that still gives a unqiue solution will be returned
- The fewer target clues you ask for, the longer it will take to generate the puzzle
- The 'Validate' button will confirm that the puzzle is still uniquely solveable
- The 'Clue' button will fill in one randon unsolved square
