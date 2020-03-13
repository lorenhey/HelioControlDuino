#ifndef HELIO_CONTROL_DUINO_SOLAR_CALCULATOR_H
#define HELIO_CONTROL_DUINO_SOLAR_CALCULATOR_H

#include <Arduino.h>

namespace heliocontrol {

struct LocalDateTime {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};

struct GeoLocation {
  float latitudeDegrees;
  float longitudeDegrees;
  int16_t utcOffsetMinutes;
};

struct SolarCoordinates {
  float azimuthDegrees;
  float elevationDegrees;
  bool aboveHorizon;
  bool valid;
};

class SolarCalculator {
 public:
  static SolarCoordinates calculate(const LocalDateTime &localTime,
                                    const GeoLocation &location);
  static bool validDateTime(const LocalDateTime &dateTime);

 private:
  static bool leapYear(uint16_t year);
  static uint16_t dayOfYear(const LocalDateTime &dateTime);
};

}  // namespace heliocontrol

#endif
