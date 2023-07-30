#include "MultiDMX_STM32.h"

#include "STM32TimerInterrupt.h"

static volatile uint8_t dmxBuffer[DMX_UNIVERSES][DMX_SIZE];
static uint8_t dmxStarted = 0;

static uint32_t begin, target;

static uint8_t dmxBit[DMX_UNIVERSES] = {0};
static PinName dmxPinNames[DMX_UNIVERSES] = {PA_0, PA_1};

#define get_GPIO_Port(p) ((p < MAX_NB_PORT) ? GPIOPort[p] : (GPIO_TypeDef *)NULL)
#define STM_PORT(X) (((uint32_t)(X) >> 4) & 0xF)
#define STM_PIN(X) ((uint32_t)(X)&0xF)
static GPIO_TypeDef *dmxPorts[DMX_UNIVERSES];
static uint32_t dmxPins[DMX_UNIVERSES];

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

  // Set DMX pin to output
  for (uint8_t i = 0; i < DMX_UNIVERSES; i++) {
    pinMode(pinNametoDigitalPin(dmxPinNames[i]), OUTPUT);
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

void setAllPins() {
  for (uint8_t i = 0; i < DMX_UNIVERSES; i++) {
    LL_GPIO_SetOutputPin(dmxPorts[i], dmxPins[i]);
  }
}
void resetAllPins() {
  for (uint8_t i = 0; i < DMX_UNIVERSES; i++) {
    LL_GPIO_ResetOutputPin(dmxPorts[i], dmxPins[i]);
  }
}

#define cyclesPerUs(us) ((F_CPU / 1000000) * us)
void waitTilTargetCycle() {
  while (DWT->CYCCNT - begin < target)
    ;
}


void TimerHandler() {
  TIMER_INTERRUPT_DISABLE();

  begin = DWT->CYCCNT;
  target = 0;
  target += cyclesPerUs(9);
  setAllPins();  // MAB
  waitTilTargetCycle();

  target += cyclesPerUs(4);
  resetAllPins();  // Start Bit
  waitTilTargetCycle();

  // Start Byte 0
  target += cyclesPerUs(4 * 8);
  resetAllPins();
  waitTilTargetCycle();

  target += cyclesPerUs(8);
  setAllPins();  // Stop Bits
  waitTilTargetCycle();

  resetAllPins();
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
  for (uint8_t u = 0; u < DMX_UNIVERSES; u++) {
    dmxPins[u] = pins[u];

    dmxPorts[u] = get_GPIO_Port(STM_PORT(pins[u]));
    dmxPins[u] = STM_LL_GPIO_PIN(pins[u]);
  };
  if (restartRequired) dmxBegin();
}

MultiDmxClass MultiDmx;
