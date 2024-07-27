#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>

#define  name2 "/VP"
#define  SIZE2 1024
#define SEM_PROD_NAME "/sem_prod"
#define SEM_REC_NAME "/sem_rec"
#define SEM_VER_NAME "/sem_ver"

int main() {
  
  // Abre los semáforos
  sem_t *sem_prod, *sem_rec, *sem_ver;
  sem_prod = sem_open("/sem_prod", O_CREAT, 0644, 0);
  sem_rec = sem_open("/sem_rec", O_CREAT, 0644, 0);
  sem_ver = sem_open("/sem_ver", O_CREAT, 0644, 0);

  if (sem_prod == SEM_FAILED || sem_rec == SEM_FAILED || sem_ver == SEM_FAILED) {
      perror("Error al abrir semáforos");
      exit(EXIT_FAILURE);
  }
  
  //damos la señal de que el proceso 3 se esta ejecutando
  sem_post(sem_ver);

  // mensaje y espera por el comando
  printf("Esperando... \n");
  sem_wait(sem_rec);

  // establecemos los parametros para el area de memoria compartida
  int fd2;
  
  // abrimos el area de memoria compartida
  fd2 = shm_open(name2, O_RDWR, 0666);
  if (fd2 == -1) {
      perror("Error en el shm_open");
      return 1;
  }

  // mapeamos el area de memoria compartida
  char *ptr2;
  ptr2 = mmap(0, SIZE2, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
    if (ptr2 == MAP_FAILED) {
      perror("Error al mapear el objeto de memoria compartida");
      return 1;
    }

  //creamos la variable que almacena el comando
  printf("Leyendo de la memoria...\n");
  char comando[SIZE2];
  strcpy(comando, ptr2);
  printf("Leido %s\n", comando);
  printf("Ejecutando %s\n", comando);
  printf("Proceso terminado\n");

  // eliminamos todo lo que este en el area de memoria compartida
  memset(ptr2, 0, sizeof(comando));

  // desbloqueamos el proceso 2
  sem_post(sem_rec);

  // cerramos los elementos que ya no necesitamos
  sem_close(sem_prod);
  sem_close(sem_rec);
  sem_close(sem_ver);
  sem_unlink(SEM_PROD_NAME);
  sem_unlink(SEM_REC_NAME);
  sem_unlink(SEM_VER_NAME);
  
  // creamos la tuberia
  int fildes4[2];
  if (pipe(fildes4) < 0 ) {
        perror("Error al crear la tuberia");
        return 1;
    }

  close(fildes4[1]);
  
  // desviamos la salida estandar a la tuberia 
  if (dup2(fildes4[1], STDOUT_FILENO) == -1) {
      perror("Error al redirigir salida estándar");
      exit(EXIT_FAILURE);
  }

  // escribimos el comando en el area de memoria compartida
  char buffer[1024];
  ssize_t count;

  while ((count = read(fildes4[0], buffer, sizeof(buffer))) > 0) {
      memcpy(ptr2, buffer, count);
  }

  close(fildes4[0]);
  
  // ejecutamos el comando 
  if (execlp(comando, comando, NULL) == -1) {
    perror("Error al ejecutar el comando");
      exit(EXIT_FAILURE);
  }
}
