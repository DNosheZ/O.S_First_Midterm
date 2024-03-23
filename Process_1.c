#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
int main(int argc, char *argv[]){
	const int SIZE = 4096;
	const char *name="/VP";//nombre y tamano del espacio compartido de memoria
	int fd;
	char *ptr;//descriptor y puntero del area de memoria compartida	
	shm_unlink(name);
	fd=shm_open(name, O_CREAT | O_RDWR, 0666);
	//borrado preventivo y creacion del espacio de memoria compartida
	if (fd==-1){
	  perror("Error en el shm_open");
	  return(-1);
	}
	
	ftruncate(fd,SIZE);
	ptr=mmap(0,SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	//desde aqui se crea el semaforo para escribir la direccion del comando a procesar
	const char sh[]="/SOMH";//cuantos semaforos requerira este proceso
	sem_t *semola=sem_open(sh, O_CREAT, 0666,0);
	if (semh==SEM_FAILED){
	  	perror("Error en sem_open\n");
	  return 1;
	}

	char ruta[1024];
	FILE *fp;
	if(argc==2){/*verifico el paso de parametros*/
	 //el semaforo funcionara desde la revision de la ruta
			   //de acceso, o solo para guardar la ruta
			   //en la tuberia sin nombre
	  const char *ruta= argv[1];//ruta de acceso del comando a ejecutar
	  printf("Uso: p1 %s\n", ruta);
	    //aqui se ingresa la ruta de acceso en la tuberia
	    //empleando el pid del proceso padre
	    //verifica la existencia de la ruta de acceso
	  int pvRuta=open(ruta,O_RDONLY);
	   if(pvRuta<0){
		 return(-1);
	   }else{
		if(fork()==0){//Proceso hijo==Proceso_2
		  //variable para la lectura de la tuberia en formato string
		  char lectura[50];
		  read(fildes[0], lectura, strlen(ruta));
		  //aqui va la verificacion de la ruta de acceso
		  //crear segunda tuberia sin nombre para comunicar de proceso 2
		  //a proceso 1	    
     	   	}else{//Proceso padre==Proceso_1
		  write(fildes[1], ruta, strlen(ruta)); 
	   	} 
	//probablemente deba crear un semaforo para proteger y dar tiempo
	//al guardado de la direccion en el espacio de memoria
	}else{//mensaje default en caso de que no se asigne un parametro
              //de ejecucion
	  printf("Uso: p1 /ruta/al/ejecutable");
	}
	/*este proceso agrega a un espacio de memoria compartido 
	  la direccion de memoria del proceso que se paso como parametro*/
	return 0;
}
