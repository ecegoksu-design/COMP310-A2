#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H

#include <stdio.h>
#include "shell.h"

#define PAGE_SIZE 3
#define MAX_SCRIPTS 10

#ifndef MEM_SIZE
#define MEM_SIZE 10
#endif

#ifndef FRAME_STORE_SIZE
#define FRAME_STORE_SIZE 18
#endif

#define MAX_FRAMES (FRAME_STORE_SIZE / PAGE_SIZE)

struct FrameEntry {
    char lines[PAGE_SIZE][MAX_USER_INPUT];
    int  owner_script; 
    int  logical_page;  
    int  lru_time;      
};

struct SCRIPT_PCB {
    int  PID;
    // index in scripts
    int  script_idx;  
    // number of lines/instructions
    int  length;             
    // the index to the instruction in the lines array
    int  instruction_idx;     
    // for AGING scheduler
    int  job_score;           
    int  num_pages;          
    int *page_table;          
    char (*backing_store)[MAX_USER_INPUT]; 
    // the next PCB in the ready queue
    struct SCRIPT_PCB *next;
};

// ReadyQueue data structure
struct ReadyQueue {
    struct SCRIPT_PCB *head;
    struct SCRIPT_PCB *tail;
    int size;
};

typedef enum { FCFS, SJF, RR, AGING, RR30 } Policy;

extern struct ReadyQueue  rq;
extern struct FrameEntry  frame_store[MAX_FRAMES];
extern struct SCRIPT_PCB *scripts[MAX_SCRIPTS];
extern int lru_clock;

void  mem_init(void);
char *mem_get_value(char *var);
void  mem_set_value(char *var, char *value);

struct SCRIPT_PCB *allocate_script(FILE *f, int pages_to_load);
int  make_script_pcb(FILE *p, int pages_to_load);
void free_pcb(struct SCRIPT_PCB *pcb);
void reset_scripts(void);
int  rq_get_head_indx(void);
int  rq_get_tail_indx(void);

struct SCRIPT_PCB *rq_dequeue(void);
void rq_enqueue(struct SCRIPT_PCB *script);
void rq_enqueue_sorted(struct SCRIPT_PCB *pcb);
void rq_enqueue_by_score(struct SCRIPT_PCB *pcb, short init);
int  scheduler_FCFS(void);
void scheduler_RR(int time_slice, short multithreaded);
void scheduler_AGING(void);
int  find_free_frame(void);
int  find_lru_frame(void);
void load_page_to_frame(struct SCRIPT_PCB *pcb, int logical_page, int frame_idx);
int  ensure_page_in_store(struct SCRIPT_PCB *pcb, int logical_page);

#endif
