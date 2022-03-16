#include "Variables.h"
#include "DtmfGenerator.h"
#include "DialedDigit.h"

#include <math.h>
#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>

static DtmfGenerator* DtmfGenerator::instance = nullptr;

/**
 * Sinwave lookup table. Use a lookup table to get sinwave values
 * instead of computing it.
 * The table was generated with /tools/sinwave_lookup_table_generator.c .
 *
 * The values will be used to produce the duty cycles of the output PWM.
 */
static const unsigned int DtmfGenerator::sinwaveLut[PERIOD_SAMPLES_COUNT] = {
    127, 130, 133, 136, 139, 143, 146, 149,
    152, 155, 158, 161, 164, 167, 170, 173,
    176, 178, 181, 184, 187, 190, 192, 195,
    198, 200, 203, 205, 208, 210, 212, 215,
    217, 219, 221, 223, 225, 227, 229, 231,
    233, 234, 236, 238, 239, 240, 242, 243,
    244, 245, 247, 248, 249, 249, 250, 251,
    252, 252, 253, 253, 253, 254, 254, 254,
    254, 254, 254, 254, 253, 253, 253, 252,
    252, 251, 250, 249, 249, 248, 247, 245,
    244, 243, 242, 240, 239, 238, 236, 234,
    233, 231, 229, 227, 225, 223, 221, 219,
    217, 215, 212, 210, 208, 205, 203, 200,
    198, 195, 192, 190, 187, 184, 181, 178,
    176, 173, 170, 167, 164, 161, 158, 155,
    152, 149, 146, 143, 139, 136, 133, 130,
    127, 124, 121, 118, 115, 111, 108, 105,
    102,  99,  96,  93,  90,  87,  84,  81,
     78,  76,  73,  70,  67,  64,  62,  59,
     56,  54,  51,  49,  46,  44,  42,  39,
     37,  35,  33,  31,  29,  27,  25,  23,
     21,  20,  18,  16,  15,  14,  12,  11,
     10,   9,   7,   6,   5,   5,   4,   3,
      2,   2,   1,   1,   1,   0,   0,   0,
      0,   0,   0,   0,   1,   1,   1,   2,
      2,   3,   4,   5,   5,   6,   7,   9,
     10,  11,  12,  14,  15,  16,  18,  20,
     21,  23,  25,  27,  29,  31,  33,  35,
     37,  39,  42,  44,  46,  49,  51,  54,
     56,  59,  62,  64,  67,  70,  73,  76,
     78,  81,  84,  87,  90,  93,  96,  99,
    102, 105, 108, 111, 115, 118, 121, 124
};

static unsigned int DtmfGenerator::computeToneStepSize(unsigned int tone)
{
    // use a float to handle large numbers in the formula
    float fTone = tone * 1.0;

    return (unsigned int) round((PERIOD_SAMPLES_COUNT * fTone) / PERIOD_FREQUENCY);
}

/*
@see https://en.wikipedia.org/wiki/Dual-tone_multi-frequency_signaling

Columns : high frequencies.
Rows : low frequencies.

         1209 Hz     1336 Hz     1477 Hz     1633 Hz

697 Hz     1           2           3           A

770 Hz     4           5           6           B

852 Hz     7           8           9           C

941 Hz     *           0           #           D

The map represents the step size on the above sinwaveLut to make on each interrupt
period.
The step size depends on the tone and the interrupt frequency.
*/
static const unsigned int DtmfGenerator::digitToTonesStepSize[12][2] = {
    { computeToneStepSize(1336), computeToneStepSize(941) }, // 0
    { computeToneStepSize(1209), computeToneStepSize(697) }, // 1
    { computeToneStepSize(1336), computeToneStepSize(697) }, // 2
    { computeToneStepSize(1477), computeToneStepSize(697) }, // 3
    { computeToneStepSize(1209), computeToneStepSize(770) }, // 4
    { computeToneStepSize(1336), computeToneStepSize(770) }, // 5
    { computeToneStepSize(1477), computeToneStepSize(770) }, // 6
    { computeToneStepSize(1209), computeToneStepSize(852) }, // 7
    { computeToneStepSize(1336), computeToneStepSize(852) }, // 8
    { computeToneStepSize(1477), computeToneStepSize(852) }, // 9
    { computeToneStepSize(1209), computeToneStepSize(941) }, // *
    { computeToneStepSize(1477), computeToneStepSize(941) }  // #
};

static DtmfGenerator* DtmfGenerator::build(
    DialedDigit* dialedDigit,
    unsigned int dtmfDurationMs
)
{
    if (nullptr == instance) {
        instance = new DtmfGenerator(dialedDigit, dtmfDurationMs);
    }

    return instance;
}

static DtmfGenerator* DtmfGenerator::getInstance()
{
    return instance;
}

DtmfGenerator::DtmfGenerator(
    DialedDigit* dialedDigit,
    unsigned int dtmfDurationMs
):  dialedDigit(dialedDigit),
    dtmfDurationMs(dtmfDurationMs),
    remainingGenerationCycles(-1),
    toneHighStepSize(0),
    toneLowStepSize(0),
    toneHighPos(0),
    toneLowPos(0)
{
    instance = this;
}

void DtmfGenerator::setup()
{
    // enable output pin (pin D3 aka PF5, see datasheet p146).
    PORTMUX.TCBROUTEA |= PORTMUX_TCB1_bm;

    // Configure timer TCB1 of the chip for PWM.
    // See Chapter 21 of ATmega4809 datasheet.

    // schedule counter speed at µC speed / 1 (i.e. same as XTAL)
    TCB1.CTRLA |= TCB_CLKSEL_CLKDIV1_gc;
    // quiet the output (by setting the compare value to 0)
    this->quiet();
    // interrupts should be captured (i.e. enable callback)
    TCB1.INTCTRL |= TCB_CAPT_bm;
    // set timer mode to Pulse Width Modulation (8bits)
    TCB1.CTRLB |= TCB_CNTMODE_PWM8_gc;
    // enable waveform output on the corresponding pin
    TCB1.CTRLB |= TCB_CCMPEN_bm;
    // do not capture input events
    TCB1.EVCTRL &= ~TCB_CAPTEI_bm;
    // enable the counter
    TCB1.CTRLA |= TCB_ENABLE_bm;

    // Clear the interrupt flag which may have been set while configuring the
    // timer (as the datasheet recommands).
    TCB1.INTFLAGS |= TCB_CAPT_bm;
}

// ISR triggered at PERIOD_FREQUENCY.
// The ISR callback is a static method.
ISR(TCB1_INT_vect)
{
    DtmfGenerator* dtmfGenerator = DtmfGenerator::getInstance();

    if (nullptr != dtmfGenerator) {
        dtmfGenerator->handleIsr();
    }

    // Clear the interrupt flag (i.e. indicates that the interrupt has been
    // handled. This is not done automatically).
    TCB1.INTFLAGS |= TCB_CAPT_bm;
}

void DtmfGenerator::handleIsr()
{
    if (0 == this->remainingGenerationCycles) {
        Serial.println("quieting");
        this->quiet();

        this->remainingGenerationCycles = -1;

        return;
    }

    if (this->remainingGenerationCycles > 0) {
        this->generateDtmf();

        --this->remainingGenerationCycles;

        return;
    }

    if (this->dialedDigit->isNew()) {
        this->scheduleDtmfGeneration();
    }
}

void DtmfGenerator::quiet()
{
    this->setDutyCycle(0);
}

void DtmfGenerator::scheduleDtmfGeneration()
{
    unsigned int dialedDigit = this->dialedDigit->flush();

    Serial.println((String) "Scheduling DTMF for digit " + dialedDigit);

    this->toneHighStepSize = digitToTonesStepSize[dialedDigit][0];
    this->toneLowStepSize = digitToTonesStepSize[dialedDigit][1];

    this->toneHighPos = 0;
    this->toneLowPos = 0;

    this->remainingGenerationCycles = (int) ((DTMF_DURATION_MS * PERIOD_FREQUENCY) / 1000);
}

void DtmfGenerator::generateDtmf()
{
    unsigned int toneHighWave = sinwaveLut[this->toneHighPos];
    unsigned int toneLowWave = sinwaveLut[this->toneLowPos];

    unsigned int sumWave = (unsigned int) ((toneHighWave + toneLowWave) / 2);

    this->setDutyCycle(sumWave);

    this->toneHighPos += this->toneHighStepSize;

    // wrap around
    if (this->toneHighPos > PERIOD_SAMPLES_COUNT -1) {
        this->toneHighPos -= PERIOD_SAMPLES_COUNT;
    }

    this->toneLowPos += this->toneLowStepSize;

    // wrap around
    if (this->toneLowPos > PERIOD_SAMPLES_COUNT -1) {
        this->toneLowPos -= PERIOD_SAMPLES_COUNT;
    }
}

/**
 * Set TCB1 counter compare value to have a `pulsesCount` duty cycle (CCMPH),
 * and to have the 8bit pulse period (CCMPL).
 */
void DtmfGenerator::setDutyCycle(unsigned int pulsesCount)
{
    pulsesCount << 8;

    TCB1.CCMP = pulsesCount | TCB1_MAX_VALUE;
}
