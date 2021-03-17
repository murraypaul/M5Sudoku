#pragma once

#include <Arduino.h>
#include <array>

#include "SudokuSquare.h"

class SudokuState
{
public:
    SudokuState() = default;

protected:
    using tdSquares = std::array<std::array<SudokuSquare,9>,9>;
    tdSquares       Squares;
    constexpr static uint8_t maxDepth = 64;
    static bool     hasSave;

public:
    void            GenerateEmpty() { for( uint8_t x = 0 ; x < 9 ; x++ ) for( uint8_t y = 0 ; y < 9 ; y++ ) Squares[x][y] = SudokuSquare(); };
    void            GenerateFromString( String str );
    void            GenerateRandom( uint8_t targetFixedCells, uint32_t targetSolveTimeMS );

    bool            Propagate();        // returns true if any changes were made
    bool            PropagateOnce( uint16_t iLoop = 0 ); // returns true if a change was made
    bool            SolveByIdentifingUniques();  // returns true if solved
    bool            SolveByGuessing( uint8_t depth = maxDepth );  // returns true if solved
    uint8_t         SolveUniquely( uint8_t depth = maxDepth, uint8_t x = 0, uint8_t y = 0, uint8_t count = 0 );  // returns 0 if unsolved, 1 if unique solution found or 2 if more than one solution found

    bool            Valid() const;
    bool            Solved() const;
    uint16_t        SumCount() const;
    uint8_t         CountFixed() const;
    bool            CheckPossible(uint8_t x, uint8_t y, uint8_t val) const;

    Point<signed char>  FindLowestCountUnsolvedSquare() const;
    void            FixOneSquare();

    SudokuSquare&   GetSquare( Point<uint8_t> pt ) { return GetSquare(pt.x,pt.y); };
    SudokuSquare&   GetSquare( uint8_t x, uint8_t y ) { return Squares[x][y]; };

    void            Load();
    void            Save();
    static void     RemoveSave();
    static bool     HasSave() { return hasSave; };
    static bool     CheckHasSave();

    void            Dump() const;

};
