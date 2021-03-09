#pragma once

#include <bitset>

class SudokuSquare
{
public:
    SudokuSquare() : BitSet(511) {};    // Set first 9 bits

protected:
    std::bitset<9>  BitSet;             // Possible values for square

public:
    uint8_t         Count() const { return BitSet.count(); };
    bool            Valid() const { return Count() > 0; };
    bool            Fixed() const { return Count() == 1; };

    bool            Possible( uint8_t i ) const { if( i > 0 && i <= 9 ) return BitSet[i-1]; else return false; };
    void            RemovePossible( uint8_t i ) { if( i > 0 && i <= 9 ) BitSet.reset(i-1); };
    void            AddPossible( uint8_t i ) { if( i > 0 && i <= 9 ) BitSet.set(i-1); };
    void            SetSolution( uint8_t i ) { BitSet.reset(); if( i > 0 && i <= 9 ) BitSet.set(i-1); };
    
    uint8_t         FirstPossible() const { for( uint8_t i = 0 ; i < 9 ; i++ ) if( BitSet[i] ) return i+1; return 255; };

    SudokuSquare    IdentifyUniques( const SudokuSquare& other ) const
    {
        SudokuSquare ret = (*this);
        for( uint8_t i = 1 ; i <= 9 ; i++ )
            if( other.Possible(i) )
                ret.RemovePossible(i);
        return ret;
    }

    String          AsPossibleString() const 
    {
        String ret = "";
        for( uint8_t i = 1 ; i <= 9 ; i++ )
            if( Possible(i) )
                ret += String(i);
            else
                ret += "_";
        return ret;
    }
};
