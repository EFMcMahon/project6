#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TARGET_SUM 19

void print_help() {
    printf("Usage: ./mathwait num1 num2 ...\n");
    printf("This program finds a pair of numbers that sum to 19 using memory shared between a parent and child process.\n");
}

int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        print_help();
        return EXIT_SUCCESS;
    }

    int shm_id = shmget(IPC_PRIVATE, 2 * sizeof(int), 0644 | IPC_CREAT);
    if (shm_id < 0) {
        perror("shmget failed");
        return EXIT_FAILURE;
    }

    int *shm_ptr = (int *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (int *) -1) {
        perror("shmat failed");
        return EXIT_FAILURE;
    }

    shm_ptr[0] = -2;
    shm_ptr[1] = -2;

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        int found = 0;
        for (int i = 1; i < argc && !found; i++) {
            for (int j = i + 1; j < argc; j++) {
                if (atoi(argv[i]) + atoi(argv[j]) == TARGET_SUM) {
                    shm_ptr[0] = atoi(argv[i]);
                    shm_ptr[1] = atoi(argv[j]);
                    found = 1;
                    break;
                }
            }
        }

        if (!found) {
            shm_ptr[0] = -1;
            shm_ptr[1] = -1;
        }

        shmdt(shm_ptr);
        exit(EXIT_SUCCESS);
    }

    wait(NULL);
    if (shm_ptr[0] == -2 && shm_ptr[1] == -2) {
        printf("Shared memory was not updated by the child process.\n");
    } else if (shm_ptr[0] == -1 && shm_ptr[1] == -1) {
        printf("No pair found.\n");
    } else {
        printf("Pair found by child: %d %d\n", shm_ptr[0], shm_ptr[1]);
    }

    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, NULL);
    return EXIT_SUCCESS;
}
