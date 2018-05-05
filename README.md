# tp-2018-1c-Real-Mandril

operativos-pre-tp
------------------------------------------
Creado para guardar ejemplos y avances particulares que puedan servir para desarrollar el tp.

tp
------------------------------------------
Creado para desarrollar el tp y cada uno de sus modulos.

Para levantar el proyecto:</br>
  1°COORDINADOR</br>
    cd tp-2018-1c-Real-Mandril/tp/coordinador/src/</br>
    make</br>
    ./coordinador</br></br>
  2°PLANIFICADOR</br>
    cd tp-2018-1c-Real-Mandril/tp/planificador/src/</br>
    make</br>
    ./planificador</br></br>
  3°ESI</br>
    cd tp-2018-1c-Real-Mandril/tp/esi/src/</br>
    make</br>
    ./esi script.esi</br></br>
  4°INSTANCIA</br>
    cd tp-2018-1c-Real-Mandril/tp/instancia/src/</br>
    make</br>
    ./instancia</br>
Conexion entre diferentes ips
------------------------------------------
1° Poner maquina viertual en modo puente.</br>
2° Generar una mac distinta en cada VM.</br>
3° Una vez levantada la vm ver q los ips sean distintos</br>
4° Configurar el ip del servidor en 0.0.0.0 (puerto 8998, con este anduvo)</br>
LISTO!</br>
5° En caso de q no ande probar esto:
	- nc -l <puerto> (en este caso 8998) en la otra pc nc <ip-de-la-otra-pc> <puerto>
	- esto deberia poder dejar enviar msj entre pcs por ip, si esto no funciona hay problemas 		  en la red	
