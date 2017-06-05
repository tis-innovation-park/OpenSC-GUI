#!/bin/sh

version="1.1"

d=`dirname $0`
cd $d
if [ $? -ne 0 ]; then
  echo cannot cd to $d, aborting
  exit 1
fi

rm -rf packages/

mkdir -p packages/buergerkarte/usr/bin
mkdir -p packages/buergerkarte/usr/share/icons/
mkdir -p packages/buergerkarte/usr/share/applications/
mkdir -p packages/buergerkarte/usr/local/share/buergerkarte/
mkdir -p packages/buergerkarte/DEBIAN

cp ../../build/src/buergerkarte packages/buergerkarte/usr/bin
cp ../../icons/provinz_wappen.png packages/buergerkarte/usr/share/icons/
cp buergerkarte.desktop packages/buergerkarte/usr/share/applications/
cp ../OpenSC_PKCS11_Module_V1.2.xpi packages/buergerkarte/usr/local/share/buergerkarte/
cp control_template packages/buergerkarte/DEBIAN/control
echo "Version: $version" >> packages/buergerkarte/DEBIAN/control

cd packages
rm buergerkarte*.deb -f
fakeroot dpkg-deb --build buergerkarte
mv buergerkarte.deb buergerkarte-$version.deb
echo buergerkarte-$version.deb created
