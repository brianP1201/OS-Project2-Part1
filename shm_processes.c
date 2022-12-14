#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

// Brian Paul

void DadProcess(int [], sem_t *);
void ChildProcess(int [], sem_t *, int);
void MomProcess(int [], sem_t *);

int  main(int  argc, char *argv[])
{
     if (argc < 3) {
          printf("usage: shm_proc <parents> <children> (where parent = [1 | 2], children = [N > 0]\n");
          exit(1);
     }

     int parents = atoi(argv[1]);
     if (parents != 1 && parents != 2){
          printf("Number of Parents should be either 1 or 2\n");
          exit(1);
     }

     int children = atoi(argv[2]);
     if (children <= 0) {
          printf("Number of Children should be at least 1\n");
          exit(1);
     }

     int    ShmID;
     int    *ShmPTR;
     int total = children + parents;
     pid_t  pid[total];
     sem_t *mutex;

     ShmID = shmget(IPC_PRIVATE, 4*sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }

     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (*ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }

     ShmPTR[0] = 0;

     if ((mutex = sem_open("banksemaphore", O_CREAT, 0644, 1)) == SEM_FAILED){
          perror("semaphore initilization");
          exit(1);
     }

     int i;
     for(i = 0; i<total; i++){
          pid[i] = fork();
          if(pid[i] < 0){
               printf("fork failed!");
               exit(1);
          } else if (pid[i] == 0) {
               if (i == 0){
                    DadProcess(ShmPTR, mutex);
               } else if (parents == 2 && i == 2){
                    MomProcess(ShmPTR, mutex);
               } else {
                    ChildProcess(ShmPTR, mutex, i);
               }
          exit(0);
          }
     }

     for(i=0; i<total; i++){
          wait(NULL);
     }

     shmdt((void *) ShmPTR);
     shmctl(ShmID, IPC_RMID, NULL);
     
}

void DadProcess(int sharedMem[], sem_t* mutex){
  int acc, randBalance;
  srand(getpid());

  int i;
  for(i = 0; i < 5; i++){
    sleep(rand()%6);
    printf("Dear Old Dad: Attempting to Check Balance\n");
    sem_wait(mutex);
    acc = sharedMem[0];
    if (acc <= 100) {
      randBalance = rand()%101;
      if (randBalance % 2) {
        acc += randBalance;
        printf("Dear old Dad: Deposits $%d / Balance = $%d\n", randBalance, acc);
        sharedMem[0] = acc;
        } else {
          printf("Dear old Dad: Doesn't have any money to give\n");
        }
    } else {
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", acc);
    }
    sem_post(mutex);
  }
}

void ChildProcess(int sharedMem[], sem_t* mutex, int id){
  int acc, randBalance;
  srand(getpid());

  int i;
  for(i = 0; i < 5; i++){
    sleep(rand()%6);
    printf("Poor Student #%d: Attempting to Check Balance\n", id);
    sem_wait(mutex);
    acc = sharedMem[0];
    randBalance = rand() % 51;

    printf("Poor Student needs $%d\n", randBalance);
    if(rand()%2 == 0){
      if (randBalance <= acc){
      acc -= randBalance;
      printf("Poor Student: Withdraws $%d / Balance = $%d\n", randBalance, acc);
      sharedMem[0] = acc;
    } else {
      printf("Poor Student: Not Enough Cash ($%d)\n", acc);
     }   
    }
    sem_post(mutex);
  }
}

void MomProcess(int sharedMem[], sem_t* mutex){
     int acc, randBalance;
     srand(getpid());

     int i;
     for(i=0; i<5; i++){
          sleep(rand()%10);
          printf("Loveable Mom: Attempting to Check Balance\n");
          sem_wait(mutex);
          acc = sharedMem[0];
          if(acc <= 100){
               randBalance = rand()%126;
               acc += randBalance;
               printf("Lovable Mom: Deposits $%d / Balance = $%d\n", randBalance, acc);
               sharedMem[0] = acc;
          } else {
               printf("Lovable Mom: Thinks Student has enough Cash($%d)\n", acc);
          }
          sem_post(mutex);
     }
}