#include "Utility.h"

#include "SudokuState.h"

void SudokuState::GenerateFromString( String str )
{
    if( str.length() != 9*9 )
        return;
    for( uint8_t y = 0 ; y < 9 ; y++ ) 
        for( uint8_t x = 0 ; x < 9 ; x++ ) 
        {
            String sval = str.substring(y*9+x,y*9+x+1);
            if( sval == " ")
                Squares[x][y] = SudokuSquare();
            else
            {
                uint8_t val = sval.toInt();
                if( val <= 0 || val > 9 )
                    return;
                Squares[x][y].SetSolution(val);
            }
        }
};

bool SudokuState::Propagate()
{
    if( Solved() )
        return false;
    uint16_t iLoop = 0;
    bool bAnyChangeMade = false;
    while( PropagateOnce(iLoop++) )
        bAnyChangeMade = true;
    return bAnyChangeMade;
}

bool SudokuState::PropagateOnce( uint16_t iLoop )
{
    bool bChangeMade = false;
    uint32_t sumCount = SumCount();
//    log_d("Loop %d, sum count %d",iLoop,sumCount);
    if( !Valid() || sumCount == 0 )
    {
//        log_d("Invalid");
        return bChangeMade;
    }
    for( uint8_t x = 0 ; x < 9 ; x++ )
        for( uint8_t y = 0 ; y < 9 ; y++ )
        {
            if( !Squares[x][y].Fixed() )
                continue;
            uint8_t val = Squares[x][y].FirstPossible();
            for( uint8_t x2 = 0 ; x2 < 9 ; x2++ )
            {
                if( Squares[x2][y].Fixed() )
                    continue;
                if( Squares[x2][y].Possible(val) )
                {
//                        log_d("Removing %d from (%d,%d)",val,x2,y);
                    Squares[x2][y].RemovePossible(val);
                    bChangeMade = true;
                    if( !Valid() ) { /*log_d("Invalid!");*/ return bChangeMade; };
                }
            }
            for( uint8_t y2 = 0 ; y2 < 9 ; y2++ )
            {
                if( Squares[x][y2].Fixed() )
                    continue;
                if( Squares[x][y2].Possible(val) )
                {
//                        log_d("Removing %d from (%d,%d)",val,x,y2);
                    Squares[x][y2].RemovePossible(val);
                    bChangeMade = true;
                    if( !Valid() ) { /*log_d("Invalid!");*/ return bChangeMade; };
                }
            }
            uint8_t blockx = x/3;
            uint8_t blocky = y/3;
            for( uint8_t x2 = blockx * 3 ; x2 < (blockx+1) * 3 ; x2++ )
                for( uint8_t y2 = blocky * 3 ; y2 < (blocky+1) * 3 ; y2++ )
                {
                    if( Squares[x2][y2].Fixed() )
                        continue;
                    if( Squares[x2][y2].Possible(val) )
                    {
//                            log_d("Removing %d from (%d,%d)",val,x2,y2);
                        Squares[x2][y2].RemovePossible(val);
                        bChangeMade = true;
                        if( !Valid() ) { /*log_d("Invalid!");*/ return bChangeMade; };
                    }
                }
        }        

    return bChangeMade;
}

bool SudokuState::SolveByIdentifingUniques()
{
}

bool SudokuState::SolveByGuessing()
{
    if( Solved() )
        return false;
    bool bAnyChangeMade = false;
    auto point = FindLowestCountUnsolvedSquare();
    if( point.x == -1 )
        return false;
    auto oldState = Squares;
    SudokuSquare oldSquare = Squares[point.x][point.y];
    log_d("Attempting to fix (%d,%d)[%s]",point.x,point.y,oldSquare.AsPossibleString());
    for( uint8_t val = 1 ; val <= 9 ; val++ )
    {
        if( Squares[point.x][point.y].Possible(val) )
        {
            log_d("Trying %d",val);
            Squares[point.x][point.y].SetSolution(val);
            Propagate();
            if( Solved() )
                return true;
            if( !Valid() )
            {
                Squares = oldState;
                continue;
            }
            if( SolveByGuessing() )
                return true;

            Squares = oldState;
            continue;
        }
    }
    return false;
}

bool SudokuState::Valid() const
{
    for( uint8_t x = 0 ; x < 9 ; x++ )
        for( uint8_t y = 0 ; y < 9 ; y++ )
            if( !Squares[x][y].Valid() )
                return false;
    for( uint8_t x = 0 ; x < 9 ; x++ )
        for( uint8_t y = 0 ; y < 9 ; y++ )
        {
            for( uint8_t x2 = 0 ; x2 < 9 ; x2++ )
                if( x != x2 && Squares[x][y].Fixed() && Squares[x2][y].Fixed() && Squares[x][y].FirstPossible() == Squares[x2][y].FirstPossible() )
                {
                    log_d("Invalid: row: (%d,%d)[%d] vs (%d,%d)[%d]", x, y, Squares[x][y].FirstPossible(), x2, y, Squares[x2][y].FirstPossible());
                    return false;
                }
            for( uint8_t y2 = 0 ; y2 < 9 ; y2++ )
                if( y != y2 && Squares[x][y].Fixed() && Squares[x][y2].Fixed() && Squares[x][y].FirstPossible() == Squares[x][y2].FirstPossible() )
                {
                    log_d("Invalid: column: (%d,%d)[%d] vs (%d,%d)[%d]", x, y, Squares[x][y].FirstPossible(), x, y2, Squares[x][y2].FirstPossible());
                    return false;
                }
            uint8_t blockx = x/3;
            uint8_t blocky = y/3;
            for( uint8_t x2 = blockx * 3 ; x2 < (blockx+1) * 3 ; x2++ )
                for( uint8_t y2 = blocky * 3 ; y2 < (blocky+1) * 3 ; y2++ )
                    if( x != x2 && y != y2 && Squares[x][y].Fixed() && Squares[x2][y2].Fixed() && Squares[x][y].FirstPossible() == Squares[x2][y2].FirstPossible() )
                    {
                        log_d("Invalid: block: (%d,%d)[%d] vs (%d,%d)[%d]", x, y, Squares[x][y].FirstPossible(), x2, y2, Squares[x2][y2].FirstPossible());
                        return false;
                    }
        }
    return true;
}

bool SudokuState::Solved() const
{
    for( uint8_t x = 0 ; x < 9 ; x++ )
        for( uint8_t y = 0 ; y < 9 ; y++ )
            if( !Squares[x][y].Fixed() )
                return false;
    return true;
}

uint16_t SudokuState::SumCount() const
{
    if( !Valid() )
    {
        log_d("Invalid");
        return 0;
    }
    uint32_t count = 0;
    for( uint8_t x = 0 ; x < 9 ; x++ )
        for( uint8_t y = 0 ; y < 9 ; y++ )
        {
            uint32_t squareCount = Squares[x][y].Count();
            if( squareCount == 0 )
                log_d("(%d,%d) has count 0",x,y);
            count += squareCount;
//            if( count == 0 )
//                log_d("(%d,%d) gives total count %d",x,y,count);
        }
    return count;
}

Point<uint8_t> SudokuState::FindLowestCountUnsolvedSquare() const
{
    if( !Valid() )
        return {-1,-1};
    if( Solved() )
        return {-1,-1};

    Point<uint8_t> point;
    uint8_t count = 10;
    for( uint8_t x = 0 ; x < 9 ; x++ )
        for( uint8_t y = 0 ; y < 9 ; y++ )
        {
            uint32_t squareCount = Squares[x][y].Count();
            if( squareCount > 1 && squareCount < count )
            {
                count = squareCount;
                point = {x,y};
            }
        }

    return point;
}

void SudokuState::Dump() const
{
    for( uint8_t y = 0 ; y < 9 ; y++ ) 
    {
        String str;
        for( uint8_t x = 0 ; x < 9 ; x++ ) 
        {
            if( Squares[x][y].Fixed() )
            {
                str += String(Squares[x][y].FirstPossible());
            }
            else if( Squares[x][y].Valid() )
            {
                const char c = ('a' + Squares[x][y].Count() - 2);
                str += c;
            }
            else
                str += "!";
        }
        log_d("%s",str.c_str());
    }
};
