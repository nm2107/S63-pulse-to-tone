#ifndef S63_DTMFGENERATOR_H
#define S63_DTMFGENERATOR_H

#include "Variables.h"
#include "DialedDigit.h"

// 0xFF, TCB1 in PWM mode is a 8bit counter, starts to count from 0
#define TCB1_MAX_VALUE 255
// how many samples per period (i.e. clock cycles per interrupt)
#define PERIOD_SAMPLES_COUNT ( TCB1_MAX_VALUE + 1 )
#define PERIOD_FREQUENCY ( XTAL / PERIOD_SAMPLES_COUNT )

class DtmfGenerator
{
    public:
        static DtmfGenerator* build(
            DialedDigit* dialedDigit,
            unsigned int dtmfDurationMs
        );
        static DtmfGenerator* getInstance();
        void setup();
        void handleIsr();

    private:
        DtmfGenerator(){};
        DtmfGenerator(
            DialedDigit* dialedDigit,
            unsigned int dtmfDurationMs
        );

        static DtmfGenerator* instance;
        static const unsigned int sinwaveLut[PERIOD_SAMPLES_COUNT];
        static const unsigned int digitToTonesStepSize[12][2];
        static unsigned int computeToneStepSize(unsigned int tone);

        DialedDigit* dialedDigit;
        unsigned int dtmfDurationMs;
        int remainingGenerationCycles;
        unsigned int toneHighStepSize;
        unsigned int toneLowStepSize;
        unsigned int toneHighPos;
        unsigned int toneLowPos;

        void quiet();
        void scheduleDtmfGeneration();
        void generateDtmf();
        void setDutyCycle(unsigned int pulsesCount);
};

#endif
