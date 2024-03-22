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
	const char sh[]="/SOMH";
	sem_t *semola=sem_open(sh, O_CREAT, 0666,0);
	if (semh==SEM_FAILED){
	  	perror("Error en sem_open\n");
	  return 1;
	}

	char ruta[1024];
	FILE *fp;
	if(argc<=2){/*verifico el paso de parametros*/
	  sem_wait(semola);

	  sem_post(semola);  
	//probablemente deba crear un semaforo para proteger y dar tiempo
	//al guardado de la direccion en el espacio de memoria
	}else{
	sem_wait(semola);
	ptr+=strlen();//en que formato pondremos lo direccion de archivo del comando ingresado
	sem_post(semola);
	
	}
	/*este proceso agrega a un espacio de memoria compartido 
	  la direccion de memoria del proceso que se paso como parametro
	*/
	sem_close(semola);
	return 0;
}
