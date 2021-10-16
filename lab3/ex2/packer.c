#include "packer.h"
#include <semaphore.h>
#include "stdlib.h"

// You can declare global variables here
int N = 2;

int *boxes[3];
int count[3];

sem_t boxNotFull[3];
sem_t boxNotEmpty[3];
sem_t mutex[3];

void packer_init(void) {
    // Write initialization code here (called once at the start of the program).
    for (int i = 0; i < 3; i++) {
        sem_init(&boxNotFull[i], 0, 1);
        sem_init(&boxNotEmpty[i], 0, 0);
        sem_init(&mutex[i], 0, 1);
        boxes[i] = malloc(sizeof(int) * 2);
    }
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

int pack_ball(int colour, int id) {
    // Write your code here.
    int result;
    colour--;

    sem_wait(&boxNotFull[colour]);
    sem_wait(&mutex[colour]);
    
    boxes[colour][count[colour]] = id;
    count[colour]++;

    // Box is full
    if (count[colour] == 2) {
        result = boxes[colour][0];
        count[colour]--; // back to zero, 2 -> 0
        sem_post(&boxNotEmpty[colour]);
        sem_post(&mutex[colour]);
    } else if (count[colour] == 1) {
        sem_post(&mutex[colour]);
        sem_post(&boxNotFull[colour]); // end of critical section assigning first ball
        sem_wait(&boxNotEmpty[colour]); // block until next ball
        result = boxes[colour][1];
        count[colour]--; // back to zero, 2 -> 0
        sem_post(&boxNotFull[colour]); // restore to init
    }

    return result;
   
}