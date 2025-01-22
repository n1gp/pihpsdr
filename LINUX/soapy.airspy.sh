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
# SOAPY: AirSpy support (contributed by Mitch AB4MW)
#                       (tested on RaspPi for Airspy HF+ Dual
#                                         and AirSpy HF+ Discovery)
#
################################################################

echo "=============================================================="
echo
echo "... installing Airspy One Host library"
echo
echo "=============================================================="

cd $THISDIR
rm -fr airspyone_host
git clone https://github.com/airspy/airspyone_host.git
cd airspyone_host
mkdir build
cd build
cmake ../ -DINSTALL_UDEV_RULES=ON
make -j
sudo make install
sudo ldconfig

echo "=============================================================="
echo
echo "... installing Airspy HF library"
echo
echo "=============================================================="

cd $THISDIR
rm -fr airspyhf
git clone https://github.com/airspy/airspyhf.git
cd airspyhf
mkdir build
cd build
cmake ../ -DINSTALL_UDEV_RULES=ON
make -j
sudo make install
sudo ldconfig

echo "=============================================================="
echo
echo "... installing Airspy SoapySDR Support"
echo
echo "=============================================================="

cd $THISDIR
yes | rm -rf SoapyAirspy
git clone https://github.com/pothosware/SoapyAirspy.git
cd SoapyAirspy
mkdir build
cd build
cmake ..
make -j
sudo make install

echo "=============================================================="
echo
echo "... installing AirspyHF SoapySDR Support"
echo
echo "=============================================================="

cd $THISDIR
yes | rm -rf SoapyAirspyHF
git clone https://github.com/pothosware/SoapyAirspyHF.git
cd SoapyAirspyHF
mkdir build
cd build
cmake ..
make -j
sudo make install
