#!/bin/bash
echo ""
echo ...::: Iniciando compilación de los procesos :::...
echo ""

<<<<<<< HEAD
cd esi/
=======
cd esi/src
>>>>>>> origin/versionVieja
make clean
make

cd ..

<<<<<<< HEAD
cd coordinador/
=======
cd coordinador/src
>>>>>>> origin/versionVieja
make clean
make

cd ..

<<<<<<< HEAD
cd planificador/
=======
cd planificador/src
>>>>>>> origin/versionVieja
make clean
make

cd ..

<<<<<<< HEAD
cd instancia
=======
cd instancia/src
>>>>>>> origin/versionVieja
make clean
make

echo ""
echo ...::: Procesos compilados correctamente :::...
echo ""

cd ..
