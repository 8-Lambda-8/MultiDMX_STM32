#include "MultiDMX_STM32.h"

#include "STM32TimerInterrupt.h"

static volatile uint8_t dmxBuffer[DMX_UNIVERSES][DMX_SIZE];
static uint8_t dmxStarted = 0;
static uint16_t dmxState = 0;

static uint8_t dmxBit[DMX_UNIVERSES] = {0};
static PinName dmxPins[DMX_UNIVERSES] = {PA_0, PA_1};

void dmxBegin();
void dmxEnd();
void dmxSendByte(volatile uint8_t);
void dmxWrite(int, uint8_t);

STM32Timer ITimer(TIM2);

void TimerHandler();

void TIMER_INTERRUPT_ENABLE() {
  ITimer.attachInterruptInterval(90, TimerHandler)
}

void TIMER_INTERRUPT_DISABLE() {
  ITimer.detachInterrupt();
}

/** Initialise the DMX engine
 */
void dmxBegin() {
  dmxStarted = 1;
  for (size_t i = 0; i < DMX_UNIVERSES; i++) {
    // Set DMX pin to output
    for (uint8_t i = 0; i < DMX_UNIVERSES; i++) {
      pinMode(dmxPins[i], OUTPUT);
    }
  }

  TIMER_INTERRUPT_ENABLE();
}

/** Stop the DMX engine
 * Turns off the DMX interrupt routine
 */
void dmxEnd() {
  dmxStarted = 0;
  TIMER_INTERRUPT_DISABLE();
}
void TimerHandler() {
  TIMER_INTERRUPT_DISABLE();

  TIMER_INTERRUPT_ENABLE();
}

/** CLASS Functions */

/** Write to a DMX channel
 * @param universe output Universe
 * @param address DMX channel in the range 1 - 512
 * @param value DMX channel value 0 - 255
 */
void MultiDmxClass::write(uint8_t universe, uint16_t address, uint8_t value) {
  if (universe >= DMX_UNIVERSES || address > DMX_SIZE || address == 0) return;
  if (!dmxStarted) dmxBegin();
  dmxBuffer[universe][address - 1] = value;
}

/** Set output pins
 * @param pins Array of digital pins to use
 */
void MultiDmxClass::usePins(PinName pins[DMX_UNIVERSES]) {
  bool restartRequired = dmxStarted;

  if (restartRequired) dmxEnd();
  for (uint8_t u = 0; u < DMX_UNIVERSES; u++) dmxPins[u] = pins[u];
  if (restartRequired) dmxBegin();
}

MultiDmxClass MultiDmx;
