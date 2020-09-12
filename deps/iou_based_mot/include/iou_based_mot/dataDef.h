////
//// Created by kai01.han on 6/4/19.
////
//
//#ifndef IOU_BASED_MOT_DATADEF_H
//#define IOU_BASED_MOT_DATADEF_H
//
//#include <iostream>
//
//namespace hobot {
//  namespace iou_mot {
//
//    struct Point_f {
//      double x;
//      double y;
//    };
//
//    class BoundingBox {
//    public:
//      BoundingBox() = default;
//
//      BoundingBox(Point_f a, Point_f b) {
//        leftTopPoint = a;
//        rightBottomPoint = b;
//      }
//
//      BoundingBox(double x1, double y1, double x2, double y2) {
//        leftTopPoint.x = x1;
//        leftTopPoint.y = y1;
//        rightBottomPoint.x = x2;
//        rightBottomPoint.y = y2;
//      }
//
//      inline void output() {
//        std::cout << this->leftTopPoint.x << " " << this->leftTopPoint.y << " "
//                  << this->rightBottomPoint.x << " " << this->rightBottomPoint.y
//                  << std::endl;
//
//      }
//
//      Point_f leftTopPoint;
//      Point_f rightBottomPoint;
//    };
//  }
//}
//#endif //IOU_BASED_MOT_DATADEF_H