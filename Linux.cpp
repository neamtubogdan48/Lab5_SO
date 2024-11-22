#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <cstdlib>
#include <ctime>
#include <sys/wait.h>
#define SHARED_MEM_NAME "/num_count_memory"
#define SEMAPHORE_NAME "/num_count_semaphore"
using namespace std;
int main() {
    srand(time(nullptr));
    int shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        cerr << "Eroare la crearea obiectului de memorie partajata" << endl;
        return 1;
    }
    ftruncate(shm_fd, sizeof(int));
    int *counter = (int*) mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (counter == MAP_FAILED) {
        cerr << "Eroare la maparea memoriei" << endl;
        return 1;
    }
    *counter = 1;
    sem_t *sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0666, 1);    
    if (sem == SEM_FAILED) {
        cerr << "Eroare la crearea semaforului" << endl;
        return 1;
    }
    if (fork() == 0) {  
        while (*counter <= 1000) {
            sem_wait(sem);            
            if (rand() % 2 == 0) {  
                (*counter)++;
                cout << "Procesul copil a incrementat valoarea la " << *counter << endl;
            } else {
                cout << "Procesul copil nu a incrementat (banul a picat pe 1)" << endl;
            }
            sem_post(sem);
            usleep(100000);
        }
    } 
    else {  
        while (*counter <= 1000) {
            sem_wait(sem);            
            if (rand() % 2 == 0) {  
                (*counter)++;
                cout << "Procesul parinte a incrementat valoarea la " << *counter << endl;
            } else {
                cout << "Procesul parinte nu a incrementat (banul a picat pe 1)" << endl;
            }
            sem_post(sem);
            usleep(100000);
        }
        wait(nullptr);
        sem_close(sem);
        sem_unlink(SEMAPHORE_NAME);
        munmap(counter, sizeof(int));
        shm_unlink(SHARED_MEM_NAME);
    }
    return 0;
}
