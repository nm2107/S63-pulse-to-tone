# Socotel S63 pulse to tone converter

The aim of this project is to create a pulse to tone converter for the Socotel
S63 rotary phone, using an
[Arduino Nano Every](https://docs.arduino.cc/hardware/nano-every) board
(ATMEGA4809 chip, SKU ABX00028).

**:warning: This is very much a work in progress.
It is not ready yet for any usage. :warning:**

![Who's there ?](https://media4.giphy.com/media/XJzlSfuuX1qI8/giphy.gif)

## Why ?

The rotary phones are composing digits by sending pulses. The amount of pulses
represents the digit. However, this way to represent digits is not supported
anymore by phone services, it has been replaced by DTMF
(Dual Tone Multi Frequency) (the `beep` you hear when pressing a key). It is
now this tone which represents the dialed digit.

So this project aims to convert the pulses of the rotary S63 phone to DTMF, in
order to be able to use this phone again to make calls.

![Really ?](https://media2.giphy.com/media/khWKF3Hx7lDDa/giphy.gif)

## Installation

Prerequisites :

- A GNU/Linux host (other hosts are compatible too, but the documentation is
GNU/Linux oriented).
- An Arduino Nano Every board.
- Installed softwares :
    - docker
    - docker-compose
    - git
    - make

First, clone the repo and go inside it :

```bash
$ git clone git@github.com:nm2107/S63-pulse-to-tone.git

$ cd S63-pulse-to-tone
```

Plug the board into your host. The board path is generally `/dev/ttyACM0`.
If it's different, you can still specify it when running make commands by using
the `DEVICE` var :

```bash
$ DEVICE=<path> make <target>
```

Make sure you have read and write accesses to the board :

```bash
$ sudo chmod o+rw /dev/ttyACM0
```

Then, build the docker image and install required dependencies :

```bash
$ make build install-deps
```

Finally, compile the app and upload it to the board :

```bash
$ make compile-release upload
```

## Development

To open a shell inside the docker container, run :

```bash
$ make shell
```

### Dependencies

Any additional dependencies should be installed via updating the `install-deps`
make target to ensure reproducibility.

### Logging

To build the app with serial logging enabled, run :

```bash
$ make compile
```

and then, upload it to the board :

```bash
$ make upload
```

You can then see the logs with :

```bash
$ make monitor
```

## MVP Roadmap

- [x] Count pulses to determine the dialed digit.
- [ ] [WIP] Generate the DTMF tone corresponding to the dialed digit.
- [ ] Be able to make a call. Would make sure if the dialed digits could be sent
one by one when dialing, or if they should be sent in a row at the end of the
dialing (after a timeout) instead.
- [ ] Draw electronic scheme of the final board (Arduino pins wiring,
S63 wiring, and other electronic components placement) (e.g. using Fritzing).
- [ ] Design the final board circuit in order to build it (e.g. using KiCAD).
- [ ] [WIP] Constantly update the doc accordingly to the progresses done here.

## Future enhancements

- Support `*` and `#` dialings. May require some dialed digits combination and
timeouts.
- Put the board asleep when the phone is hung up in order to save power
consumption.

## Links

- Chip documentation :
    - https://store.arduino.cc/products/arduino-nano-every
    - https://www.microchip.com/en-us/product/ATMEGA4809 (datasheet available here)
    - https://github.com/arduino/ArduinoCore-megaavr/blob/master/variants/nona4809/pins_arduino.h#L114-L141

- S63 phone maintenance :
    - http://socotels63.blogspot.com/2011/05/tester-les-impulsions-du-socotel-s63.html
    - http://socotels63.blogspot.com/2011/05/modifier-la-duree-des-impulsions-du.html

- Rotary phones wiring :
    - https://www.oldphoneworks.com/files/oldphoneguy/rotatone/U43rotatone.pdf
    - http://www.l2l1.com/docs/cablage_u43.pdf
    - http://alain.levasseur.pagesperso-orange.fr/page22.htm
    - http://jla-1313-blog.overblog.com/2019/10/encore-un-convertisseur-pour-vos-anciens-telephones-a-cadran.html

- Other projects hacking a rotary phone :
    - https://github.com/ThomasChappe/S63_Arduino
    - https://github.com/revolunet/s63

- DTMF generation examples and PWM explanations :
    - http://ww1.microchip.com/downloads/en/Appnotes/doc1982.pdf
    - https://github.com/KC7MMI/AVR314-DTMF-Generator-for-Arduino
    - https://github.com/310weber/rotary_dial
    - https://github.com/inaxeon/rotarydial
    - https://passionelectronique.fr/pwm-arduino/
    - https://www.renesas.com/eu/en/document/apn/pwm-producing-dtmf-signal-dtmf?language=en
    - https://emalliab.wordpress.com/2022/01/23/arduino-nano-every-timers-and-pwm/

- TCA and TCB timers of the ATmega4809 chip :
    - http://ww1.microchip.com/downloads/en/AppNotes/TB3217-Getting-Started-with-TCA-90003217A.pdf
    - http://ww1.microchip.com/downloads/en/Appnotes/TB3214-Getting-Started-with-TCB-90003214A.pdf

- List of available registries keys and bitmasks for the ATmega4809 chip,
(using the Atmel AVR GCC toolchain ; requires this project installation first) :
    - `.arduino15/packages/arduino/tools/avr-gcc/7.3.0-atmel3.6.1-arduino5/avr/include/avr/iom4809.h`
