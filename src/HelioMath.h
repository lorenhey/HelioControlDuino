#ifndef HELIO_CONTROL_DUINO_MATH_H
#define HELIO_CONTROL_DUINO_MATH_H

#include <Arduino.h>

namespace heliocontrol {

inline float clampAngle(float value, float minimum, float maximum) {
  if (value < minimum) {
    return minimum;
  }
  if (value > maximum) {
    return maximum;
  }
  return value;
}

inline bool elapsed(uint32_t now, uint32_t since, uint32_t duration) {
  return static_cast<uint32_t>(now - since) >= duration;
}

inline int16_t normalizedDifference(int32_t positive, int32_t negative,
                                    uint32_t total) {
  if (total == 0) {
    return 0;
  }

  int32_t result = ((positive - negative) * 100L) /
                   static_cast<int32_t>(total);
  if (result > 100) {
    result = 100;
  } else if (result < -100) {
    result = -100;
  }
  return static_cast<int16_t>(result);
}

inline bool nearAngle(float first, float second, float tolerance = 0.75f) {
  const float difference = first > second ? first - second : second - first;
  return difference <= tolerance;
}

}  // namespace heliocontrol

#endif
