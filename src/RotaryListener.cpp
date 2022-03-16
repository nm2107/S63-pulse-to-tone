#include "Variables.h"
#include "RotaryListener.h"
#include "DialedDigit.h"

#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>

static const unsigned int RotaryListener::rotaryDigits[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };

static RotaryListener* RotaryListener::instance = nullptr;

static RotaryListener* RotaryListener::build(
    DialedDigit* dialedDigit,
    unsigned int pinPollDelayMs
)
{
    if (nullptr == instance) {
        instance = new RotaryListener(dialedDigit, pinPollDelayMs);
    }

    return instance;
}

static RotaryListener* RotaryListener::getInstance()
{
    return instance;
}

RotaryListener::RotaryListener(
    DialedDigit* dialedDigit,
    unsigned int pinPollDelayMs
):  dialedDigit(dialedDigit),
    pinPollDelayMs(pinPollDelayMs),
    pulsesCount(0),
    rotaryMovePinStatus(HIGH),
    previousPulsePinStatus(LOW),
    pulsePinStatus(LOW)
{
}

void RotaryListener::setup()
{
    // enable input pins
    pinMode(ROTARY_MOVE_PIN, INPUT_PULLUP);
    pinMode(PULSE_PIN, INPUT_PULLUP);

    // Configure timer TCB2 of the chip to trigger an ISR every
    // PIN_POLL_DELAY_MS ms.
    // See Chapter 21 of ATmega4809 datasheet.

    // schedule counter speed at µC speed / 1 (i.e. same as XTAL)
    TCB2.CTRLA |= TCB_CLKSEL_CLKDIV1_gc;
    // set counter compare value to have interrupts captured each
    // PIN_POLL_DELAY_MS ms
    TCB2.CCMP = this->getTcb2CompareValue();
    // interrupts should be captured (i.e. enable callback)
    TCB2.INTCTRL |= TCB_CAPT_bm;
    // set timer mode to interrupt (i.e. fire and event each time it has reached
    // its CCMP value)
    TCB2.CTRLB |= TCB_CNTMODE_INT_gc;
    // do not capture input events
    TCB2.EVCTRL &= ~TCB_CAPTEI_bm;
    // enable the counter
    TCB2.CTRLA |= TCB_ENABLE_bm;

    // Clear the interrupt flag which may have been set while configuring the
    // timer (as the datasheet recommands).
    TCB2.INTFLAGS |= TCB_CAPT_bm;
}

/**
  * @return unsigned int The compare value of the TCB2 counter to use for having
  * a period duration of PIN_POLL_DELAY_MS when the counter counts at
  * XTAL Hz rate.
  */
unsigned int RotaryListener::getTcb2CompareValue()
{
    // convert to floats
    const float _xtal = XTAL * 1.0;
    const float _max_val = TCB2_MAX_VALUE * 1.0;
    const float _delay = this->pinPollDelayMs * 1.0;

    // result of
    // TCB2_MAX_VALUE * (PIN_POLL_DELAY_MS / (XTAL / TCB2_MAX_VALUE)) - 1
    // -1 as the timer starts to count from 0
    return (unsigned int) (_max_val * (_delay / (_xtal / _max_val))) -1;
}

// ISR triggered each pinPollDelayMs ms.
// The ISR callback is a static method.
ISR(TCB2_INT_vect)
{
    RotaryListener* rotaryListener = RotaryListener::getInstance();

    if (nullptr != rotaryListener) {
        rotaryListener->handleIsr();
    }

    // Clear the interrupt flag (i.e. indicates that the interrupt has been
    // handled. This is not done automatically).
    TCB2.INTFLAGS |= TCB_CAPT_bm;
}

void RotaryListener::handleIsr()
{
    this->pollPins();
    this->handlePinsStatuses();
}

void RotaryListener::pollPins()
{
    this->rotaryMovePinStatus = digitalRead(ROTARY_MOVE_PIN);
    this->previousPulsePinStatus = this->pulsePinStatus;
    this->pulsePinStatus = digitalRead(PULSE_PIN);
}

void RotaryListener::handlePinsStatuses()
{
    if (!this->isRotaryMoving()) {
        this->flushPulses();

        return;
    }

    if (this->isRotaryMoving() && this->hasPulseStarted()) {
        this->addPulse();
    }
}

void RotaryListener::addPulse()
{
    ++this->pulsesCount;
}

void RotaryListener::flushPulses()
{
    if (0 == this->pulsesCount) {
        // nothing to do when no pulses
        return;
    }

    // Do not continue when we have collected too much pulses.
    // It may happen in case of bad electric installation, or wrong poll delay,
    // or wrong rotary pulse duration (supposed to be 66ms, with a pause of
    // 33ms between two pulses), or the kids are simply spinning the rotary for
    // a long time :p
    if (this->pulsesCount > sizeof(rotaryDigits)) {
        this->pulsesCount = 0;

        return;
    }

    unsigned int dialedDigit = rotaryDigits[this->pulsesCount -1];
    this->pulsesCount = 0;

    Serial.println((String)"Dialed digit " + dialedDigit);

    this->dialedDigit->push(dialedDigit);
}

bool RotaryListener::isRotaryMoving() const
{
    return LOW == this->rotaryMovePinStatus;
}

bool RotaryListener::hasPulseStarted() const
{
    return this->previousPulsePinStatus != this->pulsePinStatus
        && HIGH == this->pulsePinStatus
    ;
}
