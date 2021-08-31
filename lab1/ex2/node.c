/*************************************
* Lab 1 Exercise 2
* Name: Goh Zheng Teck
* Student No: 
* Lab Group: 08
*************************************/

#include "node.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Add in your implementation below to the respective functions
// Feel free to add any headers you deem fit (although you do not need to)

// Inserts a new node with data value at index (counting from head
// starting at 0).
// Note: index is guaranteed to be valid.
void insert_node_at(list *lst, int index, int data) {

    // Creates a new Note & set the data
    node *nn;
    nn = (node*) malloc(sizeof(node));
    nn -> data = data;

    // If list is empty
    if (lst -> head == NULL) {

        // Set head and next to the new node
        nn -> next = nn;
        lst -> head = nn;

    } else {

        // Set current to first node in list
        node *current = lst -> head;
        
        // Base case when index is 0
        if (index == 0) {

            // Set next of new node to first node in list
            nn -> next = lst -> head;

            // Run through list to get to last node
            while (current -> next != lst -> head) 
                current = current -> next;
            
            // Point list to new node
            lst -> head = nn;

        } else {

            // Find the node before the index in list
            for (int i = 1; i != index; i++) 
                current = current -> next;       

            // Sets the next node of new node
            if (current -> next == lst -> head) 
                nn -> next = lst -> head;
            else 
                nn -> next = current -> next;   
        }

        // Set the next of current node to the new node
        current -> next = nn;

    }
    
}

// Deletes node at index (counting from head starting from 0).
// Note: index is guarenteed to be valid.
void delete_node_at(list *lst, int index) {

    if (lst -> head == NULL) {
        return;
    }

    // Set current to first node in list
    node *current = lst -> head;

    if (current == current -> next) {
        free(lst -> head);
        lst -> head = NULL;
    } else {
        
        if (index == 0) {

            // Set temp to first node
            node *temp = lst -> head;

            // Set head pointer to second node
            lst -> head = current -> next;

            // Find the last node in list 
            while (current -> next != temp)
                current = current -> next;
            
            // Set last node to point to first node
            current -> next = lst -> head;

            // Free memory
            free(temp);
            
        } else {

            // Find the node before the index in list
            for (int i = 1; i != index; i++)
                current = current -> next;

            // Set temp to the node to be removed
            node *temp = current -> next; 

            // Set the next node of current to the next node of temp
            current -> next = temp -> next;   

            // Free memory
            free(temp);

        }
    }
}

// Rotates list by the given offset.
// Note: offset is guarenteed to be non-negative.
void rotate_list(list *lst, int offset) {
    if (lst -> head == NULL) {
        return;
    }

    // Set current to first node in list
    node *current = lst -> head;

    // Set current the off set node
    for (int i = 0; i < offset; i++)
        current  = current -> next;

    // Set list head to point to current
    lst -> head = current;

}

// Reverses the list, with the original "tail" node
// becoming the new head node.
void reverse_list(list *lst) {
    if (lst -> head == NULL) {
        return;
    }
    // Set the  prev, current and next nodes
    node *prev = NULL;
    node *current = lst -> head;
    node *next = NULL;

    // Reverse until last node is first node
    while (current -> next != lst -> head) {
        // Store next node
        next = current -> next;

        // Reverse current node's pointer
        current -> next = prev;

        // Move pointers one position ahead
        prev = current;
        current = next;
    }

    // Store first node
    next = current -> next;

    // Reverse current node's pointer
    current -> next = prev;

    // Reverse first node's pointer
    next -> next = current;

    // Set list head's pointer
    lst -> head = current;

}

// Resets list to an empty state (no nodes) and frees
// any allocated memory in the process
void reset_list(list *lst) {
    if (lst -> head == NULL) {
        return;
    }
    node* fn = lst -> head;
    if (fn -> next != fn) {
        node* current = fn -> next;

        // Free nodes by freeing node and the next
        while (current != fn) {
            node* next = current->next;
            free(current);
            current = next;
        }
        
    }
    // Free up memory
    free(fn);
    // Set list head's pointer to null
    lst -> head = NULL;

}
