#include "SolarCalculator.h"

#include <math.h>

namespace heliocontrol {
namespace {

const float PI_F = 3.14159265358979323846f;

float toRadians(float degreeValue) { return degreeValue * PI_F / 180.0f; }
float toDegrees(float radiansValue) {
  return radiansValue * 180.0f / PI_F;
}

float clampUnit(float value) {
  if (value < -1.0f) {
    return -1.0f;
  }
  if (value > 1.0f) {
    return 1.0f;
  }
  return value;
}

}  // namespace

bool SolarCalculator::leapYear(uint16_t year) {
  return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

bool SolarCalculator::validDateTime(const LocalDateTime &dateTime) {
  if (dateTime.year < 2000 || dateTime.month < 1 || dateTime.month > 12 ||
      dateTime.hour > 23 || dateTime.minute > 59 ||
      dateTime.second > 59) {
    return false;
  }

  static const uint8_t daysPerMonth[] = {31, 28, 31, 30, 31, 30,
                                          31, 31, 30, 31, 30, 31};
  uint8_t maximumDay = daysPerMonth[dateTime.month - 1];
  if (dateTime.month == 2 && leapYear(dateTime.year)) {
    maximumDay = 29;
  }
  return dateTime.day >= 1 && dateTime.day <= maximumDay;
}

uint16_t SolarCalculator::dayOfYear(const LocalDateTime &dateTime) {
  static const uint16_t daysBeforeMonth[] = {0,   31,  59,  90,  120, 151,
                                              181, 212, 243, 273, 304, 334};
  uint16_t result = daysBeforeMonth[dateTime.month - 1] + dateTime.day;
  if (dateTime.month > 2 && leapYear(dateTime.year)) {
    ++result;
  }
  return result;
}

SolarCoordinates SolarCalculator::calculate(const LocalDateTime &localTime,
                                             const GeoLocation &location) {
  SolarCoordinates result;
  result.azimuthDegrees = 0.0f;
  result.elevationDegrees = -90.0f;
  result.aboveHorizon = false;
  result.valid = false;

  if (!validDateTime(localTime) || location.latitudeDegrees < -90.0f ||
      location.latitudeDegrees > 90.0f ||
      location.longitudeDegrees < -180.0f ||
      location.longitudeDegrees > 180.0f ||
      location.utcOffsetMinutes < -840 ||
      location.utcOffsetMinutes > 840) {
    return result;
  }

  const uint16_t yearDays = leapYear(localTime.year) ? 366 : 365;
  const float fractionalHour =
      localTime.hour + localTime.minute / 60.0f +
      localTime.second / 3600.0f;
  const float gamma =
      2.0f * PI_F / yearDays *
      (dayOfYear(localTime) - 1 + (fractionalHour - 12.0f) / 24.0f);

  const float equationOfTime =
      229.18f *
      (0.000075f + 0.001868f * cos(gamma) -
       0.032077f * sin(gamma) - 0.014615f * cos(2.0f * gamma) -
       0.040849f * sin(2.0f * gamma));

  const float declination =
      0.006918f - 0.399912f * cos(gamma) +
      0.070257f * sin(gamma) - 0.006758f * cos(2.0f * gamma) +
      0.000907f * sin(2.0f * gamma) -
      0.002697f * cos(3.0f * gamma) +
      0.00148f * sin(3.0f * gamma);

  const float localMinutes =
      localTime.hour * 60.0f + localTime.minute + localTime.second / 60.0f;
  const float timeOffset = equationOfTime +
                           4.0f * location.longitudeDegrees -
                           location.utcOffsetMinutes;
  float trueSolarMinutes = fmod(localMinutes + timeOffset, 1440.0f);
  if (trueSolarMinutes < 0.0f) {
    trueSolarMinutes += 1440.0f;
  }

  float hourAngleDegrees = trueSolarMinutes / 4.0f - 180.0f;
  if (hourAngleDegrees < -180.0f) {
    hourAngleDegrees += 360.0f;
  }

  const float latitude = toRadians(location.latitudeDegrees);
  const float hourAngle = toRadians(hourAngleDegrees);
  const float cosineZenith =
      clampUnit(sin(latitude) * sin(declination) +
                cos(latitude) * cos(declination) * cos(hourAngle));
  const float zenith = acos(cosineZenith);
  result.elevationDegrees = 90.0f - toDegrees(zenith);

  const float azimuthRadians =
      atan2(sin(hourAngle),
            cos(hourAngle) * sin(latitude) -
                tan(declination) * cos(latitude));
  result.azimuthDegrees =
      fmod(toDegrees(azimuthRadians) + 180.0f, 360.0f);
  if (result.azimuthDegrees < 0.0f) {
    result.azimuthDegrees += 360.0f;
  }

  result.aboveHorizon = result.elevationDegrees > 0.0f;
  result.valid = true;
  return result;
}

}  // namespace heliocontrol
