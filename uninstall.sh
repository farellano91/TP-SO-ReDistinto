#!/bin/bash

cd /home/utnso

echo ""
echo "Desinstalando Commons"
echo ""

cd so-commons-library
sudo make uninstall

echo ""
echo "Commons desinstaladas"
echo ""

cd ..

echo ""
echo "Desinstalando Parsi"
echo ""


cd parsi/src
sudo make uninstall

echo ""
echo "Parsi desinstalado"
echo ""

cd /home/utnso/tp-2018-1c-Real-Mandril
