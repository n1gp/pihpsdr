# pihpsdr
Raspberry Pi 3 standalone code for HPSDR

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

Added an iambic keyer based on the verilog code Phil Harman VK6PH did in
iambic.v.

I also needed two more GPIOs for the paddles so I disabled UART0 giving me
BCM GPIOs 14 & 15.

To disable edit /boot/cmdline.txt and remove the console=ttyAMA0
and if you have an /etc/inittab comment out the following line like:
#T0:23:respawn:/sbin/getty -L ttyAMA0 115200 vt100

I also repurposed the AF_FUNCTION GPIO for a sidetone output hooked to my internal
speaker. It's not the greatest sounding but it is timed pretty well.

NOTES:
I noticed sometimes that when I started pihpsdr all of the GPIO button alerts would fire.
I've put in some experimental code using gpioGlitchFilter() and it seems to help.
