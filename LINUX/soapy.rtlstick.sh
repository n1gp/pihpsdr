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
# SOAPY support for RTL sticks
#
################################################################

echo "=============================================================="
echo
echo "... installing SoapySDR RTL-stick libraries"
echo
echo "=============================================================="

cd $THISDIR
yes | rm -rf SoapyRTLSDR
git clone https://github.com/pothosware/SoapyRTLSDR

cd $THISDIR/SoapyRTLSDR
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make
sudo make install
sudo ldconfig

