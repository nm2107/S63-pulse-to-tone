#include "Variables.h"
#include "DialedDigit.h"
#include "RotaryListener.h"
#include "DtmfGenerator.h"

void setup() {
#ifdef ENABLE_LOGGING
    Serial.begin(9600);
#endif

    const DialedDigit* dialedDigit = new DialedDigit();
    const RotaryListener* rotaryListener = RotaryListener::build(dialedDigit, PIN_POLL_DELAY_MS);
    const DtmfGenerator* dtmfGenerator = DtmfGenerator::build(dialedDigit, DTMF_DURATION_MS);

    rotaryListener->setup();
    dtmfGenerator->setup();
}

void loop() {
    // Use a polling strategy instead of arduino's attachInterrupt as it has
    // produced imprecise results for our use case when determining the input
    // pins statuses.
    // The polling is done via the ATmega4809 chip timers, as we can't use any
    // `delay` calls here as it would pause the program (e.g. pause the DTMF
    // generation).

    while(1); // gets rid of jitter
}
