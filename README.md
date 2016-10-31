# pihpsdr
Raspberry Pi 3 standalone code for HPSDR, forked from G0ORX

Supports both the old and new ethernet protocols.

Download release/documentation/pihpsdr-install.pdf for instructions to install on RPi3.

N1GP UPDATES
============

Added support for SX1509 I2C Expander.

Check out GPIO changes in gpio.c for changes required to use a Waveshare
5" LCD Touchscreen since it uses some of the default pihpsdr GPIOs.
I disabled the I2C pins by putting the following in:
/etc/modprobe.d/raspi-blacklist.conf
blacklist i2c-bcm2708

This gave me BCM GPIOs 2 & 3

Ported the iambic keyer FPGA code Phil Harman VK6PH did in
the Verilog file iambic.v over to C and used nanosleep as a timing
mechanism.

This only works using the NEW protocol for now (see below). To use it make
sure to unselect "cw keyer internal" and select the mode you prefer. The
bug mode and straight key are implemented.

If you want you can unselect "cw breakin mode" and then you can use the "MOX"
button and send CW staying in PTT mode. Press MOX again to release PTT.

I also needed two more GPIOs for the paddles so I disabled UART0 giving me
BCM GPIOs 14 & 15.

To disable edit /boot/cmdline.txt and remove the console=ttyAMA0
and if you have an /etc/inittab comment out the following line:

T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100

Added a sidetone using either the PI's audio ouput jack or a GPIO by
utilizing wiringPi's softToneWrite(). In iambic.c set SIDETONE_GPIO = 0
for PI audio out or to an actual GPIO value for the sofToneWrite().

NOTES
=====
Currently the iambic keyer only works using the NEW ethernet protocol.
The current OLD protocol Hermes FGPA firmware doesn't support external
CW. I have modified the OLD FPGA firmware to support it for use with this
pihpsdr program, but currently it's experimental and use at your own risk.
Email me if you want to try it.

I noticed sometimes that when I started pihpsdr all of the GPIO button alerts would fire.
I've put in some experimental code using gpioGlitchFilter() and it seems to help.
