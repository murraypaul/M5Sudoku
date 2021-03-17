#include "DisplayManager.h"

#include "Utility.h"

#include "LayoutItem.h"

#include <Preferences.h>

#include "SudokuState.h"

extern SudokuState CurrentState;
extern String LastValidation;

Preferences preferences;
const char* Preferences_App = "M5Sudoku";

std::vector<uint8_t> vector_rand81(81);
std::random_device rd_;
std::mt19937 g_(rd_());

uint8_t  TargetFixedCells = 24;
uint32_t TargetSolveTimeMS = 60 * 1000;

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

        std::iota(vector_rand81.begin(), vector_rand81.end(), 0);
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
                        Rect<uint16_t>(Point<uint16_t>(offset + border + x*widthOne,border + y*widthOne),Size<uint16_t>(widthOne,widthOne)), 1 + y*3 + x) );
            LayoutItems.push_back( std::make_shared<LayoutItem_SudokuGrid>(
                Rect<uint16_t>(offset + border,border,offset + width - border,width - border), true));
        }
        {
            uint16_t border = 18;
            uint16_t width = (540) * 2 / 3 + 2*border;
            uint16_t offsetX = 540-border + (960-540-width)/2;
            uint16_t offsetY = width - 3*border;
            uint16_t lineHeight = 56;
            uint8_t itemCount = 0;
            uint16_t itemBorder = 9;
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX + border,offsetY + itemCount*(lineHeight + itemBorder)),Size<uint16_t>(1*width/2 - border,lineHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return "Validate"; }
                , []() -> bool { return true; } 
                , std::make_shared<LayoutItemAction_StdFunction>([]()
                {
                    SudokuState temp = CurrentState;
                    temp.Propagate();
                    uint8_t result = temp.SolveUniquely();
                    LastValidation = result == 1 ? CurrentState.Solved() ? "Solved!" : "Valid" : result == 2 ? "Non-unique" : "Invalid";
                    BaseDisplayManager.draw(true);
                })));
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX+1*width/2 + border,offsetY + itemCount*(lineHeight + itemBorder)),Size<uint16_t>(1*width/2 - border,lineHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return LastValidation; }
                , []() -> bool { return false; } 
                , nullptr));
            itemCount++;
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX + border,offsetY + itemCount*(lineHeight + itemBorder)),Size<uint16_t>(1*width/2 - border,lineHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return "New Game"; }
                , []() -> bool { return true; } 
                , std::make_shared<LayoutItemAction_StdFunction>([this]()
                {
                    this->ShowNewGameDialog();
                })));
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX+width/2 + border,offsetY + itemCount*(lineHeight + itemBorder)),Size<uint16_t>(1*width/2 - border,lineHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return !CurrentState.Solved() ? "Clue" : ""; }
                , []() -> bool { return !CurrentState.Solved() ? true : false; } 
                , std::make_shared<LayoutItemAction_StdFunction>([]()
                {
                    CurrentState.FixOneSquare();
                    LastValidation = "";
                    BaseDisplayManager.draw();
                })));
            itemCount++;
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX+width/2 + border,offsetY + itemCount*(lineHeight + itemBorder)),Size<uint16_t>(width/2 - border,lineHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return "Save"; }
                , []() -> bool { return true; } 
                , std::make_shared<LayoutItemAction_StdFunction>([]()
                {
                    CurrentState.Save();
                    BaseDisplayManager.draw();
                })));
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX + border,offsetY + itemCount*(lineHeight + itemBorder)),Size<uint16_t>(width/2 - border,lineHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return SudokuState::HasSave() ? "Load" : ""; }
                , []() -> bool { return SudokuState::HasSave() ? true : false; } 
                , std::make_shared<LayoutItemAction_StdFunction>([]()
                {
                    if( SudokuState::HasSave() )
                    {
                        CurrentState.Load();
                        LastValidation = "";
                        BaseDisplayManager.draw(true);
                    }
                })));

        }
        break;
    case eLayout::eNewGame:
        Rotation = 0 + (flip?180:0);
        CanvasPos = {120,64};
        CanvasSize = {960-120*2,540-64*2};
        Rect<uint16_t> canvasRect{{0,0},CanvasSize};

        clearCanvas = false;

        LayoutItems.push_back( std::make_shared<LayoutItem_Rectangle>(canvasRect) );
        LayoutItems.push_back( std::make_shared<LayoutItem_Rectangle>(canvasRect.shrinkBy({5,5})) );
        LayoutItems.push_back( std::make_shared<LayoutItem_StaticText>(Rect<uint16_t>(Point<uint16_t>(20,10),Size<uint16_t>(CanvasSize.cx-40,64)),&FreeSansBold24pt7b,TC_DATUM,String("New Game"),nullptr) );

        LayoutItems.push_back( std::make_shared<LayoutItem_StaticText>(Rect<uint16_t>(Point<uint16_t>(20,10+64),Size<uint16_t>(CanvasSize.cx-40,64)),&FreeSansBold12pt7b,TC_DATUM,String("Target Clues"),nullptr) );
        {
            uint16_t border = 32;
            uint16_t width = CanvasSize.cx - border*2;
            uint16_t offsetX = border;
            uint16_t offsetY = 128-8;
            uint16_t itemBorder = 32;
            uint16_t itemWidth = (width - itemBorder*3)/4;
            uint16_t itemHeight = 64;
            uint16_t itemCount = 0;
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX + itemCount*(itemWidth+itemBorder),offsetY),Size<uint16_t>(itemWidth,itemHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return "22"; }
                , []() -> bool { return TargetFixedCells == 22; } 
                , std::make_shared<LayoutItemAction_StdFunction>([this]() { TargetFixedCells = 22; this->draw(); } )));
            itemCount++;
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX + itemCount*(itemWidth+itemBorder),offsetY),Size<uint16_t>(itemWidth,itemHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return "24"; }
                , []() -> bool { return TargetFixedCells == 24; } 
                , std::make_shared<LayoutItemAction_StdFunction>([this]() { TargetFixedCells = 24; this->draw(); } )));
            itemCount++;
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX + itemCount*(itemWidth+itemBorder),offsetY),Size<uint16_t>(itemWidth,itemHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return "26"; }
                , []() -> bool { return TargetFixedCells == 26; } 
                , std::make_shared<LayoutItemAction_StdFunction>([this]() { TargetFixedCells = 26; this->draw(); } )));
            itemCount++;
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX + itemCount*(itemWidth+itemBorder),offsetY),Size<uint16_t>(itemWidth,itemHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return "28"; }
                , []() -> bool { return TargetFixedCells == 28; } 
                , std::make_shared<LayoutItemAction_StdFunction>([this]() { TargetFixedCells = 28; this->draw(); } )));
        }

        LayoutItems.push_back( std::make_shared<LayoutItem_StaticText>(Rect<uint16_t>(Point<uint16_t>(20,10+128+64),Size<uint16_t>(CanvasSize.cx-40,64)),&FreeSansBold12pt7b,TC_DATUM,String("Time to Find"),nullptr) );
        {
            uint16_t border = 32;
            uint16_t width = CanvasSize.cx - border*2;
            uint16_t offsetX = border;
            uint16_t offsetY = 256-16;
            uint16_t itemBorder = 32;
            uint16_t itemWidth = (width - itemBorder*3)/4;
            uint16_t itemHeight = 64;
            uint16_t itemCount = 0;
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX + itemCount*(itemWidth+itemBorder),offsetY),Size<uint16_t>(itemWidth,itemHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return "30s"; }
                , []() -> bool { return TargetSolveTimeMS == 30*1000; } 
                , std::make_shared<LayoutItemAction_StdFunction>([this]() { TargetSolveTimeMS = 30*1000; this->draw(); } )));
            itemCount++;
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX + itemCount*(itemWidth+itemBorder),offsetY),Size<uint16_t>(itemWidth,itemHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return "60s"; }
                , []() -> bool { return TargetSolveTimeMS == 60*1000; } 
                , std::make_shared<LayoutItemAction_StdFunction>([this]() { TargetSolveTimeMS = 60*1000; this->draw(); } )));
            itemCount++;
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX + itemCount*(itemWidth+itemBorder),offsetY),Size<uint16_t>(itemWidth,itemHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return "90s"; }
                , []() -> bool { return TargetSolveTimeMS == 90*1000; } 
                , std::make_shared<LayoutItemAction_StdFunction>([this]() { TargetSolveTimeMS = 90*1000; this->draw(); } )));
            itemCount++;
            LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
                Rect<uint16_t>(Point<uint16_t>(offsetX + itemCount*(itemWidth+itemBorder),offsetY),Size<uint16_t>(itemWidth,itemHeight))
                , &FreeSans12pt7b, CC_DATUM
                , []() -> String { return "120s"; }
                , []() -> bool { return TargetSolveTimeMS == 120*1000; } 
                , std::make_shared<LayoutItemAction_StdFunction>([this]() { TargetSolveTimeMS = 120*1000; this->draw(); } )));
        }

        LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
            Rect<uint16_t>(Point<uint16_t>(CanvasSize.cx - 400,CanvasSize.cy-84),Size<uint16_t>(180, 64))
            , &FreeSans12pt7b, CC_DATUM
            , []() -> String { return "Cancel"; }
            , []() -> bool { return true; } 
            , std::make_shared<LayoutItemAction_StdFunction>([this]()
            {
                this->Cancelled = true;
                this->ShouldClose = true;
            })));

        LayoutItems.push_back( std::make_shared<LayoutItem_DynamicText>(
            Rect<uint16_t>(Point<uint16_t>(CanvasSize.cx - 200,CanvasSize.cy-84),Size<uint16_t>(180, 64))
            , &FreeSans12pt7b, CC_DATUM
            , []() -> String { return "Go"; }
            , []() -> bool { return true; } 
            , std::make_shared<LayoutItemAction_StdFunction>([this]()
            {
                this->ShouldClose = true;
            })));

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

void DisplayManager::draw( bool bFullRedraw ) 
{
    DesiredUpdateRect = Rect<uint16_t>{0,0,0,0};
    DesiredUpdateMode = UPDATE_MODE_NONE;

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
    //vTaskDelay(1);
    static uint32_t lastActive = millis();
    static uint32_t inactivityTimeout = 5 * 60 * 1000;

    auto ts = millis();
    if( ts > lastActive + inactivityTimeout )
    {
        Rect<uint16_t> voltRect(CanvasSize.cx-60,0,CanvasSize.cx,60);
        fillRect(voltRect,0);
        drawString(&FreeSans9pt7b,BL_DATUM,String(M5.getBatteryVoltage()/1000.0), voltRect);
        M5EPD_flushAndUpdateArea(voltRect,UPDATE_MODE_GC16);
        doShutdownIfOnBattery();
        lastActive = ts;
    }

    // Prevent repeated press detection
    static bool wasFingerDown = false;
//    static bool wasButtonPressed = false;

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
//        if( enableButtons )
//            wasButtonPressed = false;
        if( M5.TP.avaliable() )
        {
            if( !M5.TP.isFingerUp() )
            {
                M5.TP.update();
                Point<uint16_t> f1 = {M5.TP.readFingerX(0), M5.TP.readFingerY(0)};
//                Point<uint16_t> f2 = {M5.TP.readFingerX(1), M5.TP.readFingerY(1)};
                auto numFingers = M5.TP.getFingerNum();
    //            delay(100);
                M5.TP.flush();
                // Get spurious touches at startup
                if( f1 != Point<uint16_t>(0,0) && !wasFingerDown )
                {
                    wasFingerDown = true;
                    lastActive = millis();
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
//        , hitIn.x, hitIn.y, hit.x, hit.y, CanvasPos.x, CanvasPos.y );
    for( auto& item : LayoutItems )
        if( item->hitTest(hit) )
            break;
}

/*

*/

void DisplayManager::ShowNewGameDialog()
{
    DisplayManager newGameDlg;
    newGameDlg.Rotation = Rotation;
    newGameDlg.SetLayout(eLayout::eNewGame);

    newGameDlg.redraw();
    newGameDlg.ShouldClose = false;

    PopupDialogActive = true;
    while( !newGameDlg.ShouldClose )
    {
        newGameDlg.doLoop(false);
        delay(100);
//        yield();
    }

    newGameDlg.clearScreen();
    if( !newGameDlg.Cancelled )
    {
        newGameDlg.drawString(&FreeSans24pt7b,CC_DATUM,"Please wait...",Rect<uint16_t>(Point<uint16_t>(0,0),newGameDlg.CanvasSize));
        newGameDlg.Canvas.pushCanvas(newGameDlg.CanvasPos.x,newGameDlg.CanvasPos.y,UPDATE_MODE_DU);

        SudokuState temp;
        temp.GenerateRandom(TargetFixedCells,TargetSolveTimeMS);
        CurrentState = temp;
        LastValidation = "";
        SudokuState::RemoveSave();
    }

    Canvas.pushCanvas(CanvasPos.x,CanvasPos.y,UPDATE_MODE_GC16);
    PopupDialogActive = false;

    BaseDisplayManager.draw(true);
}

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
    CurrentState.Save();

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
