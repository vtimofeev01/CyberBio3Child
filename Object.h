//#pragma once
//
////My headers
//#include <opencv2/core/mat.hpp>
//#include "Settings.h"
//#include "MyTypes.h"
//
//
//
////extern SDL_Renderer* renderer;
//
//
////Only 2 object types atm
enum ObjectTypes
{
	abstract,
	bot
};
//
//
//class Object
//{
//
//private:
//
//
//protected:
//
//	//Time in ticks since object was created
//	unt lifetime = 0;
//
//	//static SDL_Renderer* renderer;
//
//public:
//
//	//Coordinates
//
//
//	//Object type
//	ObjectTypes type;
//
//	Object(int X, int Y) :x(X), y(Y), type(abstract) {};
//
//
//	//Basic 'dummy' draw function if needed
////	virtual void draw(frame_type &image);
//
//	/*This function returns true when the creature dies
//	You should call it on every simulation tick before you
//	call same function in descendant class
//	Returns:
//	0 - all fine
//	1 - object destroyed
//	2 - nothing to do(last tick frame matches current frame)*/
//	virtual int tick();
//
//	static unt currentFrame;
//    //Bot main draw function
//    void draw(frame_type &image);
//	//Returns lifetime
//	unt GetLifetime();
//
//};
//
