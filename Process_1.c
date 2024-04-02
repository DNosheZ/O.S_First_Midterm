#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>

 
// definimos variables globales
#define  name2 "/VP"
#define  SIZE2 1024
#define SEM_PROD_NAME "/sem_prod"
#define SEM_REC_NAME "/sem_rec"
#define SEM_VER_NAME "/sem_ver"

//definimos la funcion principal
int main(int argc, char *argv[]) {

    //verificamos que se haya ingresado un argumento
    if (argc != 2) {
        printf("Uso: %s /ruta/al/ejecutable\n", argv[0]);
        return 1;
    }

    // creamos la variable que contendra la ruta del archivo
    char ruta[1024];
    strcpy(ruta, argv[1]);
  
    // creamos las tuberias
    int fildes1[2];
    int fildes2[2];
    int fildes3[2];
  
    if (pipe(fildes1) < 0 || pipe(fildes2) < 0 || pipe(fildes3) < 0){
        perror("Error al crear la tuberia");
        return 1;
    }
  
  // Abre los semáforos
  sem_t *sem_prod, *sem_rec, *sem_ver;
  sem_prod = sem_open("/sem_prod", O_CREAT, 0644, 0);
  sem_rec = sem_open("/sem_rec", O_CREAT, 0644, 0);
  sem_ver = sem_open("/sem_ver", O_CREAT, 0644, 0);

  if (sem_prod == SEM_FAILED || sem_rec == SEM_FAILED || sem_ver == SEM_FAILED) {
      perror("Error al abrir semáforos");
      exit(EXIT_FAILURE);
  }
    
    // creamos el proceso hijo
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error al crear el proceso hijo");
        return 1;
    }
  
    // Proceso hijo (Proceso 2)
    if (pid == 0) { 
      
        // cerramos los extremos de la tuberia que no se usaran
        close(fildes1[1]);
        close(fildes2[0]);
        close(fildes3[0]);

        // creamos variables necesarias
        char lectura[1024];
        int valor;
        sem_getvalue(sem_ver, &valor);
        
        // escribimos el comando en la variable lectura
        ssize_t bytes_read = read(fildes1[0], lectura, sizeof(lectura));
        if (bytes_read < 0) {
            perror("Error al leer desde la tuberia");
            return 1;
        }
        
        // verificamos la existenciad de la ruta del archivo
        int InexistenciaRuta = 0;
        if (access(ruta, F_OK) < 0) {
            InexistenciaRuta = 1;
            ssize_t bytes_written = write(fildes2[1], &InexistenciaRuta, sizeof(int));
            if (bytes_written < 0) {
                perror("Error al escribir en la tuberia");
                return 1;
            }
        } 
      
        // si la ruta existe, verificamos que el proceso 3 este en ejecucion
        else if (valor == 0) {
            InexistenciaRuta = 2;
            ssize_t bytes_written = write(fildes2[1], &InexistenciaRuta, sizeof(int));
            if (bytes_written < 0) {
              perror("Error al escribir en la tuberia");
              return 1;
            }
          }
          
        // si el proceso 3 esta en ejecucion continuamos
        else{

          // establecemos los parametros para el area de memoria compartida
          int fd2;
          char *ptr2;

          // abrimos el area de memoria compartida
          fd2 = shm_open(name2, O_CREAT | O_RDWR, 00600);
          if (fd2 == -1) {
            perror("Error al abrir el area de memoria compartida");
            return 1;
          }

          // configuramos el tamaño del area de memoria compartida
          if (ftruncate(fd2, SIZE2) == -1) {
            perror("Error al truncar memoria compartida");
            exit(EXIT_FAILURE);
          }

          // mapeamos el area de memoria compartida
          ptr2 = mmap(0, SIZE2, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
          if (ptr2 == MAP_FAILED) {
            perror("Error al mapear el objeto de memoria compartida");
            return 1;
          }
          
          // escribimos el comando en el area de memoria compartida
          memcpy(ptr2, lectura, bytes_read);

          // damos la señal de que el proceso 2 ya ingreso el comando
          sem_post(sem_rec);

          // esperamos a que el proceso 3 termine
          sleep(1);

          // creamos la variable que almacena la ejecucion del comando
          char ejecucion[SIZE2];
          strcpy(ejecucion, ptr2);

          // escribimos en la tuberia la ejecucion
          ssize_t escritura_ejecucion = write(fildes3[1], &ejecucion, sizeof(ejecucion));
          if (escritura_ejecucion < 0) {
            perror("Error al escribir en la tuberia");
            return 1;
                 
        }
      }

      // cerramos los elementos que ya no necesitamos
      close(fildes3[1]);
      close(fildes1[0]);
      close(fildes2[1]);
      munmap(name2, SIZE2);
      shm_unlink(name2);
      sem_close(sem_prod);
      sem_close(sem_rec);
      sem_close(sem_ver);
      sem_unlink(SEM_PROD_NAME);
      sem_unlink(SEM_REC_NAME);
      sem_unlink(SEM_VER_NAME);
      
    }
    // Proceso padre (Proceso 1)
    else { 

        // cerramos el extremo de la tuberia que no se usara
        close(fildes1[0]);
        close(fildes2[1]);
        close(fildes3[1]);
      
        // escribimos el comando en la tuberia
        ssize_t bytes_written = write(fildes1[1], ruta, sizeof(ruta));
        if (bytes_written < 0) {
            perror("Error al escribir en la tuberia");
            return 1;
        }

        // recibimos las verificaciones del proceso 2
        int ExistRut = 0;
        ssize_t bytes_read = read(fildes2[0], &ExistRut, sizeof(int));
        if (bytes_read < 0) {
            perror("Error al leer desde la tuberia");
            return 1;
        }

        // confirmamos el caso de terminacion e imprimimos el mensaje correspondiente
        if (ExistRut == 1) {
            printf("No se encuentra el archivo a ejecutar.\n");
            exit(1);
        } 
        else if (ExistRut == 2) {
            printf("Proceso p3 no parece estar en ejecución.\n");
            exit(1);
        }

      // leemos la ejecucion del comando de la tuberia
      char ejecucion[SIZE2];
      ssize_t lectura_ejecucion = read(fildes3[0], &ejecucion, sizeof(ejecucion));
      if (bytes_read < 0) {
          perror("Error al leer desde la tuberia");
          return 1;
      }

      // imprimimos la ejecucion del comando
      printf("%s", ejecucion);
      printf("Procesos terminados\n");
      
      // cerramos los elementos que ya no necesitamos
      close(fildes1[1]);
      close(fildes2[0]);
      close(fildes3[0]);
      
    return 0;
}
  
}
      
