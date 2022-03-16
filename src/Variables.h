#ifndef S63_VARIABLES_H
#define S63_VARIABLES_H

// µC freq. configured by default to 16MHz by arduino-cli's boards.txt.
#define XTAL 16000000
// You may want to lower this value if you have difficulties to determine the
// rotary pulses count. Do not exceed 25.
#define PIN_POLL_DELAY_MS 20
// how long the tone should be played, in ms
#define DTMF_DURATION_MS 300

#endif
