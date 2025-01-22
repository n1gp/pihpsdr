#!/bin/sh

################################################################
#
# A script to setup everything that needs be done on a
# 'virgin' RaspPi.
#
################################################################

################################################################
#
# a) determine the location of THIS script
#    (this is where the files should be located)
#    and assume this is in the pihpsdr directory
#
################################################################

SCRIPTFILE=`realpath $0`
THISDIR=`dirname $SCRIPTFILE`

################################################################
#
# SOAPY: SDRPlay support (contributed by Mitch AB4MW)
#                        (tested on RaspPi for SDRPlay RSP1a)
#
################################################################

echo "=============================================================="
echo
echo "... installing SDRPlay3 driver and SoapySDR Support"
echo
echo "=============================================================="

cd $THISDIR
rm -f SDRplay*.run*
wget -O SDRplay_RSP_API-Linux-3.15.2.run \
https://www.sdrplay.com/software/SDRplay_RSP_API-Linux-3.15.2.run

chmod +x SDRplay_RSP_API-Linux-3.15.2.run
$THISDIR/SDRplay_RSP_API-Linux-3.15.2.run

yes | rm -rf SoapySDRPlay3
git clone https://github.com/pothosware/SoapySDRPlay3

cd $THISDIR/SoapySDRPlay3
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j
sudo make install
sudo ldconfig
