#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MC "/myMemoryObj"
#define MC_SIZE 1024

int main() {
    sem_t *sem = sem_open("/verificacionExe", O_CREAT, 0644, 1); // Crea el semáforo

    if(sem == SEM_FAILED) {
        perror("sem_open failed");
        return 1;
    }
    sem_wait(sem);
    int memoria_compartida = shm_open(MC, O_RDWR, 0);
    if (memoria_compartida == -1) {
        perror("Error al abrir memoria compartida");
        exit(EXIT_FAILURE);
    }

    char *ptr;
    ptr = mmap(0, MC_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memoria_compartida, 0);
    if (ptr == MAP_FAILED) {
        perror("Error al mapear memoria compartida");
        exit(EXIT_FAILURE);
    }
  
    if (dup2(memoria_compartida, STDOUT_FILENO) == -1) {
        perror("Error al redirigir salida estándar");
        exit(EXIT_FAILURE);
    }

    char comando[MC_SIZE];
    strcpy(comando, ptr);
  
    memset(ptr, 0, MC_SIZE);
  
    if (execlp(comando, comando, NULL) == -1) {
        perror("Error al ejecutar el comando");
        exit(EXIT_FAILURE);
    }
    sem_post(sem); 
    sem_close(sem);

    return 0;
}
