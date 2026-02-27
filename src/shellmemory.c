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

/*** Shell Memories ***/
int PID_COUNTER = 0;
struct memory_struct shellmemory[MEM_SIZE];
struct source_struct sourcememory;
struct ReadyQueue rq;
/********/

// Schedulers
int scheduler_FCFS() {
    while (rq.size > 0) {
        struct SCRIPT_PCB* proc = rq_dequeue();
        if (!proc) break;

        while (proc->instruction_idx < proc->length) {
            char* instr = proc->script_addr[proc->instruction_idx++];
            parseInput(instr);
        }

        // Clean up
        for (int j = 0; j < proc->length; j++)
            proc->script_addr[j][0] = '\0';
        sourcememory.scripts[proc->script_idx] = NULL;
        free(proc);
    }
    return 0;
}

void scheduler_RR(int time_slice) {
    while (rq.head != NULL) {
        struct SCRIPT_PCB *current = rq_dequeue();
        int executed = 0;
        while (executed < time_slice && current->instruction_idx < current->length) {
            char *instr = current->script_addr[current->instruction_idx++];
            parseInput(instr);
            executed++;
        }
        if (current->instruction_idx < current->length) {
            rq_enqueue(current);
        } else {
            for (int j = 0; j < current->length; j++)
                current->script_addr[j][0] = '\0';
            sourcememory.scripts[current->script_idx] = NULL;
            free(current);
        }
    }
}

void scheduler_AGING(void) {
    while (rq.head != NULL) {
        struct SCRIPT_PCB *current = rq_dequeue();

        if (current->instruction_idx < current->length) {
            char *instr = current->script_addr[current->instruction_idx++];
            parseInput(instr);
        }

        if (current->instruction_idx >= current->length) {
            for (int j = 0; j < current->length; j++)
                current->script_addr[j][0] = '\0';
            sourcememory.scripts[current->script_idx] = NULL;
            free(current);
            continue;
        }

        struct SCRIPT_PCB *temp = rq.head;
        while (temp != NULL) {
            if (temp->job_score > 0)
                temp->job_score--;
            temp = temp->next;
        }

        rq_enqueue_by_score(current);
    }
}

// ReadyQueue Functions
 void rq_enqueue(struct SCRIPT_PCB *script){
    if (rq.size == 0) {
        rq.head = script;
        rq.tail = rq.head;
    } else {
        rq.tail->next = script;
        rq.tail = script;
    }
    script->next = NULL;
    rq.size++;
}

void rq_enqueue_sorted(struct SCRIPT_PCB* pcb){
    if (rq.head == NULL || pcb->length < rq.head->length) {
        pcb->next = rq.head;
        rq.head = pcb;
        if (rq.tail == NULL) rq.tail = pcb;
    }
    else {
        struct SCRIPT_PCB* prev = rq.head;
        while (prev->next != NULL && prev->next->length <= pcb->length) {
            prev = prev->next;
        }
        pcb->next = prev->next;
        prev->next = pcb;
        if (pcb->next == NULL) rq.tail = pcb;
    }
    rq.size++;
}

void rq_enqueue_by_score(struct SCRIPT_PCB *pcb) {
    pcb->next = NULL;

    if (rq.head == NULL || pcb->job_score <= rq.head->job_score) {
        pcb->next = rq.head;
        rq.head = pcb;
        if (rq.tail == NULL) rq.tail = pcb;
        rq.size++;
        return;
    }

    struct SCRIPT_PCB *prev = rq.head;

    while (prev->next != NULL && prev->next->job_score < pcb->job_score) {
        prev = prev->next;
    }

    pcb->next = prev->next;
    prev->next = pcb;
    if (pcb->next == NULL) rq.tail = pcb;
    rq.size++;
}

struct SCRIPT_PCB* rq_dequeue() {
    if (rq.size == 0 || rq.head == NULL) return NULL;

    struct SCRIPT_PCB* proc = rq.head;
    rq.head = proc->next;
    rq.size--;

    if (rq.head == NULL) rq.tail = NULL;

    proc->next = NULL;
    return proc;
}

int rq_get_head_indx(){
    return (rq.head) ? rq.head->script_idx : -1;
}

int rq_get_tail_indx(){
    return (rq.tail) ? rq.tail->script_idx : -1;
}



// Scripts
 int make_script_pcb(FILE *p) {
    // Creates a new SCRIPT PCB
    struct SCRIPT_PCB *pcb = allocate_script(p);
    if (!pcb) return -1;
    rq_enqueue(pcb);
    return pcb->script_idx;
}

struct SCRIPT_PCB* allocate_script(FILE* f){
    int slot = -1;
    for (int i = 0; i < 3; i++) {
        if (sourcememory.scripts[i] == NULL) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return NULL;

    struct SCRIPT_PCB *pcb = malloc(sizeof(struct SCRIPT_PCB));
    if (!pcb) return NULL;
    sourcememory.scripts[slot] = pcb;

    int start = 0;
    while (start < MEM_SIZE && sourcememory.script_lines[start][0] != '\0') start++;
    if (start >= MEM_SIZE) {
        free(pcb);
        sourcememory.scripts[slot] = NULL;
        return NULL;
    }

    pcb->PID = PID_COUNTER++;
    pcb->script_addr = &sourcememory.script_lines[start];
    pcb->script_idx = slot;
    pcb->instruction_idx = 0;
    pcb->next = NULL;
    pcb->length = 0;

    int line = start;
    char buffer[MAX_USER_INPUT];
    while (fgets(buffer, MAX_USER_INPUT-1, f) != NULL) {
        strcpy(sourcememory.script_lines[line], buffer);
        line++;
        pcb->length++;
        if (line >= MEM_SIZE) {
            for (int l = start; l < line; l++) sourcememory.script_lines[l][0] = '\0';
            free(pcb);
            sourcememory.scripts[slot] = NULL;
            return NULL;
        }
    }
    pcb->job_score = pcb->length;
    return pcb;
}

char *get_script_line(int index, int i) {
    struct SCRIPT_PCB *script = sourcememory.scripts[index];
    if (!script || i >= script->length) return NULL;
    script->instruction_idx = i;
    return script->script_addr[i];
}

int get_pcb_length(int pcb_indx) {
    return sourcememory.scripts[pcb_indx]->length;
}

// Shell General Memory
void mem_init(){
    rq.head = NULL;
    rq.tail = NULL;
    rq.size = 0;

    for (int i = 0; i < MEM_SIZE; i++) {
        shellmemory[i].var   = "none";
        shellmemory[i].value = "none";
        // Initialize script_lines array
        sourcememory.script_lines[i][0] = '\0';
    }
    // Initialize the array for script pointers
    for (int i = 0; i < 3; i++)
        sourcememory.scripts[i] = NULL;
}

void reset_scripts(){
    int script_idx = 0;
    while (script_idx < MEM_SIZE) {
        sourcememory.script_lines[script_idx][0] = '\0';
        script_idx++;
    }
    for (int i = 0; i < 3; i++) {
        if (sourcememory.scripts[i]) {
            free(sourcememory.scripts[i]);
            sourcememory.scripts[i] = NULL;
        }
    }
}

// Variable memory
void mem_set_value(char *var_in, char *value_in) {
    for (int i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }
    //Value does not exist, need to find a free spot.
    for (int i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, "none") == 0) {
            shellmemory[i].var   = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }
}

char *mem_get_value(char *var_in) {
    for (int i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            return strdup(shellmemory[i].value);
        }
    }
    return "Variable does not exist";
}

int match(char *model, char *var) {
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i]) matchCount++;
    }
    return (matchCount == len) ? 1 : 0;
}