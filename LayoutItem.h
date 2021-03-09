#pragma once

#include <memory>
#include <list>
#include <functional>

#include <M5EPD.h>

#include "Utility.h"

class DisplayManager;

class LayoutItemAction
{
public:
    virtual ~LayoutItemAction() = default;

    virtual bool    hasAction() { return true; };
    virtual void    doAction() = 0;
};

class LayoutItemAction_None : public LayoutItemAction
{
public:
    virtual bool    hasAction() { return false; };
    virtual void    doAction() {};
};

class LayoutItemAction_StdFunction : public LayoutItemAction
{
public:
    LayoutItemAction_StdFunction( std::function<void(void)> func ) : Func(func) {};

    std::function<void(void)>   Func;

    virtual bool    hasAction() { return !!Func; };
    virtual void    doAction() { Func(); };
};

class LayoutItem
{
public:
    using tdAction = std::shared_ptr<LayoutItemAction>;
    LayoutItem( Rect<uint16_t> rect, tdAction action = nullptr );
    virtual ~LayoutItem() = default;

    Rect<uint16_t>   Location;
    tdAction    Action;

    virtual void draw( DisplayManager& ) = 0;
    virtual bool hitTest( const Point<uint16_t>& );
};

class LayoutItemWithFont : public LayoutItem
{
public:
    using tdAction = std::shared_ptr<LayoutItemAction>;

    LayoutItemWithFont( Rect<uint16_t> rect, const GFXfont* font = nullptr, uint8_t align = TL_DATUM, tdAction action = nullptr );
    virtual ~LayoutItemWithFont() = default;

    const GFXfont*  Font = nullptr;
    uint8_t  TextAlign = TL_DATUM;

    virtual void drawString( DisplayManager&, String str );
};

class LayoutItem_ButtonIcon : public LayoutItem
{
public:
    LayoutItem_ButtonIcon( Rect<uint16_t> rect, const unsigned char* data, size_t size, tdAction action = nullptr );

    const unsigned char* Data = nullptr;
    size_t  DataSize = 0;
    virtual void draw( DisplayManager& ) override;
};

class LayoutItem_ButtonIconWithHighlight : public LayoutItem_ButtonIcon
{
public:
    using tdHighlightFunc = std::function<bool(void)>;
    LayoutItem_ButtonIconWithHighlight( Rect<uint16_t> rect, const unsigned char* data, size_t size, tdHighlightFunc func = nullptr, tdAction action = nullptr );

    tdHighlightFunc HighlightFunc;
    virtual void draw( DisplayManager& ) override;
};

class LayoutItem_Rectangle : public LayoutItem
{
    using LayoutItem::LayoutItem;
    virtual void draw( DisplayManager& ) override;
};

class LayoutItem_StaticText : public LayoutItemWithFont
{
public:
    LayoutItem_StaticText( Rect<uint16_t> rect, const GFXfont* font, uint8_t align, String text, tdAction action = nullptr );

    String  Text;
    virtual void draw( DisplayManager& ) override;
};

class LayoutItem_DynamicText : public LayoutItemWithFont
{
public:
    using tdStringFunc = std::function<String(void)>;
    using tdOutlineFunc = std::function<bool(void)>;
    LayoutItem_DynamicText( Rect<uint16_t> rect, const GFXfont* font, uint8_t align, tdStringFunc textFunc, tdOutlineFunc outlineFunc = [](){return false;}, tdAction action = nullptr );

    tdStringFunc    TextFunc;
    tdOutlineFunc   OutlineFunc;
    virtual void draw( DisplayManager& ) override;
};

class LayoutItemAction_SudokuSquare : public LayoutItemAction
{
public:
    LayoutItemAction_SudokuSquare( uint8_t x, uint8_t y );

    Point<uint8_t>  WhichSquare;
    virtual void    doAction()  override;
};

class LayoutItem_SudokuSquare : public LayoutItem
{
public:
    LayoutItem_SudokuSquare( Rect<uint16_t> rect, uint8_t x, uint8_t y );

    Point<uint8_t>  WhichSquare;
    virtual void draw( DisplayManager& ) override;
};

class LayoutItemAction_SudokuSubSquare : public LayoutItemAction
{
public:
    LayoutItemAction_SudokuSubSquare( uint8_t val );

    uint8_t         WhichValue;
    virtual void    doAction()  override;
};

class LayoutItem_SudokuSubSquare : public LayoutItem
{
public:
    LayoutItem_SudokuSubSquare( Rect<uint16_t> rect, uint8_t val );

    uint8_t         WhichValue;
    virtual void draw( DisplayManager& ) override;
};

class LayoutItem_SudokuMainBackground : public LayoutItem
{
public:
    LayoutItem_SudokuMainBackground( Rect<uint16_t> rect );

    virtual void draw( DisplayManager& ) override;
};

class LayoutItem_SudokuGrid : public LayoutItem
{
public:
    LayoutItem_SudokuGrid( Rect<uint16_t> rect, bool bSub = false );

    bool SubGrid = false;
    virtual void draw( DisplayManager& ) override;
};
