#ifndef HELIO_CONTROL_DUINO_HOST_ARDUINO_H
#define HELIO_CONTROL_DUINO_HOST_ARDUINO_H

#include <stdint.h>

class __FlashStringHelper;

#define F(value) \
  (reinterpret_cast<const __FlashStringHelper *>(value))

extern uint32_t helioTestMillis;

inline uint32_t millis() { return helioTestMillis; }

#endif
