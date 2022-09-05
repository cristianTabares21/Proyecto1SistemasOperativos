#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LINE 80  // Cantidad maxima de caracteres para el comando
#define LECTURA 0  // Extremo de lectura para el pipe
#define ESCRITURA 1  // Extremo de escritura para el pipe
#define FILE_NAME "historial.txt"  // Nombre por defecto del archivo que utilizamos para almacenar el comando anterior

int hayPipes(char commands[]){  // Funcion que calcula si hay pipes ("|") en el comando
    if(strchr(commands,'|')!=NULL){  // Busca el caracter en el comando
        return 1;  
    }else{
        return 0;  
    }
}

int hayRedireccionamiento(char commands[]){  // Funcion que calcula si hay redireccionamiento de salida (">") en el comando
    if(strchr(commands, '>') != NULL){  // Busca el caracter en el comando
        return 1;  
    }else{
        return 0;
    }
}

int contarCadena(char commands[]){  // Funcion que se encarga de contar cuantos tokens hay en el comando
    char *puntero;
    int count = 0;
    puntero = strtok(commands, " ");  // Se separa el comando por espacios

    while(puntero != NULL){
        puntero = strtok(NULL, " ");  // Se recorre todo el comando
        count++;  // Se aumenta el contador cuando es un token
    }
    return count;
}

void cadenaTokens(char commands[], char *salida[]){  // Funcion que se encarga de almacenar en una lista los tokens del comando
    char *puntero;
    int i = 0;
    puntero = strtok(commands, " ");  // Se separa el comando por espacios

    while(puntero != NULL){
        salida[i] = puntero;  // Se almacena el token en el nuevo arreglo
        puntero = strtok(NULL, " ");  // Se recorre todo el comando
        i++;  // Se aumenta la posicion
    }
    salida[i] = NULL;  // Se hace que la ultima posicion del arreglo sea NULL
}

void partirCadena(char delimitador[], char commands[], char *splitCadena[]){  // Funcion que se encarga de partir una cadena dado un delimitador
    char *puntero;
    int i = 0;
    puntero = strtok(commands, delimitador);  // Se parte la cadena segun el delimitador "|" o ">"

    while(puntero != NULL){
        splitCadena[i] = puntero;  // Se almacenan las partes que quedan despues de partir la cadena
        puntero = strtok(NULL, delimitador);  // Se recorre todo el comando
        i++;  // Se aumenta la posicion
    }
}

void ejecucionComandoBasico(char *tokens[], char commands[]){  // Programa que ejecuta un comando basico, sin pipes ni redirecciones
    if(execvp(tokens[0], tokens)==-1){  // Ejecuta el execvp con los argumentos que recibe del comando digito por el usuario
        printf("Error en el comando: %s\n", commands);  // En caso de error, imprime el error por pantalla
    }
}

void ejecucionUnPipe(char *args1[], char *args2[], char commands[]){  // Funcion que ejecuta el comando pipe ("|")
    pid_t pid1, pid2;  // Utilizamos dos procesos
    int pipe1[2];  // Creamos un pipe
    pipe(pipe1);

    pid1 = fork();  // Creamos el primer proceso
    if(pid1 == 0){  // Proceso hijo 1
        dup2(pipe1[ESCRITURA], STDOUT_FILENO);  // Dup2 recibe lo que se va a escribir por el pipe y lo duplica como salida estandar
        close(pipe1[LECTURA]);  // Se cierra el extremo del pipe que no va a ser utilizado
        if(execvp(args1[0], args1) == -1){  // ejecutamos el execvp con los argumentos del comando
            printf("Error en el comando: %s\n", commands);  // Si el execvp falla imprime el error
        }
    }

    pid2 = fork();  // Creamos el segundo proceso
    if(pid2 == 0){  // Proceso hijo 2
        dup2(pipe1[LECTURA], STDIN_FILENO);  // Dup2 recibe lo que se lee por el pipe y lo duplica como entrada estandar
        close(pipe1[ESCRITURA]);  // Se cierra el extremo del pipe que no va a ser utilizado
        if(execvp(args2[0], args2) == -1){  // Se ejecuta el segundo execvp con los argumentos del comando
            printf("Error en el comando: %s\n", commands);  // Si el execvp falla imprime el error
        }
    }
    close(pipe1[LECTURA]);  // Se cierra el extremo de lectura del pipe
    close(pipe1[ESCRITURA]);  // Se cierra el extremo de escritura del pipe
    wait(NULL);  // Se espera a que los hijos finalicen
}

void ejecucionRedireccionamiento(char commands[], char nombreArchivo[]){  // Funcion que se encarga de escribir en un archivo de texto
    close(STDOUT_FILENO);  // Se cierra la escritura estandar ya que no se va a enviar nada
    creat(nombreArchivo, S_IRWXU);  //  Se usa la funcion create para crear el archivo en caso de que no exista con el nombre pasado por los parametros
    // ademas, se le da el permiso de lectura, escritura y ejecucion
    ejecucionPrograma(commands);  // Se ejecuta la funcion que se encarga de ejecutar todo el programa
}

void ejecucionPrograma(char commands[]){  
    int tamañoComando = strlen(commands);  // Se obtiene el tamaño del comando del usuario
    int pipesInCommand = hayPipes(commands);  // Variable que almacena 1 si hay un pipe o 0 si no lo hay
    int redirInCommand = hayRedireccionamiento(commands);  // Variable que almacena 1 si hay redireccion y 0 si no

    char copia[tamañoComando];  // Se crea una copia del comando del usuario
    strcpy(copia, commands);

    if(pipesInCommand == 0 && redirInCommand == 0){  // No hay pipes ni hay redireccion
        int tamCommand = contarCadena(copia);  // Se llama a la funcion para saber cuantos tokens hay en el comando ej: ls -la = 2 tokens
        char *tokens[tamCommand+1];  // Se crea la lista que va a almacenar los tokens con el tamaño calculado y se le adiciona 1 porque en la posicion final va NULL
        cadenaTokens(commands, tokens);  // Se llama a la funcion que dado el comando del usuario, coge cada token y lo guarda en una lista nueva sin espacios
        ejecucionComandoBasico(tokens, commands);  // Se llama a la funcion que ejecuta un comando basico
    }   
    else if(pipesInCommand == 1 && redirInCommand == 0){  // Hay pipes pero no hay redireccion
        char *cadenaDividida[2];  // Se crea una lista nueva que va a contener dos partes del comando, la parte antes del pipe y la parte despues del pipe
        partirCadena("|", copia, cadenaDividida);  // Se llama a la funcion que dado un delimitador parte el comando en dos
        
        // Creacion de la sub cadena 1
        char copia1[strlen(cadenaDividida[0])];  // Se crea una copia de la primera parte del comando
        strcpy(copia1, cadenaDividida[0]);
        int tamSubCadena1 = contarCadena(copia1);  // Se cuenta cuantos tokens tiene esa primera parte
        char *tokensSubCadena1[tamSubCadena1+1];  // Se crea una nueva lista donde se almacenara los tokens de  la primera parte sin espacios
        cadenaTokens(cadenaDividida[0], tokensSubCadena1);  // Se llama a la funcion para que guarde en esa nueva lista todos los tokens
        
        // Creacion de la sub cadena 2
        char copia2[strlen(cadenaDividida[1])];  // Hace lo mismo que lo anterior pero con la parte despues del pipe
        strcpy(copia2, cadenaDividida[1]);
        int tamSubCadena2 = contarCadena(copia2);
        char *tokensSubCadena2[tamSubCadena2+1];
        cadenaTokens(cadenaDividida[1], tokensSubCadena2);

        // Se ejecuta el programa con la sub cadena 1 y la sub cadena 2
        ejecucionUnPipe(tokensSubCadena1, tokensSubCadena2, commands);  // Se llama a la funcion que ejecuta un pipe
    }
    else if((pipesInCommand==0 || pipesInCommand==1) && redirInCommand == 1){  // puede o no haber pipes pero si hay redireccionamiento de salida
        char *cadenaDividida[2];  // Se crea una lista que contendra dos partes del comando, la parte antes del ">" y la parte despues del ">" que sera el nombre del archivo
        partirCadena(">", commands, cadenaDividida);  // Se parte la cadena dado el delimitador (">")
        cadenaDividida[1] = strtok(cadenaDividida[1], " ");  // Se parte la primera parte del comando por espacios, es decir, quedan los tokens sin espacios
        ejecucionRedireccionamiento(cadenaDividida[0], cadenaDividida[1]);  // Se llama la funcion que escribe en un archivo la salida de la ejecucion
    }
}

int main(){  // Funcion main
    int salida = 1;  // Flag de salida
    int i = 0;  // Contador de cantidad de comandos ingresados

    while(salida){  // While de ejecucion

        printf("CC-JP-Shell> ");  // Nombre del shell <decorativo>
        fflush(stdout);  
        char commands[MAX_LINE];  // Se crea una lista que contendra el comando del usuario
        scanf(" %99[^\n]", commands);  // Se lee el comando digitado por el usuario 
        char recuperar[MAX_LINE];  // Se crea una lista que contendra el ultimo comando digitado por el usuario
        FILE *fichero;  // Se crea un fichero

        if(i == 0){  // Si es el primer comando que se ejecuta
            fichero = fopen(FILE_NAME, "w");  // Se abre el fichero con permiso de escritura
            fputs(commands, fichero);  // Se escribe el comando en el fichero
            fclose(fichero);  // Se cierra el fichero

        }else{  // Si ya se ha ejecutado otro comando antes
            fichero = fopen(FILE_NAME, "rt");  // Se abre el fichero con permisos de lectura 
            fgets (recuperar, MAX_LINE, fichero);  // Se lee el ultimo comando que habia digitado el usuario
            fclose(fichero);  // Se cierra el fichero

            fichero = fopen(FILE_NAME, "w");  // Se abre el fichero con permiso de escritura
            fputs(commands, fichero);  // Se escribe el comando en el fichero
            fclose(fichero);  // Se cierra el fichero
        }

        if(strcmp("exit", commands) == 0){  // Si el comando digitado es "exit", se sale de la ejecucion
            break;  // Finaliza el programa
        }
        
        pid_t pid = fork();  // Se crea un proceso
        if(pid == 0){  // Proceso hijo
            if(strcmp("!!", commands) == 0 && i != 0){  // Si el usuario digita !! y ya se ha ejecutado un comando antes
                ejecucionPrograma(recuperar);  // Se ejecuta el programa con el ultimo comando que se habia ejecutado
                break;  // Finaliza para recibir otro comando
            }
            else if (strcmp("!!", commands) == 0 && i == 0){  // Si el usuario digita !! y no se ha ejecutado un comando antes
                printf("No hay comando para ejecutar\n");  // Imprime un error
                break;
            }
            else{  // Si no hay !!
                ejecucionPrograma(commands);  // Se ejecuta el programa con el comando ingresado
                break;  // Finaliza para recibir otro comando
            }
        }else{  // Proceso padre
            wait(NULL);  // El padre espera al hijo
        }
        i++;  // Se incrementa el contador de comandos ejecutados
    }
    return 0;
}