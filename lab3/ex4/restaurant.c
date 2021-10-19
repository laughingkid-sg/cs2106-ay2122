#include "restaurant.h"
#include <semaphore.h>
#include "stdlib.h"
#include <stdio.h>

// You can declare global variables here

//LinkedList for queueing customers
typedef struct NODE {
    sem_t available;
    struct NODE *next;
    int pax;
} node;

typedef struct {
    node *start;
    node *end;
} list;

// Control Entrance 
sem_t mutexEntrance;

// Store tables avalability
int *tables[5]; // to be malloc to 2d array
int tableCount[5]; // store number of tables for a table size

list queue;

void restaurant_init(int num_tables[5]) {
    // Write initialization code here (called once at the start of the program).
    // It is guaranteed that num_tables is an array of length 5.
    // TODO

    for (int i = 0; i < 5; i++) {
        tables[i] = (int *)calloc(num_tables[i], sizeof(int)); // default = 0, then 0 = available, 1 = taken
        tableCount[i] = num_tables[i];
    }

    sem_init(&mutexEntrance, 0, 1);

}

void restaurant_destroy(void) {
    // Write deinitialization code here (called once at the end of the program).
    // TODO
    for (int i = 0; i < 5; i++) {
        free(tables[i]);
    }
    if (queue.start != NULL) {
        node *curr = queue.start;
        while (curr != NULL) {
            sem_destroy(&(curr->available));
            node *temp = curr;
            curr = curr->next;
            free(temp);
        }
    }
    sem_destroy(&mutexEntrance);
}

int request_for_table(group_state *state, int num_people) {
    // Write your code here.
    // Return the id of the table you want this group to sit at.
    // TODO

    // As required
    on_enqueue();
    int tableAvailable = 0;
    num_people--;

    // Check if there are tables
    // If have let then in, else add to queue
    sem_wait(&mutexEntrance);
    int tableID = 0;
    //printf("a: %d; b: %d \n", tables[2][0], tables[2][1]);
    for (int i = 0; i < tableCount[num_people]; i++) {
        //printf("num_people: %d; i: %d, canseat: %d \n", num_people, i, tables[num_people][i]);
        if (tables[num_people][i] == 0) {
            //printf("num_people: %d; i: %d \n", num_people, i);
            tableAvailable = 1; // Update that table is available for this thread
            tables[num_people][i] = 1; // update table taken in shared array
            if (num_people == 0) 
                tableID = i;
            else {
                for (int k = 0; k < num_people; k++) {
                    tableID += tableCount[k]; // add up table count for all tables sizes before current
                }
                tableID += i; // auto off set 
            }
            
            break;
        }
    }
    // table available
    if (tableAvailable) {
        state->tableID = tableID;
        state->pax = num_people;
        sem_post(&mutexEntrance);
    } else {
        node *newGroup = (node *)malloc(sizeof(node));
        sem_t available;
        sem_init(&available, 0, 0);
        newGroup->available = available;
        newGroup->next = NULL;
        newGroup->pax = num_people;

        if (queue.start == NULL) /* Queue is empty */ {
            queue.start = newGroup;
            queue.end = newGroup;
        } else /* Add to end of queue */ {
            queue.end->next = newGroup;
            queue.end = newGroup;
        }

        sem_post(&mutexEntrance);
        // FML GOT REKTED HERE
        sem_wait(&(newGroup->available)); // block untl table exist

        // some group leave and there's slot 
        sem_wait(&mutexEntrance);
        sem_destroy(&available);
        free(newGroup);
        // printf("arr: %d, arr: %d, x: %d \n", num_people, 1, tables[num_people][1]);
        for (int i = 0; i < tableCount[num_people]; i++) {
            
            if (tables[num_people][i] == 0) {
                //printf("people: %d, i: %d", num_people, i);
                tableAvailable = 1; // Update that table is available for this thread
                tables[num_people][i] = 1; // update table taken in shared array
                if (num_people == 0) 
                    tableID = i;
                else {
                    for (int k = 0; k < num_people; k++) {
                        tableID += tableCount[k]; // add up table count for all tables sizes before current
                    }
                    tableID += i; // auto off set 
                }
                break;
            }
        }

        state->tableID = tableID;
        state->pax = num_people;
        sem_post(&mutexEntrance);
        
    }
    
    
    return tableID;
}

void leave_table(group_state *state) {
    // Write your code here.
    // TODO
    sem_wait(&mutexEntrance);
    // Update the 2d array make it avaible
    int i = state->tableID;
    int k = 0; // looping through different sizes array

    /**
     * Navigation within 2d Array T.T 
    */
    while (i >= tableCount[k]) /* Getting to the correct array */ {
        i = i - tableCount[k]; //
        k++; // Increase to next table size
        //printf("counter: %d \n", i);
    }
    // printf("k: %d \n", k);
    tables[k][i] = 0; // set table to avaible
    //printf("k: %d, i: %d, z: %d \n", k, i, tables[k][i]);

    // handle quueuue -> dequeue & post
    node *curr = queue.start;
    node *prev = NULL;
    
    while (curr != NULL) {
        if (curr->pax == state->pax) /* there exisit in queue that is same size as the one just freed*/ {
            
            // if is head // POP from Queue (LL)
            if (prev == NULL) {
                if (curr->next == NULL) /* only item*/ {
                    queue.end = NULL;
                    queue.start = NULL;
                } else /*not only item*/ {
                    queue.start = curr->next; // change head
                    curr->next = NULL; // rest pointer extra
                }
            } else if (curr->next == NULL) /*TAIL*/ {
                queue.end = prev;
                prev->next = NULL;
            } else {
                prev->next = curr->next;
                curr->next = NULL;
            }
            // printf("result: %d \n", sem_post(&(curr->available)));
            sem_post(&(curr->available)); // post is OKIE, Tested
            break;
        }
        prev = curr;
        curr = curr->next;
    }
    sem_post(&mutexEntrance);

}