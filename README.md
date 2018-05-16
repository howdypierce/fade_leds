# README: fade_leds

fade_leds is a quick-and-dirty command line utility that controls a
strand of WS2812 LEDs hanging off a Raspberry Pi.  These LEDs are
often sold by Adafruit under the brand name "Neopixels".

To use the utility:

	 `fade_leds color-1 color-2 [color...]`

...where each color is a hex string (with or without the leading 0x).
The program will immediately display color-1 on all LEDs in the
strand, and then will fade into color-2 over a one-second interval,
and so on with each color specified, taking one second to transition
between each color pair.  At the end of execution, the LEDs will be
left showing the last color listed.

At the moment everything is hardcoded for my particular setup, using a
strand of 5 LEDs and GPIO pin 18 (in "BCM" numbering; that corresponds
to pin 12 in "Board" numbering). To change these assumptions, edit the
#defines in `fade_leds.c`.  Obviously, in the long term all of these
should be made into command line parameters.

The fade_leds utility requires the rpi_ws281x library, available at
[GitHub](https://github.com/jgarff/rpi_ws281x). (Only the C library
`libws2811.a` is required from that project -- not the Python stuff.)

Because the LEDs are driven using DMA, this utility must be installed
setuid root.

To build:
```
   make
   sudo make install
```
