#ifndef MultiDmx_h
#define MultiDmx_h

#include "Arduino.h"

#define DMX_SIZE 512

#ifndef DMX_UNIVERSES
#define DMX_UNIVERSES 2
#endif

class MultiDmxClass {
 public:
  void write(uint8_t universe, uint16_t channel, uint8_t value);
  void usePins(PinName pins[DMX_UNIVERSES]);
};
extern MultiDmxClass MultiDmx;

#endif