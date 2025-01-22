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
# SOAPY: AdalmPluto support
#
# A) download and install libiio
#    NOTE: libiio has just changed the API and SoapyPlutoSDR
#          is not yet updated. So compile version 0.25, which
#          is the last one with the old API
#
# CURRENTLY DISABLED: apt get libiio-dev also works (give v0.23)
#
################################################################
#echo "... installing libiio with old API"
#
#cd $THISDIR
#yes | rm -r libiio
#git clone https://github.com/analogdevicesinc/libiio.git
#git checkout v0.25
#
#cd $THISDIR
#mkdir build
#cd build
#cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
#make
#sudo make install
#sudo ldconfig

################################################################
#
# B) download and install Soapy for Adalm Pluto
#
################################################################

echo "=============================================================="
echo
echo "... installing SoapySDR AdalmPluto libraries"
echo
echo "=============================================================="

cd $THISDIR
yes | rm -rf SoapyPlutoSDR
git clone https://github.com/pothosware/SoapyPlutoSDR

cd $THISDIR/SoapyPlutoSDR
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make
sudo make install
sudo ldconfig
