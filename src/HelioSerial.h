#ifndef HELIO_CONTROL_DUINO_SERIAL_H
#define HELIO_CONTROL_DUINO_SERIAL_H

#include <Arduino.h>

#include "HelioController.h"

namespace heliocontrol {

class SerialConsole {
 public:
  SerialConsole(Stream &stream, HelioController &controller);

  void begin(bool showBanner = true);
  void poll();
  void printStatus();
  void printHelp();

 private:
  void execute(char *line);
  bool parseFloatToken(char *token, float &value) const;
  void replyOk(const __FlashStringHelper *message);
  void replyError(const __FlashStringHelper *message);

  Stream &stream_;
  HelioController &controller_;
  char buffer_[64];
  uint8_t length_;
  bool overflow_;
};

}  // namespace heliocontrol

#endif
