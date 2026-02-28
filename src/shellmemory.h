#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H

#include <stdio.h>
#include "shell.h"   
#define MEM_SIZE 1000
#define MAX_SCRIPTS 10

struct SCRIPT_PCB {
    int PID;
    // the address of the loaded lines (points to the first line in script_lines)
    char (*script_addr)[MAX_USER_INPUT];
    // index in scripts
    int script_idx;
    // number of lines/instructions
    int length;
    // the index to the instruction in the lines array
    int instruction_idx;
    int job_score;
    // the next PCB in the ready queue
    struct SCRIPT_PCB* next;
};

// ReadyQueue data structure
struct ReadyQueue {
    struct SCRIPT_PCB* head;
    struct SCRIPT_PCB* tail;
    int size;
};

// Data structure for SOURCE command
struct source_struct {
    // Stores the actual scripts pointers
    struct SCRIPT_PCB *scripts[MAX_SCRIPTS];
    
    char script_lines[MEM_SIZE][MAX_USER_INPUT];
};

typedef enum { FCFS, SJF, RR, AGING, RR30 } Policy;

extern struct ReadyQueue rq;
extern struct source_struct sourcememory;

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
void reset_scripts();
int make_script_pcb(FILE *p);
int get_pcb_length(int pcb_indx);
char *get_script_line(int index, int i);
int rq_get_head_indx();
int rq_get_tail_indx();
int scheduler_FCFS();

struct SCRIPT_PCB* allocate_script(FILE *f);
void rq_enqueue_sorted(struct SCRIPT_PCB *pcb);
void scheduler(Policy policy);
struct SCRIPT_PCB* rq_dequeue(void);
void rq_enqueue(struct SCRIPT_PCB *script);   
void scheduler_RR(int time_slice, short multithreaded);
void rq_enqueue_by_score(struct SCRIPT_PCB *pcb, short init);
void scheduler_AGING(void);

#endif