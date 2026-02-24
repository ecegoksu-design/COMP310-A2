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
    // the address of the loaded lines (points to the first line in script_lines)
    char(*script_addr)[MAX_USER_INPUT];
    // index in scripts
    int script_idx;
    // number of lines/instructions
    int length;
    // the index to the instruction in the lines array
    int instruction_idx;
    // the next PCB in the ready queue
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
    // Stores the actual scripts pointers
    struct SCRIPT_PCB *scripts[3];

    /* Stores lines of running scripts
     * May need to adjust for A3 */
    char script_lines[MEM_SIZE][MAX_USER_INPUT];
};


/*** Shell Memories ***/
struct memory_struct shellmemory[MEM_SIZE];

struct source_struct sourcememory;

/********/

// Helper functions
void rq_enqueue(struct SCRIPT_PCB *script);

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
            sourcememory.script_lines[i][0] = '\0'; // Might want to initialize the entire array in case of errors
        }
    }
    // Initialize the array for script pointers
    for (i = 0; i<3; i++)
        sourcememory.scripts[i] = 0;
}

int make_script_pcb(FILE *p){
    // Creates a new SCRIPT PCB
    //TODO: STILL DON'T KNOW HOW TO ASSIGN PID!!

    int i;
    struct SCRIPT_PCB *newpcb = malloc(sizeof(struct SCRIPT_PCB));
    for (i = 0; i < 3; i++) {
        if(sourcememory.scripts[i] == 0){
            sourcememory.scripts[i] = newpcb;
            break;
        }        
    } if (i == 3) exit(-1); // Shouldn't be more than 3 files.
    
    newpcb->script_idx = i;
    newpcb->instruction_idx = 0;

    // Find the first free line in script_lines
    int x=0;
    while(sourcememory.script_lines[x][0] != '\0') x++;
    newpcb->script_addr = &sourcememory.script_lines[x];
    const int starting_line = x;

    // Start loading the lines of the script into shell memory
    char line[MAX_USER_INPUT];
    while(NULL != fgets(line, MAX_USER_INPUT - 1, p)){
        memcpy(sourcememory.script_lines[x++], line, strlen(line)+1); // copy line
        memset(line, 0, sizeof(line));
    }

    newpcb->length = x - starting_line;
    rq_enqueue(newpcb);

    return newpcb->script_idx;
}

char *get_script_line(int index, int i) {
    // returns the instructions at the ith line of script at the given index.
    // Assigns insruction index of PCB, probably a better way to do this...
    // returns NULL if ith line does not exist.

    struct SCRIPT_PCB *script = sourcememory.scripts[index];
    script->instruction_idx = i;
    
    if (i > script->length) return NULL;
    return script->script_addr[i]; // ith line after starting address
}

// Might need a ready queue reset function too ...
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

    script->next = NULL;
    rq.size++;
}

int get_pcb_length(int pcb_indx) {
    return sourcememory.scripts[pcb_indx]->length;
}

void reset_scripts(){
    int script_idx = 0;

    while(script_idx < MEM_SIZE){
        for (int j = 0; j < MAX_USER_INPUT; j++)
            sourcememory.script_lines[script_idx][j] = '\0';
        
        if (!sourcememory.script_lines[++script_idx][0]) break; // Reached the end of the last script
    }
    for (int i = 0; i<3; i++) {
        free(sourcememory.scripts[i]);
        sourcememory.scripts[i] = 0;
    }
}

int rq_get_head_indx(){
    return rq.head->script_idx;
}

int rq_get_tail_indx(){
    return rq.tail->script_idx;
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
