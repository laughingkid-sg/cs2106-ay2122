#include "packer.h"
#include <semaphore.h>
#include "stdlib.h"

void populateArr(int id, int *other_ids, int box[]);

// You can declare global variables here
int N;

int *boxes[3];
int count[3];

sem_t boxNotFull[3];
sem_t boxNotEmpty[3];
sem_t mutex[3];

void packer_init(int balls_per_pack) {
    // Write initialization code here (called once at the start of the program).
    // It is guaranteed that balls_per_pack >= 2.
    for (int i = 0; i < 3; i++) {
            sem_init(&boxNotFull[i], 0, 1);
            sem_init(&boxNotEmpty[i], 0, 0);
            sem_init(&mutex[i], 0, 1);
            boxes[i] = malloc(sizeof(int) * balls_per_pack);
    }
    N = balls_per_pack;
}

void packer_destroy(void) {
    // Write deinitialization code here (called once at the end of the program).
    for (int i = 0; i < 3; i++) {
        sem_destroy(&boxNotFull[i]);
        sem_destroy(&boxNotEmpty[i]);
        sem_destroy(&mutex[i]);
        free(boxes[i]);
    }
}

void pack_ball(int colour, int id, int *other_ids) {
    // Write your code here.
    // Remember to populate the array `other_ids` with the (balls_per_pack-1) other balls.
    colour--;
    sem_wait(&boxNotFull[colour]);                                          //--- SYNC 2
    sem_wait(&mutex[colour]);                                               //--- SYNC 1

    boxes[colour][count[colour]] = id;
    count[colour]++;

    if (count[colour] == N) { // Box is FULL
        populateArr(id, other_ids, boxes[colour]);
        count[colour]--; // back to zero, N -> 0

        for (int i = 0; i < N - 1; i++) {
            sem_post(&boxNotEmpty[colour]); // unblock everything           //--- SYNC 3
        }
        sem_post(&mutex[colour]);                                           //--- SYNC 1

    } else if (count[colour] > 0) { // balls coming in from 1 to N 
        sem_post(&mutex[colour]);                                           //--- SYNC 1
        sem_post(&boxNotFull[colour]); // unblock crtitical section         //--- SYNC 2

        sem_wait(&boxNotEmpty[colour]); // block till box is full           //--- SYNC 3
        populateArr(id, other_ids, boxes[colour]);
        count[colour]--; // back to zero, N -> 0
    }

    if (count[colour] == 1) // Reset
        sem_post(&boxNotFull[colour]);                                      //--- SYNC 2
    
}

void populateArr(int id, int *other_ids, int box[]) {
    int k = 0;
    for (int i = 0; i < N; i++) {
        if (id != box[i])
            other_ids[k++] = box[i];
    }
}