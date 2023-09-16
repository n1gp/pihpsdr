#!/bin/sh

#####################################################
#
# prepeare your Macintosh for compiling piHPSDR
#
######################################################

################################################################
#
# a) MacOS does not have "realpath" so we need to fiddle around
#
################################################################

THISDIR="$(cd "$(dirname "$0")" && pwd -P)"

################################################################
#
# b) Initialize HomeBrew and required packages
#    (this does no harm if HomeBrew is already installed)
#
################################################################
  
#
# This installs the "command line tools", these are necessary to install the
# homebrew universe
#
xcode-select --install

#
# This installes the core of the homebrew universe
#
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"

#
# At this point, there is a "brew" command either in /usr/local/bin (Intel Mac) or in
# /opt/homebrew/bin (Silicon Mac). Look what applies
#
BREW=junk

if [ -x /usr/local/bin/brew ]; then
  BREW=/usr/local/bin/brew
fi

if [ -x /opt/homebrew/bin/brew ]; then
  BREW=/opt/homebrew/bin/brew
fi

if [ $BREW == "junk" ]; then
  echo HomeBrew installation obviously failed, exiting
  exit
fi

#
# This adjusts the PATH
#
if [ $SHELL == "/bin/sh" ]; then
$BREW shellenv sh >> $HOME/.profile
fi
if [ $SHELL == "/bin/csh" ]; then
$BREW shellenv csh >> $HOME/.cshrc
fi

#
# All needed for pihpsdr
#
$BREW install gtk+3
$BREW install librsvg
$BREW install pkg-config
$BREW install portaudio
$BREW install fftw
$BREW install libusb

#
# This is for the SoapySDR universe
# There are even more radios supported for which you need
# additional modules, for a list, goto the web page
# https://formulae.brew.sh
# and insert the search string "pothosware". In the long
# list produced, search for the same string using the
# "search" facility of your internet browser
#
$BREW install cmake
#
# This may be necessary if an older version exists
#
$BREW uninstall soapysdr
$BREW install pothosware/pothos/soapyplutosdr
$BREW install pothosware/pothos/limesuite
$BREW install pothosware/pothos/soapyrtlsdr
$BREW install pothosware/pothos/soapyairspy
$BREW install pothosware/pothos/soapyairspyhf
$BREW install pothosware/pothos/soapyhackrf
$BREW install pothosware/pothos/soapyredpitaya
$BREW install pothosware/pothos/soapyrtlsdr

#
# This is for PrivacyProtection
#
$BREW analytics off

################################################################
#
# c) create links in /usr/local if necessary (only if
#    HomeBrew is installed in /opt/local
#
################################################################

if [ ! -d /usr/local/lib ]; then
  echo "/usr/local/lib does not exist, creating symbolic link ..."
  sudo "rm -f /usr/local/lib; ln -s /opt/local/lib /usr/local/lib"
fi
if [ ! -d /usr/local/bin ]; then
  echo "/usr/local/bin does not exist, creating symbolic link ..."
  sudo "rm -f /usr/local/bin; ln -s /opt/local/bin /usr/local/bin"
fi
if [ ! -d /usr/local/include ]; then
  echo "/usr/local/include does not exist, creating symbolic link ..."
  sudo "rm -f /usr/local/include; ln -s /opt/local/include /usr/local/include"
fi

################################################################
#
# d) download and install WDSP
#
################################################################
cd $THISDIR
yes | rm -r wdsp
git clone https://github.com/dl1ycf/wdsp

cd $THISDIR/wdsp
make -j 4
make install

