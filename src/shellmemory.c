#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"
#include "shell.h"

/*** structs ***/
struct memory_struct {
    char *var;
    char *value;
};

struct SCRIPT_PCB{
    int PID;
    // the address of the loaded lines (points to the first line in scipt_lines)
    char(*script_addr)[MAX_USER_INPUT];
    // index in
    int script_idx;
    // number of lines/instructions
    int length;
    // the index to the instruction in the lines array
    int instruction_idx;
    //the next PCB in the ready queue
    struct SCRIPT_PCB* next;
};

// ReadyQueue data structure
struct ReadyQueue {
    struct SCRIPT_PCB* head;
    struct SCRIPT_PCB* tail; // tail could also be identified by checking pcb.next == NULL;
    int size;
} rq;

// Data structure for SOURCE command
struct source_struct{
    // Stores the actual scripts
    struct SCRIPT_PCB *scripts[3];

    // Stores lines of a running scripts
    // May need to adjust for A3
    char script_lines[MEM_SIZE][MAX_USER_INPUT];
};


/*** Shell Memories ***/
struct memory_struct shellmemory[MEM_SIZE];

struct source_struct sourcememory;

/********/

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
        // Initialize script_lines array
        for (int j = 0; j < MAX_USER_INPUT; j++) {
            sourcememory.script_lines[i][j] = '\0';
        }
    }
    // Initialize scripts
    for (i = 0; i<3; i++)
        sourcememory.scripts[i] = 0;
}

struct SCRIPT_PCB *make_script_pcb(FILE *p){
    // Creates a new SCRIPT PCB
    struct SCRIPT_PCB new;
    for (int i = 0; i < 3; i++) {

    }

    return &new;
}
void rq_enqueue(struct SCRIPT_PCB *script){
    // Adds script to the tail of ReadyQueue rq
    if (rq.size==0) {
        rq.head = script;
        rq.tail = rq.head;
    }
    else {
    rq.tail->next = script;
    rq.tail = script;
    }

    rq.size++;
}

void reset_scripts(){
    int script_idx = 0;

    while(script_idx < MEM_SIZE){
        for (int j = 0; j < MAX_USER_INPUT; j++)
            sourcememory.script_lines[script_idx][j] = 0;
        
        if (!sourcememory.script_lines[++script_idx][0]) break; // Reached the end of the last script
    }
}

struct SCRIPT_PCB *rq_get_head(){
    return rq.head;
}

struct SCRIPT_PCB *rq_get_tail(){
    return rq.tail;
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