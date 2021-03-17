# M5Sudoku
Sudoku application for M5Paper

Can generate uniquely solveable Sudoku puzzles, targetting a given number of clues, to act as a difficulty setting.

At any point, can validate that the puzzle is still solvable.

Can mark squares as either a known value, or a set of possible values.

Can save and reload current state to EEPROM.

Will automatically shutdown and save state after 5 minutes of inactivity.

Notes:
- Selecting a square in the large grid will display it in the small grid on the right 
- The selected square is highlighted in the large grid
- You can use the small grid to either set a single known value for the square, or select multiple possible values
- It may not be possible to generate a uniquely solveable puzzle of the given numbers of clues in the time requested
- In this situation, the puzzle with the lowest number of clues that still gives a unqiue solution will be returned
- The fewer target clues you ask for, the longer it will take to generate the puzzle
- The 'Validate' button will confirm that the puzzle is still uniquely solveable
- The 'Clue' button will fill in one randon unsolved square
- Over time the screen may get a bit muddy, due to the fast refresh option used on the EPD screen
- The 'Validate' button will also do a full screen slow refresh, which will clean up the display
