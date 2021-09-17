/**
 * CS2106 AY21/22 Semester 1 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "myshell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

enum statues {
    Exited, 
    Running, 
    Terminating
};

typedef struct {
	int pID;
    int exitStatus;
	enum statues state;
} process_t;

int cmdCounter(size_t num_tokens, char **tokens);
void updateStatues();
void printInfo();
process_t* getProcess();

process_t *info[MAX_PROCESSES];
int length;

void my_init(void) {
    // Initialize what you need here
    length = 0;
}

void my_process_command(size_t num_tokens, char **tokens) {
    // Your code here, refer to the lab document for a description of the arguments
    if (!strcmp(tokens[0], "info")) {
        updateStatues();
        printInfo();
        return;
    } else if (!strcmp(tokens[0], "wait")) {
        int exitStatus;
        process_t *process = getProcess((int) atoi(tokens[1]));
        if (process != NULL && process->state == Running) {
            waitpid(process->pID, &exitStatus, 0);
            process->state = Exited;
            process->exitStatus = WEXITSTATUS(exitStatus);
        }
        return;
    } else if (!strcmp(tokens[0], "terminate")) {
        process_t *process = getProcess((int) atoi(tokens[1]));
        if (process != NULL && process->state == Running) {
            // int exitStatus, pID;
            kill(process->pID, SIGTERM);
            process->state = Terminating;
        }
        return;
    } else {
        
        int cmdCount = cmdCounter(num_tokens, tokens);

        for (int i = 0; i < cmdCount; i++) {

        }
        int result, exitStatus, isBG = !strcmp(tokens[num_tokens - 2], "&");
        int pID = fork();

        process_t *newProcess = (process_t*)malloc(sizeof(process_t));
        switch (pID) {

        case -1:
            printf("Error in creating process\n");
            break;
        
        case 0:
            if (isBG)
                tokens[num_tokens - 2] = NULL;
            if (execv(tokens[0], tokens) == -1)
                exit(EXIT_FAILURE);
            break;
        default:
            newProcess->pID = pID;

            if (isBG) {
                result = waitpid(pID, &exitStatus, WNOHANG);
                if (result == -1) {
                    printf("%s not found\n", tokens[0]);
                    return;
                }
                else {
                    newProcess->state = Running;
                    printf("Child[%d] in background\n", pID);
                }

            } else {
                waitpid(pID, &exitStatus, 0);
                if (WEXITSTATUS(exitStatus) == EXIT_FAILURE) {
                    printf("%s not found\n", tokens[0]);
                    return;
                } else {
                    newProcess->state = Exited;
                    newProcess->exitStatus = WEXITSTATUS(exitStatus);
                }
            }
            info[length] = newProcess;
            length++;
            break;
        }
    }
}

void my_quit(void) {
    // Clean up function, called after "quit" is entered as a user command
    int exitStatus;
    while (length > 0) {
        length--;
        if (info[length]->state != Exited) {
            switch (info[length]->state) {
            case Running:
                kill(info[length]->pID, SIGTERM);
                waitpid(info[length]->pID, &exitStatus, 0);
                break;
            case Terminating:
                waitpid(info[length]->pID, &exitStatus, 0);
                break;
            case Exited:
                break;
            }
        }
        free(info[length]);
    }
    printf("Goodbye!\n");
}

int cmdCounter(size_t num_tokens, char **tokens) {
    int i = 1;
    for (int i = 0; i < (int)num_tokens - 2; i++) {
        if (!strcmp(tokens[i], "&&"))
            i++;
    }
    return i;
}

void updateStatues() {
    int exitStatus;
    for (int i = 0; i < length; i++) {
        process_t* process = info[i];
        if (process->state != Exited && waitpid(process->pID, &exitStatus, WNOHANG) > 0) {
            process->state = Exited;
            process->exitStatus = WEXITSTATUS(exitStatus);
        }
    }
}

void printInfo() {
    for (int i = 0; i < length; i++) {
        switch (info[i]->state) {
        case 0:
            printf("[%d] Exited %d\n", info[i]->pID, info[i]->exitStatus);
            break;
        case 1:
            printf("[%d] Running\n", info[i]->pID);
            break;
        case 2:
            printf("[%d] Terminating\n", info[i]->pID);
            break;
        }
    }
}

process_t* getProcess(int pID) {
    for (int i = 0; i < length; i++) {
        if (info[i]->pID == pID) {
            return info[i];
        }
    }
    return NULL;
}