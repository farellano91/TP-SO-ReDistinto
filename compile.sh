#!/bin/bash
echo ""
echo ...::: Iniciando compilación de los procesos :::...
echo ""

cd esi/
make clean
make

cd ..

cd coordinador/
make clean
make

cd ..

cd planificador/
make clean
make

cd ..

cd instancia
make clean
make

echo ""
echo ...::: Procesos compilados correctamente :::...
echo ""

cd ..
