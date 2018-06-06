#!/bin/bash

cd /home/utnso

echo ""
echo "Clonando Commons"
echo ""

git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
sudo make install

echo ""
echo "Commons instaladas"
echo ""

cd ..

echo ""
echo "Clonando Parsi"
echo ""

git clone https://github.com/sisoputnfrba/parsi
cd parsi
sudo make install

echo ""
echo "Parsi instalado"
echo ""

cd /home/utnso/tp-2018-1c-Real-Mandril

compile.sh


