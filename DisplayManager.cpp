#include "DisplayManager.h"

#include "Utility.h"

#include "LayoutItem.h"

#include <Preferences.h>

Preferences preferences;
const char* Preferences_App = "M5Sudoku";

DisplayManager BaseDisplayManager;

DisplayManager::DisplayManager()
: Canvas(&M5.EPD)
{

}

M5EPD_Canvas& DisplayManager::GetCanvas()
{
    return Canvas;
}

void DisplayManager::Init( bool appInit )
{
    if( appInit )
    {
        M5.begin();

        preferences.begin(Preferences_App);
        CurrentLayout = (eLayout)preferences.getChar("Layout",(int8_t)CurrentLayout);
        Rotation = preferences.getShort("Rotation",(int16_t)Rotation);
        preferences.end();
        log_d("Battery voltage: %d", M5.getBatteryVoltage());
    }

    SetLayout(CurrentLayout);
}

void DisplayManager::SetLayout( eLayout layout )
{
    Canvas.deleteCanvas();

    LayoutItems.clear();

    bool clearCanvas = true;
    bool flip = Rotation >= 180;
    switch( layout )
    {
    case eLayout::eBoard_Portait:
    default:
        Rotation = 90 + (flip?180:0);
        CanvasPos = {0,0};
        CanvasSize = {540,960};
        {
            uint16_t border = 18;
            uint16_t width = (540 - 2 * border) / 9;
            for( uint8_t x = 0 ; x < 9 ; x++ )
                for( uint8_t y = 0 ; y < 9 ; y++ )
                    LayoutItems.push_back( std::make_shared<LayoutItem_SudokuSquare>(
                        Rect<uint16_t>(border + x*width,border + y*width,border + (x+1)*width,border + (y+1)*width), x, y) );
        }
        break;
    case eLayout::eBoard_Landscape:
        Rotation = 0 + (flip?180:0);
        CanvasPos = {0,0};
        CanvasSize = {960,540};
        {
            uint16_t border = 18;
            uint16_t width = 540;
            uint16_t widthOne = (width - 2 * border) / 9;
            LayoutItems.push_back( std::make_shared<LayoutItem_SudokuMainBackground>(
                Rect<uint16_t>(border,border,width - border,width - border)));
            for( uint8_t x = 0 ; x < 9 ; x++ )
                for( uint8_t y = 0 ; y < 9 ; y++ )
                    LayoutItems.push_back( std::make_shared<LayoutItem_SudokuSquare>(
                        Rect<uint16_t>(border + x*widthOne,border + y*widthOne,border + (x+1)*widthOne,border + (y+1)*widthOne), x, y) );
            LayoutItems.push_back( std::make_shared<LayoutItem_SudokuGrid>(
                Rect<uint16_t>(border,border,width - border,width - border)));
        }
        {
            uint16_t border = 18;
            uint16_t width = (540 - 2 * border) * 2 / 3;
            uint16_t offset = 540-border + (960-540-width)/2;
            uint16_t widthOne = (width - 2 * border) / 3;
            LayoutItems.push_back( std::make_shared<LayoutItem_SudokuMainBackground>(
                Rect<uint16_t>(offset + border,border,offset + width - border,width - border)));
            for( uint8_t x = 0 ; x < 3 ; x++ )
                for( uint8_t y = 0 ; y < 3 ; y++ )
                    LayoutItems.push_back( std::make_shared<LayoutItem_SudokuSubSquare>(
                        Rect<uint16_t>(offset + border + x*widthOne,border + y*widthOne,offset + border + (x+1)*widthOne,border + (y+1)*widthOne), 1 + y*3 + x) );
            LayoutItems.push_back( std::make_shared<LayoutItem_SudokuGrid>(
                Rect<uint16_t>(offset + border,border,offset + width - border,width - border), true));
        }
        break;
    }

    CurrentLayout = layout;

    Rotation = Rotation % 360;
    log_d("New rotation %d",Rotation);

    M5.TP.SetRotation(Rotation);
    M5.EPD.SetRotation(Rotation);
    if( clearCanvas )
        M5.EPD.Clear(true);

    log_d("Canvas size (%d,%d) at (%d,%d)", CanvasSize.cx, CanvasSize.cy, CanvasPos.x, CanvasPos.y);
    Canvas.createCanvas(CanvasSize.cx,CanvasSize.cy);
}

void DisplayManager::drawString( const GFXfont* font, uint8_t datum, String str, const Rect<uint16_t>& rect )
{
    switch( datum )
    {
        case TL_DATUM:
        default:
            drawString(font, datum, str, rect.left, rect.top);
            break;
        case TR_DATUM:
            drawString(font, datum, str, rect.right, rect.top);
            break;
        case TC_DATUM:
            drawString(font, datum, str, (rect.left+rect.right)/2, rect.top);
            break;
        case BL_DATUM:
            drawString(font, datum, str, rect.left, rect.bottom);
            break;
        case BR_DATUM:
            drawString(font, datum, str, rect.right, rect.bottom);
            break;
        case BC_DATUM:
            drawString(font, datum, str, (rect.left+rect.right)/2, rect.bottom);
            break;
        case CL_DATUM:
            drawString(font, datum, str, rect.left, (rect.top+rect.bottom)/2);
            break;
        case CR_DATUM:
            drawString(font, datum, str, rect.right, (rect.top+rect.bottom)/2);
            break;
        case CC_DATUM:
            drawString(font, datum, str, (rect.left+rect.right)/2, (rect.top+rect.bottom)/2);
            break;
    }
}

void DisplayManager::drawString( const GFXfont* font, uint8_t datum, String str, uint32_t x, uint32_t y )
{
    if( font )
        Canvas.setFreeFont(font);
    Canvas.setTextDatum(datum);
    Canvas.drawString(str, x, y);
}

void DisplayManager::draw() 
{
    DesiredUpdateRect = Rect<uint16_t>{0,0,0,0};
    DesiredUpdateMode = UPDATE_MODE_NONE;

    bool bFullRedraw = false;
    if( bFullRedraw )
        clearScreen();

    Rect<uint16_t> redrawRect{0,0,0,0};
    for( auto& item : LayoutItems )
    {
        if( !item.get( ))
            continue;
        {
            item->draw( *this );
            if( redrawRect.width() == 0 )
                redrawRect = item->Location;
            else
                redrawRect = redrawRect.outersect(item->Location);
        }
    }
//    log_i("Redraw rect = (%d,%d,%d,%d)",redrawRect.left,redrawRect.top,redrawRect.right,redrawRect.bottom);
    if( bFullRedraw )
    {
//        log_i("New track");
        DesiredUpdateRect = Rect<uint16_t>(CanvasPos,CanvasSize);
        DesiredUpdateMode = UPDATE_MODE_GC16;
    }
    else if( redrawRect.width() > 0 )
    {
//        log_i("Have redraw rect");
        DesiredUpdateRect = redrawRect;
        DesiredUpdateMode = UPDATE_MODE_DU4;
    }

    if( DesiredUpdateMode != UPDATE_MODE_NONE )
    {
//        log_i("Delayed update");
        M5.EPD.WritePartGram4bpp(CanvasPos.x, CanvasPos.y, CanvasSize.cx, CanvasSize.cy, (uint8_t*)Canvas.frameBuffer());
        if( !PopupDialogActive )
        {
//            log_i("Delayed update - no popup");
            M5.EPD.UpdateArea(CanvasPos.x, CanvasPos.y, CanvasSize.cx, CanvasSize.cy, DesiredUpdateMode);
            DesiredUpdateMode = UPDATE_MODE_NONE;
        }
    }
    if( this != &BaseDisplayManager && BaseDisplayManager.DesiredUpdateMode != UPDATE_MODE_NONE )
    {
//        log_i("Delayed update - not base");
        M5.EPD.WritePartGram4bpp(BaseDisplayManager.CanvasPos.x, BaseDisplayManager.CanvasPos.y, BaseDisplayManager.CanvasSize.cx, BaseDisplayManager.CanvasSize.cy, (uint8_t*)BaseDisplayManager.Canvas.frameBuffer());
        M5.EPD.WritePartGram4bpp(CanvasPos.x, CanvasPos.y, CanvasSize.cx, CanvasSize.cy, (uint8_t*)Canvas.frameBuffer());
        Rect<uint16_t> updateRect = Rect<uint16_t>{CanvasPos,CanvasSize}.outersect(BaseDisplayManager.DesiredUpdateRect);        
        M5.EPD.UpdateArea(updateRect.left, updateRect.top, updateRect.right, updateRect.bottom, BaseDisplayManager.DesiredUpdateMode);
        BaseDisplayManager.DesiredUpdateMode = UPDATE_MODE_NONE;
    }    
}

void DisplayManager::redraw()
{
    for( auto& item : LayoutItems )
    {
        if( !item.get( ))
            continue;
        item->draw( *this );
    }
    refreshScreen(UPDATE_MODE_GC16);
}

void DisplayManager::clearScreen()
{
    Canvas.fillCanvas(0);
}

void DisplayManager::refreshScreen( m5epd_update_mode_t mode )
{
    Canvas.pushCanvas(CanvasPos.x,CanvasPos.y,mode);
}

void DisplayManager::drawRect( const Rect<uint16_t>& rect, uint32_t colour )
{
    Canvas.drawRect(rect.left,rect.top,rect.width(),rect.height(),colour);
}
void DisplayManager::fillRect( const Rect<uint16_t>& rect, uint32_t colour )
{
    Canvas.fillRect(rect.left,rect.top,rect.width(),rect.height(),colour);
}
void DisplayManager::M5EPD_flushAndUpdateArea( const Rect<uint16_t>& rect, m5epd_update_mode_t updateMode )
{
    M5.EPD.WriteFullGram4bpp((uint8_t*)Canvas.frameBuffer());
    M5.EPD.UpdateArea(rect.left, rect.top, rect.width(), rect.height(),updateMode);
} 

void DisplayManager::doLoop( bool enableButtons )
{
    // Prevent repeated press detection
    static bool wasFingerDown = false;
    static bool wasButtonPressed = false;

    M5.update();
//    if (enableButtons && !wasButtonPressed && M5.BtnL.wasPressed()) {
//        log_i("BtnL - Previous");
//        wasButtonPressed = true;
//        HandleButtonL();
//    }
//    else if (enableButtons && !wasButtonPressed && M5.BtnP.wasPressed()) {
//        log_i("BtnP - Toggle");
//        wasButtonPressed = true;
//        HandleButtonP();
//    }
//    else if (enableButtons && !wasButtonPressed && M5.BtnR.wasPressed()) {
//        log_i("BtnR - Next");
//        wasButtonPressed = true;
//        HandleButtonR();
//    }
//    else
    {
        if( enableButtons )
            wasButtonPressed = false;
        if( M5.TP.avaliable() )
        {
            if( !M5.TP.isFingerUp() )
            {
                M5.TP.update();
                Point<uint16_t> f1 = {M5.TP.readFingerX(0), M5.TP.readFingerY(0)};
                Point<uint16_t> f2 = {M5.TP.readFingerX(1), M5.TP.readFingerY(1)};
                auto numFingers = M5.TP.getFingerNum();
    //            delay(100);
                M5.TP.flush();
                // Get spurious touches at startup
                if( f1 != Point<uint16_t>(0,0) && !wasFingerDown )
                {
                    wasFingerDown = true;
                    switch( numFingers )
                    {
                        case 2:
                //          HandleDoubleFinger( f1.first, f1.second, f2.first, f2.second );
                        break;
                        case 1:
                            HandleSingleFinger( f1 );
                        break;
                    }
                    delay(200);
                }
            } else
                wasFingerDown = false;
        } else
            wasFingerDown = false;
    }

    if( DesiredUpdateMode != UPDATE_MODE_NONE )
    {
//        log_i("Delayed update");
        M5.EPD.WritePartGram4bpp(CanvasPos.x, CanvasPos.y, CanvasSize.cx, CanvasSize.cy, (uint8_t*)Canvas.frameBuffer());
        if( !PopupDialogActive )
        {
//            log_i("Delayed update - no popup");
            M5.EPD.UpdateArea(CanvasPos.x, CanvasPos.y, CanvasSize.cx, CanvasSize.cy, DesiredUpdateMode);
            DesiredUpdateMode = UPDATE_MODE_NONE;
        }
    }
    if( this != &BaseDisplayManager && BaseDisplayManager.DesiredUpdateMode != UPDATE_MODE_NONE )
    {
//        log_i("Delayed update - not base");
        M5.EPD.WritePartGram4bpp(BaseDisplayManager.CanvasPos.x, BaseDisplayManager.CanvasPos.y, BaseDisplayManager.CanvasSize.cx, BaseDisplayManager.CanvasSize.cy, (uint8_t*)BaseDisplayManager.Canvas.frameBuffer());
        M5.EPD.WritePartGram4bpp(CanvasPos.x, CanvasPos.y, CanvasSize.cx, CanvasSize.cy, (uint8_t*)Canvas.frameBuffer());
        Rect<uint16_t> updateRect = Rect<uint16_t>{CanvasPos,CanvasSize}.outersect(BaseDisplayManager.DesiredUpdateRect);        
        M5.EPD.UpdateArea(updateRect.left, updateRect.top, updateRect.right, updateRect.bottom, BaseDisplayManager.DesiredUpdateMode);
        BaseDisplayManager.DesiredUpdateMode = UPDATE_MODE_NONE;
    }
}

void DisplayManager::HandleButtonL() { };
void DisplayManager::HandleButtonP() { };
void DisplayManager::HandleButtonR() { };
void DisplayManager::HandleSingleFinger( const Point<uint16_t>& hitIn )
{
    Point<uint16_t> hit = hitIn - CanvasPos;
//    log_d("Converted hit from (%d,%d) to (%d,%d), canvas pos (%d,%d)"
        , hitIn.x, hitIn.y, hit.x, hit.y, CanvasPos.x, CanvasPos.y );
    for( auto& item : LayoutItems )
        if( item->hitTest(hit) )
            break;
}
/*
void DisplayManager::ShowSettingsMenu()
{
    DisplayManager settingsManager;
    settingsManager.Rotation = Rotation;
    settingsManager.SetLayout(eLayout::eSettings);

    settingsManager.redraw();
    settingsManager.ShouldClose = false;

    PopupDialogActive = true;
    while( !settingsManager.ShouldClose )
    {
        settingsManager.doLoop(false);
        delay(100);
//        yield();
    }

    preferences.begin(Preferences_App);
    preferences.putChar("Layout",(int8_t)CurrentLayout);
    preferences.putShort("Rotation",(int16_t)Rotation);
    preferences.putShort("MaxJpeg",(int16_t)MaxPreferredImageSize);
    preferences.end();
    
    settingsManager.clearScreen();
    settingsManager.Canvas.pushCanvas(settingsManager.CanvasPos.x,settingsManager.CanvasPos.y,UPDATE_MODE_GC16);
    Canvas.pushCanvas(CanvasPos.x,CanvasPos.y,UPDATE_MODE_GC16);
    PopupDialogActive = false;
}
*/
void DisplayManager::doShutdownIfOnBattery()
{
    // Cannot tell for sure, guess
    uint32_t bv = M5.getBatteryVoltage();
    log_d("Battery voltage %d vs target of 4200",bv);
    if( bv < 4200 )
        doShutdown();
}

void DisplayManager::doShutdown()
{
    Point<uint16_t> windowPoint;
    if( Rotation % 180 == 0 )
        windowPoint = {280,70};
    else
        windowPoint = {70,280};
    Rect<uint16_t> windowRect = {windowPoint,{400,400}};

    M5EPD_Canvas tempCanvas(&M5.EPD);
    tempCanvas.createCanvas(windowRect.width(),windowRect.height());

    tempCanvas.fillRect(0,0,windowRect.width(),windowRect.height(),0);
    tempCanvas.drawRect(0,0,windowRect.width(),windowRect.height(),15);
    tempCanvas.drawRect(5,5,windowRect.width()-10,windowRect.height()-10,15);

    tempCanvas.setTextDatum(TC_DATUM);
    tempCanvas.setFreeFont(&FreeSansBold24pt7b);
    tempCanvas.drawString("Shutdown",windowRect.width()/2,50);
    tempCanvas.setFreeFont(&FreeSansBold18pt7b);
    tempCanvas.drawString("Hold side button",windowRect.width()/2,200);
    tempCanvas.drawString("to restart",windowRect.width()/2,240);

    tempCanvas.pushCanvas(windowPoint.x,windowPoint.y,UPDATE_MODE_GC16);
    
    log_d("About to disable main power");
    delay(200);
    M5.disableMainPower();

    while( true )
        yield();
}
