#!/bin/bash
echo ""
echo ...::: Iniciando compilación de los procesos :::...
echo ""

cd esi/src
make clean
make

cd ..

cd coordinador/src
make clean
make

cd ..

cd planificador/src
make clean
make

cd ..

cd instancia/src
make clean
make

echo ""
echo ...::: Procesos compilados correctamente :::...
echo ""

cd ..
