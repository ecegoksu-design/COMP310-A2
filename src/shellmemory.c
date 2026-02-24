#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"
#include "shell.h"

struct memory_struct {
    char *var;
    char *value;
};

struct SCRIPT_PCB{
int PID;
// the address of the loaded lines (points to the first line)
char(*script_addr)[MAX_USER_INPUT];
// number of lines/instructions
int length;
// the index to the instruction in the lines array
int instruction_idx;
//the next PCB in the ready queue
struct SOURCE_PCB* next;
};

struct memory_struct shellmemory[MEM_SIZE];

// For SOURCE command, stores lines of a running script
// May need to adjust for A3
char scripts[MEM_SIZE][MAX_USER_INPUT];

// ReadyQueue data structure
struct ReadyQueue {
    struct SOURCE_PCB* head;
    struct SOURCE_PCB* tail; // tail could be identified by checking pcb.next == NULL;
    int size;
}; 


// Helper functions
int match(char *model, char *var) {
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i]) matchCount++;
    }
    if (matchCount == len) {
        return 1;
    } else return 0;
}

// Shell memory functions

void mem_init(){
    int i;
    for (i = 0; i < MEM_SIZE; i++){		
        shellmemory[i].var   = "none";
        shellmemory[i].value = "none";
        // Initialize scripts array
        for (int j = 0; j < MAX_USER_INPUT; j++)
            scripts[i][j] = '\0';
    }
}


void reset_scripts(){
    int script_idx = 0;

    while(script_idx < MEM_SIZE){
        for (int j = 0; j < MAX_USER_INPUT; j++)
            scripts[script_idx][j] = 0;
        
        if (!scripts[++script_idx][0]) break; // Reached the end of the last script
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++){
        if (strcmp(shellmemory[i].var, var_in) == 0){
            shellmemory[i].value = strdup(value_in);
            return;
        } 
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++){
        if (strcmp(shellmemory[i].var, "none") == 0){
            shellmemory[i].var   = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return;
        } 
    }

    return;
}

//get value based on input key
char *mem_get_value(char *var_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++){
        if (strcmp(shellmemory[i].var, var_in) == 0){
            return strdup(shellmemory[i].value);
        } 
    }
    return "Variable does not exist";
}