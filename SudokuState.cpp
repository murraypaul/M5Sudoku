#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include <iterator>

#include <Preferences.h>

#include "Utility.h"

#include "SudokuState.h"

extern Preferences preferences;
extern const char* Preferences_App;

extern std::vector<uint8_t> vector_rand81;
extern std::mt19937 g_;

bool SudokuState::hasSave = false;

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
           //vTaskDelay(1);
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

bool SudokuState::SolveByGuessing( uint8_t depth )
{
    if( Solved() )
        return true;
    // Avoid stack overflow issues
    if( depth <= 0 )
        return false;
    bool bAnyChangeMade = false;
    auto point = FindLowestCountUnsolvedSquare();
    if( point.x == -1 )
        return false;
    auto oldState = Squares;
    SudokuSquare oldSquare = Squares[point.x][point.y];
//    log_d("Attempting to fix (%d,%d)[%s]",point.x,point.y,oldSquare.AsPossibleString().c_str());
    for( uint8_t val = 1 ; val <= 9 ; val++ )
    {
        //vTaskDelay(1);
        if( Squares[point.x][point.y].Possible(val) )
        {
//            log_d("Trying %d",val);
            Squares[point.x][point.y].SetSolution(val);
            Propagate();
            if( Solved() )
                return true;
            if( !Valid() )
            {
                Squares = oldState;
                continue;
            }
            if( SolveByGuessing(depth-1) )
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
//                    log_d("Invalid: row: (%d,%d)[%d] vs (%d,%d)[%d]", x, y, Squares[x][y].FirstPossible(), x2, y, Squares[x2][y].FirstPossible());
                    return false;
                }
            for( uint8_t y2 = 0 ; y2 < 9 ; y2++ )
                if( y != y2 && Squares[x][y].Fixed() && Squares[x][y2].Fixed() && Squares[x][y].FirstPossible() == Squares[x][y2].FirstPossible() )
                {
//                    log_d("Invalid: column: (%d,%d)[%d] vs (%d,%d)[%d]", x, y, Squares[x][y].FirstPossible(), x, y2, Squares[x][y2].FirstPossible());
                    return false;
                }
            uint8_t blockx = x/3;
            uint8_t blocky = y/3;
            for( uint8_t x2 = blockx * 3 ; x2 < (blockx+1) * 3 ; x2++ )
                for( uint8_t y2 = blocky * 3 ; y2 < (blocky+1) * 3 ; y2++ )
                    if( x != x2 && y != y2 && Squares[x][y].Fixed() && Squares[x2][y2].Fixed() && Squares[x][y].FirstPossible() == Squares[x2][y2].FirstPossible() )
                    {
//                        log_d("Invalid: block: (%d,%d)[%d] vs (%d,%d)[%d]", x, y, Squares[x][y].FirstPossible(), x2, y2, Squares[x2][y2].FirstPossible());
                        return false;
                    }
        }
    return true;
}

bool SudokuState::Solved() const
{
    if( !Valid() )
        return false;
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
//        log_d("Invalid");
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
            //vTaskDelay(1);
            uint32_t squareCount = Squares[x][y].Count();
            if( squareCount > 1 && squareCount < count )
            {
                count = squareCount;
                point = {x,y};
                if( count == 2 )
                    return point;
            }
        }

    return point;
}

uint8_t SudokuState::CountFixed() const
{
    uint8_t count = 0;
    for( uint8_t y = 0 ; y < 9 ; y++ ) 
        for( uint8_t x = 0 ; x < 9 ; x++ ) 
            if( Squares[x][y].Fixed() )
                count++;
    return count;
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

void SudokuState::FixOneSquare()
{
    if( !Valid() )
        return;
    if( Solved() )
        return;
    SudokuState temp = *this;
    temp.Propagate();
    temp.SolveByGuessing();
    if( !temp.Solved() )
        return;
    if( !temp.Valid() )
        return;
    while( true )
    {
        //vTaskDelay(1);
        uint8_t square = random(0,81);
        uint8_t x = square/9;
        uint8_t y = square%9;
        if( Squares[x][y].Fixed() )
            continue;
        Squares[x][y] = temp.Squares[x][y];
        break;
    }
}

#if 0
void SudokuState::GenerateRandom()
{
    auto ts = millis();
    uint32_t reduceMaxTime = 60*1000;
    SudokuState current;
    while( true )
    {
        SudokuState temp;
        while( !temp.Solved() )
        {
            uint16_t loop = 0;
            while( true && loop++ < 100 )
            {
                uint8_t square = random(0,81);
                uint8_t x = square/9;
                uint8_t y = square%9;
                if( temp.Squares[x][y].Fixed() )
                    continue;
                uint8_t val = random(1,10);
                while( !temp.Squares[x][y].Possible(val) )
                    val = 1 + val%9;
                temp.Squares[x][y].SetSolution(val);
                temp.Propagate();
                break;
            }
            if( !temp.Valid() )
                break;
        }
        if( temp.Valid() )
        {
            temp.SolveByGuessing();
            if( temp.Solved() )
            {
                current = temp;
                break;
            }
        }
    }
    log_d("Solved state (%c,%c)",current.Valid()?'Y':'N',current.Solved()?'Y':'N');
    current.Dump();

    uint8_t targetFixed = 22;
    uint16_t outerloop = 0;
    SudokuState solved = current;
    SudokuState lastResult = solved;
    ts = millis();
    while( true && outerloop++ < 100 && millis() - ts < reduceMaxTime )
    {
        current = solved;
        log_d("Current has %d fixed squares", current.CountFixed());
        uint16_t loop = 0;
        while( current.CountFixed() > targetFixed && loop++ < 100 )
        {
            uint8_t square = random(0,81);
            uint8_t x = square/9;
            uint8_t y = square%9;
            if( !current.Squares[x][y].Fixed() )
                continue;
//            SudokuState check = current;
//            check.Squares[x][y] = SudokuSquare();       
//            check.Propagate();
//            check.SolveByGuessing();
//            if( check.Solved() )
//            {
                current.Squares[x][y] = SudokuSquare();
//                log_d("Cleared (%d,%d), still solveable, count fixed %d",x,y,current.CountFixed());
//            }
        }
        if( current.CountFixed() <= targetFixed )
        {
            log_d("Hit fixed square target");
            SudokuState check = current;
            check.Propagate();
            check.SolveByGuessing();
            lastResult = current;
            if( check.Solved() )
            {
                log_d("Still solvable");
                break;
            }
            else
                log_d("Retry");
        }
    }
    current = lastResult;
    log_d("Complete (%c,%d fixed squares)", current.Valid()?'Y':'N', current.CountFixed());
    if( current.Valid() )
        (*this) = current;
    else
        (*this) = solved;
}
#else
void SudokuState::GenerateRandom( uint8_t targetFixedCells, uint32_t targetSolveTimeMS )
{
    auto ts = millis();
    log_d("Starting GenerateRandom at %d", ts);
    
    SudokuState current;
    while( true )
    {
        std::shuffle(vector_rand81.begin(), vector_rand81.end(), g_);
        SudokuState temp;
        for( uint8_t square : vector_rand81 )
        {
            //vTaskDelay(1);
            if( temp.Solved() )
                break;

            uint8_t x = square/9;
            uint8_t y = square%9;
            if( temp.Squares[x][y].Fixed() )
                continue;
            uint8_t val = random(1,10);
            while( !temp.Squares[x][y].Possible(val) )
                val = 1 + val%9;
            temp.Squares[x][y].SetSolution(val);
            temp.Propagate();
        }
        if( temp.Valid() )
        {
            uint8_t result = temp.SolveUniquely();
            log_d("Result: %d", result);
            if( result == 1 )
            {
                current = temp;
                break;
            }
        }
    }
    log_d("Solved state (%c,%c) in %d",current.Valid()?'Y':'N',current.Solved()?'Y':'N',millis()-ts);
    current.Dump();

    uint16_t outerloop = 0;
    SudokuState solved = current;
    SudokuState lastResult = solved;
    uint8_t bestCount = 82;
    while( true && outerloop++ < 100 && millis() - ts < targetSolveTimeMS )
    {
        std::shuffle(vector_rand81.begin(), vector_rand81.end(), g_);
        current = solved;
        log_d("Current has %d fixed squares", current.CountFixed());
        for( uint8_t square : vector_rand81 )
        {
            //vTaskDelay(1);
            if( current.CountFixed() <= targetFixedCells )
                break;

            uint8_t x = square/9;
            uint8_t y = square%9;
            if( !current.Squares[x][y].Fixed() )
                continue;
            SudokuState check = current;
            check.Squares[x][y] = SudokuSquare();       
            check.Propagate();
            uint8_t result = check.SolveUniquely();
            if( result == 1 )
            {
                current.Squares[x][y] = SudokuSquare();
                log_d("Cleared (%d,%d), still solveable, count fixed %d",x,y,current.CountFixed());
            } 
            else
                log_d("Removal failed, result = %d, count fixed %d", result, current.CountFixed());
        }
        uint8_t thisCount = current.CountFixed();
        if( thisCount < bestCount )
        {
            log_d("New best %d",thisCount);
            lastResult = current;
            bestCount = thisCount;
            if( bestCount <= targetFixedCells )
                break;
        }
    }
    current = lastResult;
    log_d("Complete (%c,%d fixed squares), total time %d", current.Valid()?'Y':'N', current.CountFixed(), millis()-ts);
    if( current.Valid() )
        (*this) = current;
    else
        (*this) = solved;

    // Verify solution
//    {
//        SudokuState temp = *this;
//        temp.Propagate();
//        auto result = temp.SolveUniquely();
//        log_d("Validation: %d",result);
//    }
}
#endif

uint8_t SudokuState::SolveUniquely( uint8_t depth, uint8_t x, uint8_t y, uint8_t count )
{
loop:
    //vTaskDelay(1);
    if( x >= 9 )
    {
        x = 0;
        y++;
    }
    if( y >= 9 )
        return count+1;
    
    SudokuSquare& thisSquare = Squares[x][y];
    SudokuSquare oldSquare = thisSquare;
    if( thisSquare.Fixed() )
    {
        x++;
        goto loop;
    }
    for( uint8_t val = 1 ; val <= 9 && count < 2 ; val++ )
    {
        //vTaskDelay(1);
        thisSquare = oldSquare;
        if( CheckPossible(x,y,val) )
        {
            thisSquare.SetSolution(val);
            count = SolveUniquely(depth-1,x+1,y,count);
        }

    }
    thisSquare = oldSquare;
    return count;
}

bool SudokuState::CheckPossible(uint8_t x, uint8_t y, uint8_t val) const
{
    for( uint8_t x2 = 0 ; x2 < 9 ; x2++ )
    {
        if( x2 == x )
            continue;
        if( Squares[x2][y].Fixed() && Squares[x2][y].FirstPossible() == val )
            return false;
    }
    for( uint8_t y2 = 0 ; y2 < 9 ; y2++ )
    {
        if( y2 == y )
            continue;
        if( Squares[x][y2].Fixed() && Squares[x][y2].FirstPossible() == val )
            return false;
    }
    uint8_t blockx = x/3;
    uint8_t blocky = y/3;
    for( uint8_t x2 = blockx * 3 ; x2 < (blockx+1) * 3 ; x2++ )
        for( uint8_t y2 = blocky * 3 ; y2 < (blocky+1) * 3 ; y2++ )
        {
            if( x2 == x && y2 == y )
                continue;
            if( Squares[x2][y2].Fixed() && Squares[x2][y2].FirstPossible() == val )
                return false;
        }      
    return true;
}

void SudokuState::Load()
{
    preferences.begin(Preferences_App);
    if( preferences.getBool("Saved",false) )
        for( uint8_t y = 0 ; y < 9 ; y++ )
            for( uint8_t x = 0 ; x < 9 ; x+= 3 )
            {
                std::bitset<27> threeSquares(preferences.getULong(String(y*9+x).c_str(),0));
                for( uint8_t j = 0 ; j < 3 ; j++ )
                {
                    SudokuSquare& thisSquare = Squares[x+j][y];
                    thisSquare = SudokuSquare();
                    for( uint8_t i = 0 ; i < 9 ; i++ )
                        if( !threeSquares[j*9+i] )
                            thisSquare.RemovePossible(i+1);
                }
            }
    preferences.end(); 
}

void SudokuState::Save()
{
    std::bitset<27> threeSquares;
    preferences.begin(Preferences_App);
    for( uint8_t y = 0 ; y < 9 ; y++ )
        for( uint8_t x = 0 ; x < 9 ; x+= 3 )
        {
            for( uint8_t j = 0 ; j < 3 ; j++ )
            {
                const SudokuSquare& thisSquare = Squares[x+j][y];
                for( uint8_t i = 0 ; i < 9 ; i++ )
                    threeSquares.set(j*9+i,thisSquare.Possible(i+1));
            }
            preferences.putULong(String(y*9+x).c_str(),threeSquares.to_ulong());
        }
    preferences.putBool("Saved",true);
    preferences.end(); 
    hasSave = true;
}

void SudokuState::RemoveSave()
{
    preferences.begin(Preferences_App);
    if( preferences.getBool("Saved",false) )
    {
        for( uint8_t y = 0 ; y < 9 ; y++ )
            for( uint8_t x = 0 ; x < 9 ; x+= 3 )
                preferences.remove(String(y*9+x).c_str());
        preferences.remove("Saved");
    }
    preferences.end(); 
    hasSave = false;
}

bool SudokuState::CheckHasSave()
{
    preferences.begin(Preferences_App);
    hasSave = preferences.getBool("Saved",false);   
    preferences.end(); 
    return hasSave;
}