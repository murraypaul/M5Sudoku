#include "LayoutItem.h"

#include <limits>

#include "DisplayManager.h"

#include "SudokuState.h"
#include "SudokuSquare.h"

extern SudokuState CurrentState;
extern Point<uint8_t> CurrentSquare;

extern String LastValidation;

bool drawIcon( M5EPD_Canvas& canvas, const unsigned char* bmpFS, size_t size, uint16_t x, uint16_t y );

LayoutItem::LayoutItem( Rect<uint16_t> rect, tdAction action )
: Location(rect)
, Action(action)
{
}

LayoutItemWithFont::LayoutItemWithFont( Rect<uint16_t> rect, const GFXfont* font, uint8_t align, tdAction action )
: LayoutItem(rect,action)
, Font(font)
, TextAlign(align)
{
}

bool LayoutItem::hitTest( const Point<uint16_t>& hit )
{
    if( Action.get() && Action->hasAction() )
    {
//        log_d("Possible hit, testing location (%d,%d) vs (%d,%d,%d,%d)"
//            , hit.x, hit.y
//            , Location.left,Location.top,Location.right,Location.bottom );
        if( Location.contains(hit) )
        {
            log_d("Hit");
            Action->doAction();
            return true;
        }
    } 

    return false;
}

void LayoutItemWithFont::drawString( DisplayManager& displayManager, String str )
{
    displayManager.fillRect(Location,0);
    displayManager.drawString( Font, TextAlign, str, Location);
}

LayoutItem_ButtonIcon::LayoutItem_ButtonIcon( Rect<uint16_t> rect, const unsigned char* data, size_t size, tdAction action )
: LayoutItem(rect,action)
, Data(data), DataSize(size)
{
}

void LayoutItem_ButtonIcon::draw( DisplayManager& displayManager )
{
//    if( HideIfNoActiveDevice && !SpotifyController::HasActiveDevice && SpotifyController::LastActiveDeviceID == "" )
//        displayManager.fillRect(Location,0);
//    else
        drawIcon(displayManager.GetCanvas(),Data,DataSize,Location.left,Location.top);
}

LayoutItem_ButtonIconWithHighlight::LayoutItem_ButtonIconWithHighlight( Rect<uint16_t> rect, const unsigned char* data, size_t size, tdHighlightFunc func, tdAction action )
: LayoutItem_ButtonIcon(rect,data,size,action)
, HighlightFunc(func)
{
}

void LayoutItem_ButtonIconWithHighlight::draw( DisplayManager& displayManager )
{
//    if( HideIfNoActiveDevice && !SpotifyController::HasActiveDevice && SpotifyController::LastActiveDeviceID == "" )
//        displayManager.fillRect(Location,0);
//    else
    {
        drawIcon(displayManager.GetCanvas(),Data,DataSize,Location.left,Location.top);
        if( HighlightFunc && HighlightFunc() )
            displayManager.drawRect(Location,15);
    }
}

LayoutItem_StaticText::LayoutItem_StaticText( Rect<uint16_t> rect, const GFXfont* font, uint8_t align, String text, tdAction action )
: LayoutItemWithFont(rect,font,align,action)
, Text(text)
{
}

void LayoutItem_Rectangle::draw( DisplayManager& displayManager )
{
    displayManager.drawRect(Location, 15);
}

void LayoutItem_StaticText::draw( DisplayManager& displayManager )
{
    drawString( displayManager, Text);
}

LayoutItem_DynamicText::LayoutItem_DynamicText( Rect<uint16_t> rect, const GFXfont* font, uint8_t align, tdStringFunc func, tdOutlineFunc outlineFunc, tdAction action )
: LayoutItemWithFont(rect,font,align,action)
, TextFunc(func)
, OutlineFunc(outlineFunc)
{

}

void LayoutItem_DynamicText::draw( DisplayManager& displayManager )
{
    displayManager.fillRect(Location, 0);
    if( TextFunc )
        drawString( displayManager, TextFunc() );
    if( OutlineFunc && OutlineFunc() )
        displayManager.drawRect(Location, 15);
}

LayoutItemAction_SudokuSquare::LayoutItemAction_SudokuSquare( uint8_t x, uint8_t y )
: WhichSquare(x,y)
{
}

void LayoutItemAction_SudokuSquare::doAction()
{
    CurrentSquare = WhichSquare;
    BaseDisplayManager.draw();
}

LayoutItem_SudokuSquare::LayoutItem_SudokuSquare( Rect<uint16_t> rect, uint8_t x, uint8_t y )
: LayoutItem(rect,std::make_shared<LayoutItemAction_SudokuSquare>(x,y))
, WhichSquare(x,y)
{
}

void LayoutItem_SudokuSquare::draw( DisplayManager& displayManager )
{
    SudokuSquare& mySquare = CurrentState.GetSquare(WhichSquare);
    if( WhichSquare == CurrentSquare )
    {
        displayManager.fillRect(Location,15); 
        displayManager.GetCanvas().setTextColor(0);
    }
    if( mySquare.Fixed() )
        displayManager.drawString(&FreeSans24pt7b,CC_DATUM,String(mySquare.FirstPossible()),Location);
    else if( mySquare.Count() == 9 )
        ;
    else
    {
        uint8_t width = Location.width()/3;
        uint8_t height = Location.height()/3;
        for( uint8_t x = 0 ; x < 3 ; x++ )
            for( uint8_t y = 0 ; y < 3 ; y++ )
                if( mySquare.Possible(y*3+x+1) )
                    displayManager.drawString(&FreeSans9pt7b,CC_DATUM,String(y*3+x+1)
                    ,Rect<uint16_t>{Location.left+x*width,Location.top+y*height,Location.left+(x+1)*width,Location.top+(y+1)*height});
    }
    if( WhichSquare == CurrentSquare )
        displayManager.GetCanvas().setTextColor(15);
}

LayoutItemAction_SudokuSubSquare::LayoutItemAction_SudokuSubSquare( uint8_t val )
: WhichValue(val)
{
}

void LayoutItemAction_SudokuSubSquare::doAction()
{
    if( CurrentState.GetSquare(CurrentSquare).Count() == 9 )
    {
        CurrentState.GetSquare(CurrentSquare).SetSolution(WhichValue);
    }
    else if( CurrentState.GetSquare(CurrentSquare).Possible(WhichValue) )
        CurrentState.GetSquare(CurrentSquare).RemovePossible(WhichValue);
    else
        CurrentState.GetSquare(CurrentSquare).AddPossible(WhichValue);
    LastValidation = "";
    BaseDisplayManager.draw();
}

LayoutItem_SudokuSubSquare::LayoutItem_SudokuSubSquare( Rect<uint16_t> rect, uint8_t val )
: LayoutItem(rect,std::make_shared<LayoutItemAction_SudokuSubSquare>(val))
, WhichValue(val)
{
}

void LayoutItem_SudokuSubSquare::draw( DisplayManager& displayManager )
{
    log_d("Drawing %d at (%d,%d,%d,%d)",WhichValue,Location.left,Location.top,Location.right,Location.bottom);
    SudokuSquare& mySquare = CurrentState.GetSquare(CurrentSquare);
    displayManager.fillRect(Location,0);
    if( CurrentState.GetSquare(CurrentSquare).Count() == 9 )
        ;
    else if( mySquare.Possible(WhichValue) )
        displayManager.drawString(&FreeSans24pt7b,CC_DATUM,String(WhichValue),Location);
    else
        ;
}

LayoutItem_SudokuMainBackground::LayoutItem_SudokuMainBackground( Rect<uint16_t> rect )
: LayoutItem(rect,nullptr)
{
}

void LayoutItem_SudokuMainBackground::draw( DisplayManager& displayManager )
{
    displayManager.fillRect(Location,0);
};

LayoutItem_SudokuGrid::LayoutItem_SudokuGrid( Rect<uint16_t> rect, bool bSub )
: LayoutItem(rect,nullptr)
, SubGrid(bSub)
{
}

void LayoutItem_SudokuGrid::draw( DisplayManager& displayManager )
{
    uint8_t numSquares = SubGrid ? 3 : 9;
    uint16_t width = Location.width()/numSquares;
    uint16_t height = Location.height()/numSquares;
    for( int y = 0 ; y <= numSquares ; y++ )
        if( y % 3 == 0 )
            displayManager.fillRect({Location.left-3,Location.top + height*y -3,Location.right+3,Location.top + height*y +3},15);
        else
            displayManager.fillRect({Location.left-1,Location.top + height*y -1,Location.right+1,Location.top + height*y +1},15);
    for( int x = 0 ; x <= numSquares ; x++ )
        if( x % 3 == 0 )
            displayManager.fillRect({Location.left + width*x -3,Location.top-3,Location.left + width*x +3,Location.bottom+3},15);
        else
            displayManager.fillRect({Location.left + width*x -1,Location.top-1,Location.left + width*x +1,Location.bottom+1},15);
};
