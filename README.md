# Proyecto1SistemasOperativos
Proyecto 1 del curso Sistemas Operativos: Unix Shell

### INTEGRANTES:
* Cristian Camilo Tabares Perez
* Jean Paul Gonzalez Pedraza

### COMPILACION DEL PROGRAMA:
  Para compilar el programa, se abre la terminal y dentro de la carpeta se ejecuta el comando make unixShell, definido en el makefile.
  Una vez compilado el programa, se generara un archivo ejecutable del programa.
  
### EJECUCION DEL PROGRAMA:
  Despues de compilar el programa, en la terminal ejecutamos el comando ./unixShell y eso ejecutara el programa.
  
### ALGUNAS CONSIDERACIONES:
  * El comando "&" no lo estamos aceptando en el programa, esto porque en el enunciado del proyecto no es muy claro sobre que se deberia hacer si el        usuario digita ese comando.
  * Al ejecutar el programa, aparecen unos warnings, esto debido a que hacemos un llamado a una funcion de una forma que genera esos warnings, sin embargo, el codigo funciona al 100%.
  * Se le agrego al makefile, que al ejecutar make clean tambien elimine los archivos .txt creados durante la ejecucion.
