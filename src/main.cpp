#include <Arduino.h>
#include "machine/TapMachine.h"

TapMachine tapMachine;

void setup()
{
  Serial.begin(9600);
  tapMachine.begin();
  tapMachine.trigger(tapMachine.EVT_NEXT);
}

void loop()
{
  tapMachine.cycle();
}
