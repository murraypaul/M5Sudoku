#pragma once

#include <list>
#include <memory>

#include <M5EPD.h>

#include "Utility.h"

class LayoutItem;

struct DisplayManager
{
    enum eLayout {
        eBoard_Landscape,
        eBoard_Portait,

        eSingleSquare,

        eNewGame
    };

protected:
    M5EPD_Canvas Canvas;
    Size<uint16_t>  CanvasSize;
    Point<uint16_t>  CanvasPos;
    uint16_t Rotation = 0;
    eLayout  CurrentLayout = eBoard_Landscape;
    std::list<std::shared_ptr<LayoutItem>>    LayoutItems;
    bool ShouldClose = false;
    bool Cancelled = false;
    bool PopupDialogActive = false;
    Rect<uint16_t>   DesiredUpdateRect{0,0,0,0};
    m5epd_update_mode_t     DesiredUpdateMode = UPDATE_MODE_NONE;

public:
    DisplayManager();
    void Init( bool appInit = false );
    void SetLayout( eLayout );

    void draw( bool bFullRedraw = false );
    void redraw();

    M5EPD_Canvas&   GetCanvas();
    void drawRect( const Rect<uint16_t>& rect, uint32_t colour );
    void fillRect( const Rect<uint16_t>& rect, uint32_t colour );
    void drawString( const GFXfont* font, uint8_t datum, String str, const Rect<uint16_t>& rect );
    void drawString( const GFXfont* font, uint8_t datum, String str, uint32_t x, uint32_t y );
    
    void clearScreen();
    void refreshScreen( m5epd_update_mode_t mode = UPDATE_MODE_GC16 );
    void M5EPD_flushAndUpdateArea( const Rect<uint16_t>& rect, m5epd_update_mode_t updateMode );

    void doLoop( bool enableButtons = true );

    void HandleButtonL();
    void HandleButtonP();
    void HandleButtonR();
    void HandleSingleFinger( const Point<uint16_t>& hit );

    void ShowNewGameDialog();

    void doShutdownIfOnBattery();
    void doShutdown();
};

extern DisplayManager BaseDisplayManager;
