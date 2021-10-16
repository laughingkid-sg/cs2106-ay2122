#include "packer.h"
#include <semaphore.h>
#include "stdlib.h"

// You can declare global variables here
sem_t mutex, box[3];
int *boxes[3];

void packer_init(void) {
    // Write initialization code here (called once at the start of the program).
    sem_init(&mutex, 0, 1);
    for (int i = 0; i < 3; i++) {
        sem_init(&box[i], 0, 0);
        boxes[i] = malloc(sizeof(int) * 2);
    }

}

void packer_destroy(void) {
    // Write deinitialization code here (called once at the end of the program).
    sem_destroy(&mutex);
    for (int i = 0; i < 3; i++) {
        sem_destroy(&box[i]);
        free(boxes[i]);
    }
}

int pack_ball(int colour, int id) {
    // Write your code here.
    colour--;
    sem_wait(&mutex); // block everything
    if (boxes[colour][0] == 0) { // First ball
        boxes[colour][0] = id; // Save ID
        sem_post(&mutex); // Free Main Mutex
        sem_wait(&box[colour]); // Enter mutex
        return boxes[colour][1];
    } else {
        boxes[colour][1] = id;
        sem_post(&mutex);
        sem_post(&box[colour]);
        return boxes[colour][0];
    }
    return 0;
}