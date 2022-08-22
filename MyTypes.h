#pragma once
#include "Systems.h"


typedef unsigned int unt;
typedef unsigned char byte;


//class Point
//{
//public:
//	int x, y;
//
//	Point(int X, int Y) :x(X), y(Y) {};
//
//	Point():x(-1), y(-1) {};
//
//	void Shift(int X, int Y)
//	{
//		x += X;
//		y += Y;
//	}
//
//	void Set(int X, int Y)
//	{
//		x = X;
//		y = Y;
//	}
//};

//class Rect
//{
//    int x, y, w, h;
//	bool IsInBounds(Point& p) const
//	{
//		return ((p.x >= x) && (p.y >= y) && (p.x <= x + w) && (p.y <= y + h));
//	}
//};



//Get random number (0 to max-1)
#define RandomVal(max) (rand() % (max))

//Roll for probability (chance in percents)
#define RandomPercent(val) ((rand()%1000)>=(1000 - ((val) * 10)))

//Simple cycle
#define repeat(times) for(int i=0;i<(times);++i)
