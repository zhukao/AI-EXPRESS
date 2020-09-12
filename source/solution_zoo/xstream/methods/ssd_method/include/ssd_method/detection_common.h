//
// Copyright (c) 2019 Horizon Robotics. All rights reserved.
//

#ifndef INCLUDE_SSDMETHOD_DETECTION_COMMON_H_
#define INCLUDE_SSDMETHOD_DETECTION_COMMON_H_

#include <sstream>
#include <string>
namespace xstream {
typedef struct Anchor {
  float cx;
  float cy;
  float w;
  float h;
  Anchor(float cx, float cy, float w, float h) : cx(cx), cy(cy), w(w), h(h) {}
  std::string toString() {
    std::stringstream ss;
    ss << "[" << cx << "," << cy << "," << w << "," << h << "]";
    return ss.str();
  }
} Anchor;

typedef struct Bbox {
  float xmin;
  float ymin;
  float xmax;
  float ymax;

  Bbox() {}

  Bbox(float xmin, float ymin, float xmax, float ymax)
      : xmin(xmin), ymin(ymin), xmax(xmax), ymax(ymax) {}

  std::string toString() {
    std::stringstream ss;
    ss << "[" << xmin << "," << ymin << "," << xmax << "," << ymax << "]";
    return ss.str();
  }
} Bbox;

typedef struct Detection {
  int id;
  float score;
  Bbox bbox;

  Detection() {}

  Detection(int id, float score, Bbox bbox)
      : id(id), score(score), bbox(bbox) {}

  static bool greater(Detection det1, Detection det2) {
    return (det1.score > det2.score);
  }

  std::string toString() {
    std::stringstream ss;
    ss << "{id: " << id << ", score: " << score << ", box: " << bbox.toString()
       << "}";
    return ss.str();
  }
} Detection;
}  // namespace xstream
#endif  // end of INCLUDE_SSDMETHOD_DETECTION_COMMON_H_
