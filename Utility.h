#pragma once

template <class T> struct Point {
    T x;
    T y;

    Point() : x(T{}), y(T{}) {}; 
    Point( T x_, T y_ ) : x(x_), y(y_) {}; 

    bool operator==( const Point<T>& other ) const { return x == other.x && y == other.y; };
    bool operator!=( const Point<T>& other ) const { return !(*this  == other); };
    Point<T> operator+( const Point<T>& other ) const { return {x+other.x,y+other.y}; };
    Point<T> operator-( const Point<T>& other ) const { return {x-other.x,y-other.y}; };
};

template <class T> struct Size {
    T cx;
    T cy;

    Size() : cx(T{}), cy(T{}) {}; 
    Size( T x, T y ) : cx(x), cy(y) {}; 

    bool operator==( const Size<T>& other ) { return cx == other.cx && cy == other.cy; };
    bool operator!=( const Size<T>& other ) { return !(*this  == other); };
};

template<class T> struct Rect {
  T left;
  T right;
  T top;
  T bottom;

  Rect(): left(T{}), top(T{}), right(T{}), bottom(T{}) {};
  Rect( T l, T t, T r, T b ): left(l), top(t), right(r), bottom(b) {};
  Rect( Point<T> p, Size<T> s ) : left(p.x), top(p.y), right(p.x+s.cx), bottom(p.y+s.cy) {};

  T width() const { return right-left; };
  T height() const { return bottom-top; };

  Rect<T>   outersect( const Rect<T>& other ) const
  {
      return {min(left,other.left), min(top,other.top), max(right,other.right), max(bottom,other.bottom)};
  }
  Rect<T>   scaleBy( double x, double y ) const
  {
      return {left,top,left+width()*x,top+height()*y};
  }
  Rect<T>   shrinkBy( const Size<T>& sz ) const
  {
      return {left+sz.cx,top+sz.cy,right-2*sz.cx,bottom-2*sz.cy};
  }
  Rect<T>   shrinkBy( T x, T y ) const
  {
      return {left+x,top+y,right-2*x,bottom-2*y};
  }
  Rect<T>   add( const Point<T>& pt )
  {
      return {left+pt.x,top+pt.y,right+pt.x,bottom+pt.y};
  }
  bool      contains( const Point<T>& pt ) const
  {
      return left <= pt.x && pt.x <= right && top <= pt.y && pt.y <= bottom;
  }
  bool      contains( T x, T y ) const
  {
      return left <= x && x <= right && top <= y && y <= bottom;
  }
};
