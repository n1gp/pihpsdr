#!/bin/sh

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
# SOAPY: HackRF support
#        source: https://github.com/pothosware/SoapyHackRF/wiki
#
################################################################

echo "=============================================================="
echo
echo "... installing HackRF SoapySDR support"
echo
echo "=============================================================="

cd $THISDIR
yes | sudo apt-get install libhackrf-dev
yes | rm -rf SoapyHackRF
git clone https://github.com/pothosware/SoapyHackRF.git
cd SoapyHackRF
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
