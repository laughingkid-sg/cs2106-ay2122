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
#include <sys/stat.h>
#include <fcntl.h>
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

int cmdRunner(size_t num_tokens, char **tokens, int isChain);
int redirectionHandler(size_t num_tokens, char **tokens, int redirects[]);
void updateStatues();
void printInfo();
process_t* getProcess();

process_t *info[MAX_PROCESSES];
int len;

void my_init(void) {
    // Initialize what you need here
    len = 0;
}

void my_process_command(size_t num_tokens, char **tokens) {
    // Your code here, refer to the lab document for a description of the arguments
    if (!strcmp(tokens[0], "info")) /* Handle info command*/ {
        updateStatues();
        printInfo();
        return;
    } else if (!strcmp(tokens[0], "wait")) /* Handle wait command*/ {
        int exitStatus;
        process_t *process = getProcess((int) atoi(tokens[1]));
        if (process != NULL && process->state == Running) {
            waitpid(process->pID, &exitStatus, 0);
            process->state = Exited;
            process->exitStatus = WEXITSTATUS(exitStatus);
        }
        return;
    } else if (!strcmp(tokens[0], "terminate")) /* Handle terminate command*/ {
        process_t *process = getProcess((int) atoi(tokens[1]));
        if (process != NULL && process->state == Running) {
            // int exitStatus, pID;
            kill(process->pID, SIGTERM);
            process->state = Terminating;
        }
        return;
    } else /* Handle standard command(s)*/ {
        /**
         * i -> counter for while loop tokens
         * isChain -> boolean for chain check used to handle 'Result 7' case
        */
        int i = 0, isChain = 0;
        while (i < ((int)num_tokens) - 2) { // -1 for 0 index, 1 for null for last

            // counter -> Start of sub-token
            int counter = i;

            // Sub-dividing
            while (counter <= ((int) num_tokens - 2) && strcmp(tokens[counter], "&&")) 
                counter++;
            
            // Prepearing sub-tokens
            char **subTokens = malloc(sizeof(char *) * MAX_PROCESSES);
            for (int k = i; k < counter; k++) {
                subTokens[k-i] = malloc(sizeof(char) * MAX_PROCESSES);
                strcpy(subTokens[k-i], tokens[k]);
            }
            subTokens[counter] = NULL;

            // Running 
            if (!cmdRunner(counter - i + 1, subTokens, isChain)) {
                for (int k = i; k < counter; k++) {
                    free(subTokens[k-i]);
                }
                free(subTokens);
                break;
            }

            // Cleaning
            for (int k = i; k < counter; k++) {
                free(subTokens[k-i]);
            }
            free(subTokens);

            //Handle chain commands
            isChain = 1;

            // Increamental 
            i = counter + 1;
        }
        return;
    }
}

void my_quit(void) {
    // Clean up function, called after "quit" is entered as a user command
    // Kill all existing processes before ending app 
    int exitStatus;
    while (len > 0) {
        len--;
        if (info[len]->state != Exited) {
            switch (info[len]->state) {
            case Running:
                kill(info[len]->pID, SIGTERM);
                waitpid(info[len]->pID, &exitStatus, 0);
                break;
            case Terminating:
                waitpid(info[len]->pID, &exitStatus, 0);
                break;
            case Exited:
                break;
            }
        }
        free(info[len]);
    }
    printf("Goodbye!\n");
}

int cmdRunner(size_t num_tokens, char **tokens, int isChain) {
    /**
     * result -> outcome of wait
     * exitStatus -> exitStatus of app
     * isBG -> check if program should be run in Aysnc
     * prgmNotExist -> check if program exist
     * fd -> file descriptor
     * pos -> position of redirect
    */
    int result, exitStatus, isBG = !strcmp(tokens[num_tokens - 2], "&"),
    prgmNotExist = access(tokens[0], F_OK | X_OK), fd, pos;
    int redirects[2]; // Max 2 cmds
    if (prgmNotExist) {
        printf("%s not found\n", tokens[0]);
        return 0;
    }

    /**
     * Check for redirects by looping through tokens
     * Handle redirect by storing their positions
    */
    int hasRedirect = redirectionHandler(num_tokens, tokens, redirects);

    /**
     * Check if file exist else don't spawn child
    */
    if (hasRedirect) {
        for (int i = 0; i < hasRedirect; i++) {
            pos = redirects[i];
            if (!strcmp(tokens[pos], "<")) {
                fd = access(tokens[pos+1], F_OK | R_OK);
                if (fd) {
                    printf("%s does not exist\n", tokens[pos + 1]);
                    return 0;
                }
            }
        }
    }

    int pID = fork();
    process_t *newProcess = (process_t*)malloc(sizeof(process_t));
    switch (pID) {
    case -1: // Fork Failed 
        printf("Error in creating process\n");
        break;
    case 0: // Child Proces
        // Handle some random error not adding NULL
        if (!hasRedirect) {
            tokens[num_tokens - 1] = NULL;
        }
        /**
         * Using open and dup2 to handle redirects
        */
        if (hasRedirect) {
            for (int i = 0; i < hasRedirect; i++) {
                pos = redirects[i];
                if (!strcmp(tokens[pos], "<")) {
                    fd = open(tokens[pos + 1], O_RDONLY, 0444);
                    if (fd != -1) {
                        dup2(fd, fileno(stdin));
                    } else {
                        // Should never reach here but a fail safe
                        printf("%s does not exist\n", tokens[pos + 1]);
                        exit(EXIT_FAILURE);
                        return 0;
                    }
                } else {
                    fd = creat(tokens[pos + 1], 0644);
                    if (fd == -1) {
                        exit(1);
                    }
                    if (!strcmp(tokens[pos], ">")) {
                        tokens[pos] = NULL;
                        dup2(fd, fileno(stdout));
                    } else if (!strcmp(tokens[pos], "2>")) {
                        dup2(fd, fileno(stderr));
                    }
                }
                if (i == 0) {
                    tokens[pos] = NULL;
                }
        }
    }
        if (isBG) // Manage Aysnc 
            tokens[num_tokens - 2] = NULL;
        if (execv(tokens[0], tokens) == -1)
            exit(EXIT_FAILURE);
        break;
    default: // Parent Process
        newProcess->pID = pID;
        if (isBG) {
            result = waitpid(pID, &exitStatus, WNOHANG);
            if (result == -1) {
                printf("%s failed\n", tokens[0]);
                newProcess->state = Exited;
                newProcess->exitStatus = WEXITSTATUS(exitStatus);
                info[len] = newProcess;
                len++;
                return 0;
            } else {
                newProcess->state = Running;
                printf("Child[%d] in background\n", pID);
            }
        } else {
            waitpid(pID, &exitStatus, 0);
            newProcess->state = Exited;
            newProcess->exitStatus = WEXITSTATUS(exitStatus);
            if (WEXITSTATUS(exitStatus) == EXIT_FAILURE || (WEXITSTATUS(exitStatus) != 0 && isChain)) {
                printf("%s failed\n", tokens[0]);
                info[len] = newProcess;
                len++;
                return 0;
            }
        }
        info[len] = newProcess;
        len++;
        break;
    }
    return 1;
}

int redirectionHandler(size_t num_tokens, char **tokens, int redirects[]) {
    int k = 0;
    for (int i = 0; i < (int)num_tokens - 2; i++){
        if (!strcmp(tokens[i], "<") || !strcmp(tokens[i], ">") || !strcmp(tokens[i], "2>")){
            redirects[k++] = i;
        }
    }
    return k;
}

void updateStatues() {
    int exitStatus;
    for (int i = 0; i < len; i++) {
        process_t* process = info[i];
        if (process->state != Exited && waitpid(process->pID, &exitStatus, WNOHANG) > 0) {
            process->state = Exited;
            process->exitStatus = WEXITSTATUS(exitStatus);
        }
    }
}

void printInfo() {
    for (int i = 0; i < len; i++) {
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
    for (int i = 0; i < len; i++) {
        if (info[i]->pID == pID) {
            return info[i];
        }
    }
    return NULL;
}