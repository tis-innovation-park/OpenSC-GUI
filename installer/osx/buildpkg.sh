#!/bin/sh

currentDir=`dirname $0`
echo $currentDir
cd $currentDir
#cd ../;

mkdir Build/
cd Build/

mkdir -p Release/Buergerkarte.app/Contents/
mkdir Release/Buergerkarte.app/Contents/Resources/
mkdir Release/Buergerkarte.app/Contents/MacOS/

cp ../../../build/src/buergerkarte  Release/Buergerkarte.app/Contents/MacOS/
cp ../../../lib/opensc/lib/mac_osx/libopensc.dylib Release/Buergerkarte.app/Contents/MacOS/libopensc.3.dylib

pkgbuild --identifier buergerkarte.bz.it --version 1.0 --root Release/ --install-location /Applications/ "Buergerkarte for Mac OS X.pkg"
