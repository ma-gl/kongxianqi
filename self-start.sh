#!/bin/bash
insmod /diskE/work/drivers/spi-429/spidev-hi3593.ko
insmod /diskE/work/drivers/ls2k_es8311/ls2k_es8311.ko
stty -F /dev/ttyS1 460800 cs8 -icanon -ignpar -echo
stty -F /dev/ttyS4 115200 cs8 -icanon -ignpar -echo
stty -F /dev/ttyS5 115200 cs8 -icanon -ignpar -echo
stty -F /dev/ttyS6 115200 cs8 -icanon -ignpar -echo
stty -F /dev/ttyS7 9600 cs8 -icanon -ignpar -echo
stty -F /dev/ttyS8 115200 cs8 -icanon -ignpar -echo
chown root:root /diskE/work/build-BDShortMsg-unknown-Debug
chown root:root /diskE/work/build-BDShortMsg-unknown-Debug/BDSHortMsg
chmod 777 /diskE/work/build-BDShortMsg-unknown-Debug/BDSHortMsg
sleep 1
chmod 777 /dev/spi-hi3593.*
chmod 777 /dev/dsp
chmod 777 /dev/mixer
echo "drivers load completed."
#/diskE/work/build-BDShortMsg-unknown-Debug/BDSHortMsg -platform linuxfb
