#!/bin/sh

d=$(dirname $0)
cd $d

if [ $? -ne 0 ]; then
  echo cannot cd to $d, aborting
  exit 1
fi

rm packages/ -rf

d=$(date "+%Y.%m.%d-%H.%M") 

version="1.0a-$d"
mkdir -p packages/buergerkarte/usr/bin
mkdir -p packages/buergerkarte/DEBIAN
cp control_template packages/control
echo "Version: $version" >> packages/control

cp ../../build/src/buergerkarte packages/buergerkarte/usr/bin
cp packages/control packages/buergerkarte/DEBIAN
chown root.root packages/buergerkarte/usr/bin/buergerkarte


cd packages
rm buergerkarte*.deb
dpkg-deb --build buergerkarte
mv buergerkarte.deb buergerkarte-$version.deb
echo buergerkarte-$version.deb created
