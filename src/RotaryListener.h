#ifndef S63_ROTARYLISTENER_H
#define S63_ROTARYLISTENER_H

#define ROTARY_MOVE_PIN 2
#define PULSE_PIN 4
// 0xFFFF, TCB2 is a 16bit counter.
#define TCB2_MAX_VALUE 65535

#include "DialedDigit.h"

class RotaryListener
{
    public:
        static RotaryListener* build(
            DialedDigit* dialedDigit,
            unsigned int pinPollDelayMs
        );
        static RotaryListener* getInstance();
        void setup();
        void handleIsr();

    private:
        RotaryListener(){};
        RotaryListener(
            DialedDigit* dialedDigit,
            unsigned int pinPollDelayMs
        );

        static RotaryListener* instance;
        static const unsigned int rotaryDigits[10];

        DialedDigit* dialedDigit;
        unsigned int pinPollDelayMs;
        unsigned int pulsesCount;
        unsigned char rotaryMovePinStatus;
        unsigned char previousPulsePinStatus;
        unsigned char pulsePinStatus;

        unsigned int getTcb2CompareValue();
        void pollPins();
        void handlePinsStatuses();
        void addPulse();
        void flushPulses();
        bool isRotaryMoving() const;
        bool hasPulseStarted() const;
};

#endif
