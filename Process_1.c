#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
int main(int argc, char *argv[]){
	char ruta[1024];
	FILE *fp;
	if(argc==2){/*verifico el paso de parametros*/
	  const char *ruta= argv[1];//ruta de acceso del comando a ejecutar
	  printf("Uso: p1 %s\n", ruta);
	  if(fork()==0){//Proceso 2
		char lectura[50];
		read(fildes1[0], lectura, strlen(ruta));
  		  //se abre memoria compartida para con Process_3.c
  		const int SIZE2 = 4096;
        const char *name2="/VP";//nombre y tamano del espacio compartido de memoria
        int fd2;
       	char *ptr2;//descriptor y puntero del area de memoria compartida
        shm_unlink(name2);
        fd2=shm_open(name2, O_CREAT | O_RDWR, 0666);
        //borrado preventivo y creacion del espacio de memoria compartida que comunica con Process_3.c
        if (fd2==-1){
            perror("Error en el shm_open");
            return(-1);
        }
	    ftruncate(fd2,SIZE2);
        ptr2=mmap(0,SIZE2,PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);

		  //aqui va la verificacion de la ruta de acceso dentro del sistema operativo
		 
		int VeriRuta=open(lectura,O_RDONLY);
		if(VeriRuta<0){
			int InexistenciaRuta=1;//este interruptor si esta bien implementado? (Javi respondio en el ultimo mensaje)
		    write(fildes2[1], &InexistenciaRuta, strlen(ruta)); 
		}
		  //crear segunda tuberia sin nombre para comunicar de proceso 2
		  //a proceso 1
		//crear espacio me memoria para comunicarse con el proceso 3, enviando la ruta de acceso ya verificada
		else{
			const int SIZE3 = 4096;
        	const char *name3="/VP";//nombre y tamano del espacio compartido de memoria
        	int fd3;
       		char *ptr3;//descriptor y puntero del area de memoria compartida
        	shm_unlink(name3);
        	fd3=shm_open(name3, O_CREAT | O_RDWR, 0666);
        //borrado preventivo y creacion del espacio de memoria compartida que comunica con Process_3.c
			if (fd3==-1){
				perror("Error en el shm_open");
				return(-1);
			}
			ftruncate(fd3,SIZE3);
			ptr2=mmap(0,SIZE3,PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
			//como comunico el espacio de memoria de este .C con el .C donde vive el proceso 3?

			//debo recibir a traves del mismo espacio de memoria, la respuesta al comando ejecutado desde el proceso 3
			//la respuesta se enviara a traves de una tuberia sin nombre al proceso 1

		}

	  }else{//Proceso 1
		//crea la tuberia para enviar la ruta de acceso
		 int fildes1[2];
	     if (pipe(fildes1)<0){
			perror("Error al crear la tuberia\n");
	     	return (1);
		 }
		 write(fildes1[1], ruta, strlen(ruta)); 
		 //crea la tuberia para verificar la existencia de la ruta de acceso
		 int fildes2[2];
	     if (pipe(fildes2)<0){
			perror("Error al crear la tuberia\n");
			return (1);
		 }
		 int ExistRut=0;
		 read(fildes2[0], ExistRut, strlen(ruta));
		 if(ExistRut==1){
			printf("No se encuentra el archivo a ejecutar.");
		 }
		//crear tuberia para recibir el resultado del comando ejecutado como parametro
		//se recibe por el proceso 2
		//como muestro la respuesta a ejecucion de un proceso traido desde una tuberia sin nombre
	  }

	} 
	//probablemente deba crear un semaforo para proteger y dar tiempo
	//al guardado de la direccion en el espacio de memoria
	}else{//mensaje default en caso de que no se asigne un parametro de ejecucion
	  printf("Uso: p1 /ruta/al/ejecutable");
	}
	close(fildes1[0]);
	close(fildes1[1]);
	close(fildes2[0]);
	close(fildes2[1]);
	return 0;
}
