#include "HelioSerial.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

namespace heliocontrol {

SerialConsole::SerialConsole(Stream &stream, HelioController &controller)
    : stream_(stream), controller_(controller), length_(0), overflow_(false) {
  buffer_[0] = '\0';
}

void SerialConsole::begin(bool showBanner) {
  length_ = 0;
  overflow_ = false;
  if (showBanner) {
    stream_.print(F("HelioControlDuino "));
    stream_.println(VERSION);
    stream_.println(F("Escriba HELP para ver los comandos."));
  }
}

void SerialConsole::poll() {
  while (stream_.available() > 0) {
    const char incoming = static_cast<char>(stream_.read());

    if (incoming == '\r' || incoming == '\n') {
      if (overflow_) {
        replyError(F("comando demasiado largo"));
      } else if (length_ > 0) {
        buffer_[length_] = '\0';
        execute(buffer_);
      }
      length_ = 0;
      overflow_ = false;
      continue;
    }

    if (incoming < 32 || incoming > 126) {
      continue;
    }

    if (length_ < sizeof(buffer_) - 1) {
      buffer_[length_++] = incoming;
    } else {
      overflow_ = true;
    }
  }
}

void SerialConsole::execute(char *line) {
  for (char *cursor = line; *cursor != '\0'; ++cursor) {
    *cursor = static_cast<char>(toupper(static_cast<unsigned char>(*cursor)));
  }

  char *command = strtok(line, " \t");
  if (command == nullptr) {
    return;
  }

  if (strcmp(command, "HELP") == 0 || strcmp(command, "?") == 0) {
    printHelp();
  } else if (strcmp(command, "STATUS") == 0) {
    printStatus();
  } else if (strcmp(command, "AUTO") == 0) {
    if (controller_.status().faults != FAULT_NONE) {
      replyError(F("hay una falla activa; use STATUS y RESET"));
    } else {
      controller_.setMode(OperatingMode::AUTOMATIC);
      replyOk(F("modo automatico"));
    }
  } else if (strcmp(command, "PARK") == 0) {
    if (controller_.status().faults != FAULT_NONE) {
      replyError(F("hay una falla activa; use STATUS y RESET"));
    } else {
      controller_.setMode(OperatingMode::PARK);
      replyOk(F("estacionando"));
    }
  } else if (strcmp(command, "STOW") == 0) {
    if (controller_.status().faults != FAULT_NONE) {
      replyError(F("hay una falla activa; use STATUS y RESET"));
    } else {
      controller_.setMode(OperatingMode::STOW);
      replyOk(F("posicion de resguardo"));
    }
  } else if (strcmp(command, "STOP") == 0) {
    controller_.setMode(OperatingMode::STOPPED);
    replyOk(F("movimiento detenido"));
  } else if (strcmp(command, "RESET") == 0) {
    if (controller_.clearFaults()) {
      replyOk(F("fallas borradas; controlador detenido"));
    } else {
      replyError(F("la condicion insegura continua activa"));
    }
  } else if (strcmp(command, "VERSION") == 0) {
    stream_.println(VERSION);
  } else if (strcmp(command, "MOVE") == 0) {
    float azimuth = 0.0f;
    float elevation = 0.0f;
    char *azimuthToken = strtok(nullptr, " \t");
    char *elevationToken = strtok(nullptr, " \t");
    char *extraToken = strtok(nullptr, " \t");
    if (!parseFloatToken(azimuthToken, azimuth) ||
        !parseFloatToken(elevationToken, elevation) ||
        extraToken != nullptr) {
      replyError(F("uso: MOVE <azimut> <elevacion>"));
    } else if (!controller_.setManualTarget(azimuth, elevation)) {
      replyError(F("objetivo fuera de limites o condicion insegura"));
    } else {
      replyOk(F("objetivo manual aceptado"));
    }
  } else {
    replyError(F("comando desconocido; use HELP"));
  }
}

bool SerialConsole::parseFloatToken(char *token, float &value) const {
  if (token == nullptr || *token == '\0') {
    return false;
  }
  char *end = nullptr;
  value = static_cast<float>(strtod(token, &end));
  return end != token && *end == '\0';
}

void SerialConsole::printStatus() {
  const ControllerStatus &status = controller_.status();
  stream_.print(F("MODE="));
  stream_.print(modeName(status.mode));
  stream_.print(F(" STATE="));
  stream_.print(stateName(status.state));
  stream_.print(F(" FAULTS=0x"));
  stream_.print(status.faults, HEX);
  stream_.print(F(" AZ="));
  stream_.print(status.azimuthPosition, 1);
  stream_.print(F("/"));
  stream_.print(status.azimuthTarget, 1);
  stream_.print(F(" EL="));
  stream_.print(status.elevationPosition, 1);
  stream_.print(F("/"));
  stream_.print(status.elevationTarget, 1);
  stream_.print(F(" LIGHT="));
  stream_.print(status.light.average);
  stream_.print(F(" ERR="));
  stream_.print(status.azimuthErrorPercent);
  stream_.print(F(","));
  stream_.print(status.elevationErrorPercent);
  stream_.print(F(" WIND="));
  stream_.print(status.safety.wind ? 1 : 0);
  stream_.print(F(" ESTOP="));
  stream_.println(status.safety.emergencyStop ? 1 : 0);
}

void SerialConsole::printHelp() {
  stream_.println(F("HELP              lista de comandos"));
  stream_.println(F("STATUS            estado y lecturas"));
  stream_.println(F("AUTO              seguimiento automatico"));
  stream_.println(F("PARK              posicion de estacionamiento"));
  stream_.println(F("STOW              posicion de resguardo"));
  stream_.println(F("STOP              detener movimiento"));
  stream_.println(F("MOVE <az> <el>    objetivo manual en grados"));
  stream_.println(F("RESET             borrar fallas si es seguro"));
  stream_.println(F("VERSION           version del firmware"));
}

void SerialConsole::replyOk(const __FlashStringHelper *message) {
  stream_.print(F("OK "));
  stream_.println(message);
}

void SerialConsole::replyError(const __FlashStringHelper *message) {
  stream_.print(F("ERROR "));
  stream_.println(message);
}

}  // namespace heliocontrol
