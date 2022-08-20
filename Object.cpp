//#include <opencv2/imgproc.hpp>
//#include "Object.h"
//
//
////void Object::draw(frame_type &image)
////{
//////	SDL_Rect rect = { FieldX + x * FieldCellSize, FieldY + y * FieldCellSize, FieldCellSize, FieldCellSize };
//////
//////	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 111);
//////	SDL_RenderFillRect(renderer, &rect);
////}
//
//int Object::tick()
//{
//	if (currentFrame == lastTickFrame)
//		return 2;
//
//	++lifetime;
//	lastTickFrame = currentFrame;
//
//	return 0;
//}
//
//unt Object::currentFrame = 0;
//
////Returns lifetime
//unt Object::GetLifetime()
//{
//	return lifetime;
//}
//
//void Object::draw(frame_type &image) {
//
//    cv::rectangle(image,
//                  oPoint(FieldX + x * FieldCellSize, FieldY + y * FieldCellSize),
//                  oPoint(FieldX + x * FieldCellSize + FieldCellSize, FieldY + y * FieldCellSize + FieldCellSize),
//                  cv::Scalar{100, 100, 100},
//                  -1,
//                  cv::LINE_8, 0);
//
//    //Draw outlines
////    drawOutlineAndHead(<#initializer#>, <#initializer#>);
//}
