#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "shellmemory.h"
#include "shell.h"

static char *fetch_line(struct SCRIPT_PCB *pcb, int line_idx);

/*** structs ***/
static struct {
    char *var;
    char *value;
} shellmemory[MEM_SIZE];

int PID_COUNTER = 0;
struct FrameEntry  frame_store[MAX_FRAMES];
struct SCRIPT_PCB *scripts[MAX_SCRIPTS];
struct ReadyQueue  rq;
int lru_clock  = 0;

// Schedulers
pthread_mutex_t rq_mutex;

int scheduler_FCFS(void) {
    while (rq.size > 0) {
        struct SCRIPT_PCB *proc = rq_dequeue();
        if (!proc) break;

        while (proc->instruction_idx < proc->length) {
            int line_idx = proc->instruction_idx;

            if (proc->page_table[line_idx / PAGE_SIZE] == -1) {
                ensure_page_in_store(proc, line_idx / PAGE_SIZE);
                continue;
            }

            char buf[MAX_USER_INPUT];
            strncpy(buf, fetch_line(proc, line_idx), MAX_USER_INPUT - 1);
            buf[MAX_USER_INPUT - 1] = '\0';
            proc->instruction_idx++;
            parseInput(buf);
        }

        free_pcb(proc);
    }
    return 0;
}

void scheduler_RR(const int time_slice, const short multithreaded) {
    (void)multithreaded;
    while (rq.head != NULL) {
        struct SCRIPT_PCB *current = rq_dequeue();
        int executed = 0;

        while (executed < time_slice && current->instruction_idx < current->length) {
            int line_idx = current->instruction_idx;

            if (current->page_table[line_idx / PAGE_SIZE] == -1) {
                ensure_page_in_store(current, line_idx / PAGE_SIZE);
                break;
            }

            char buf[MAX_USER_INPUT];
            strncpy(buf, fetch_line(current, line_idx), MAX_USER_INPUT - 1);
            buf[MAX_USER_INPUT - 1] = '\0';
            current->instruction_idx++;
            parseInput(buf);
            executed++;
        }

        if (current->instruction_idx < current->length)
            rq_enqueue(current);
        else
            free_pcb(current);
    }
}

void scheduler_AGING(void) {
    while (rq.head != NULL) {
        struct SCRIPT_PCB *current = rq_dequeue();

        if (current->instruction_idx < current->length) {
            int line_idx = current->instruction_idx;

            if (current->page_table[line_idx / PAGE_SIZE] == -1) {
                ensure_page_in_store(current, line_idx / PAGE_SIZE);
            } else {
                char buf[MAX_USER_INPUT];
                strncpy(buf, fetch_line(current, line_idx), MAX_USER_INPUT - 1);
                buf[MAX_USER_INPUT - 1] = '\0';
                current->instruction_idx++;
                parseInput(buf);
            }
        }

        if (current->instruction_idx >= current->length) {
            free_pcb(current);
            continue;
        }

        // AGING
        struct SCRIPT_PCB *temp = rq.head;
        while (temp != NULL) {
            if (temp->job_score > 0) temp->job_score--;
            temp = temp->next;
        }
        rq_enqueue_by_score(current, 0);
    }
}

int find_free_frame(void) {
    for (int i = 0; i < MAX_FRAMES; i++)
        if (frame_store[i].owner_script == -1)
            return i;
    return -1;
}

int find_lru_frame(void) {
    int lru_idx  = 0;
    int min_time = frame_store[0].lru_time;
    for (int i = 1; i < MAX_FRAMES; i++) {
        if (frame_store[i].lru_time < min_time) {
            min_time = frame_store[i].lru_time;
            lru_idx  = i;
        }
    }
    return lru_idx;
}

void load_page_to_frame(struct SCRIPT_PCB *pcb, int logical_page, int frame_idx) {
    int start_line = logical_page * PAGE_SIZE;

    for (int i = 0; i < PAGE_SIZE; i++)
        frame_store[frame_idx].lines[i][0] = '\0';

    for (int i = 0; i < PAGE_SIZE; i++) {
        int src = start_line + i;
        if (src < pcb->length)
            strcpy(frame_store[frame_idx].lines[i], pcb->backing_store[src]);
    }

    frame_store[frame_idx].owner_script = pcb->script_idx;
    frame_store[frame_idx].logical_page = logical_page;
    frame_store[frame_idx].lru_time     = lru_clock++;

    pcb->page_table[logical_page] = frame_idx;
}

static void print_victim(int frame_idx) {
    for (int i = 0; i < PAGE_SIZE; i++) {
        char *line = frame_store[frame_idx].lines[i];
        if (line[0] == '\0') continue;
        printf("%s", line);
        int len = strlen(line);
        if (len > 0 && line[len - 1] != '\n')
            printf("\n");
    }
}

int ensure_page_in_store(struct SCRIPT_PCB *pcb, int logical_page) {
    if (pcb->page_table[logical_page] != -1)
        return 0;   

    int frame = find_free_frame();

    if (frame == -1) {
        frame = find_lru_frame();

        int own    = frame_store[frame].owner_script;
        int own_pg = frame_store[frame].logical_page;

        printf("Page fault! Victim page contents:\n\n");
        print_victim(frame);
        printf("\nEnd of victim page contents.\n");

        if (own != -1 && scripts[own] != NULL)
            scripts[own]->page_table[own_pg] = -1;

    } else {
        printf("Page fault!\n");
    }

    load_page_to_frame(pcb, logical_page, frame);
    return 1;
}

struct SCRIPT_PCB *allocate_script(FILE *f, int pages_to_load) {
    int slot = -1;
    for (int i = 0; i < MAX_SCRIPTS; i++) {
        if (scripts[i] == NULL) { slot = i; break; }
    }
    if (slot == -1) return NULL;

    int capacity = 16;
    char (*lines)[MAX_USER_INPUT] =
        malloc(capacity * sizeof(char[MAX_USER_INPUT]));
    if (!lines) return NULL;

    int length = 0;
    char buf[MAX_USER_INPUT];
    while (fgets(buf, MAX_USER_INPUT - 1, f) != NULL) {
        if (length >= capacity) {
            capacity *= 2;
            char (*tmp)[MAX_USER_INPUT] =
                realloc(lines, capacity * sizeof(char[MAX_USER_INPUT]));
            if (!tmp) { free(lines); return NULL; }
            lines = tmp;
        }
        strcpy(lines[length++], buf);
    }

    if (length == 0) { free(lines); return NULL; }

    {
        char (*tmp)[MAX_USER_INPUT] =
            realloc(lines, length * sizeof(char[MAX_USER_INPUT]));
        if (tmp) lines = tmp;
    }

    struct SCRIPT_PCB *pcb = malloc(sizeof(struct SCRIPT_PCB));
    if (!pcb) { free(lines); return NULL; }

    int num_pages = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    int *pt = malloc(num_pages * sizeof(int));
    if (!pt) { free(lines); free(pcb); return NULL; }

    for (int i = 0; i < num_pages; i++) pt[i] = -1;

    pcb->PID             = PID_COUNTER++;
    pcb->script_idx      = slot;
    pcb->length          = length;
    pcb->instruction_idx = 0;
    pcb->job_score       = length;
    pcb->num_pages       = num_pages;
    pcb->page_table      = pt;
    pcb->backing_store   = lines;
    pcb->next            = NULL;

    scripts[slot] = pcb;

    int loaded = 0;
    for (int pg = 0; pg < num_pages && loaded < pages_to_load; pg++) {
        int frame = find_free_frame();
        if (frame == -1) break;
        load_page_to_frame(pcb, pg, frame);
        loaded++;
    }

    return pcb;
}

// Scripts
int make_script_pcb(FILE *p, int pages_to_load) {
    // Creates a new SCRIPT PCB
    struct SCRIPT_PCB *pcb = allocate_script(p, pages_to_load);
    if (!pcb) return -1;
    rq_enqueue(pcb);
    return pcb->script_idx;
}

void free_pcb(struct SCRIPT_PCB *pcb) {
    scripts[pcb->script_idx] = NULL;
    free(pcb->backing_store);
    free(pcb->page_table);
    free(pcb);
}

void reset_scripts(void) {
    for (int i = 0; i < MAX_SCRIPTS; i++) {
        if (scripts[i]) {
            free(scripts[i]->backing_store);
            free(scripts[i]->page_table);
            free(scripts[i]);
            scripts[i] = NULL;
        }
    }
    for (int i = 0; i < MAX_FRAMES; i++) {
        for (int j = 0; j < PAGE_SIZE; j++)
            frame_store[i].lines[j][0] = '\0';
        frame_store[i].owner_script = -1;
        frame_store[i].logical_page = -1;
        frame_store[i].lru_time     = 0;
    }
    lru_clock = 0;
}

// ReadyQueue Functions
void rq_enqueue(struct SCRIPT_PCB *script) {
    script->next = NULL;
    if (rq.size == 0) {
        rq.head = script;
        rq.tail = script;
    } else {
        rq.tail->next = script;
        rq.tail = script;
    }
    rq.size++;
}

void rq_enqueue_sorted(struct SCRIPT_PCB *pcb) {
    if (rq.head == NULL || pcb->length < rq.head->length) {
        pcb->next = rq.head;
        rq.head   = pcb;
        if (rq.tail == NULL) rq.tail = pcb;
    } else {
        struct SCRIPT_PCB *prev = rq.head;
        while (prev->next != NULL && prev->next->length <= pcb->length)
            prev = prev->next;
        pcb->next  = prev->next;
        prev->next = pcb;
        if (pcb->next == NULL) rq.tail = pcb;
    }
    rq.size++;
}

void rq_enqueue_by_score(struct SCRIPT_PCB *pcb, const short init) {
    // Tie breaking during queue init and management is different for this Scheduler.
    // Pass 1 for init when initializing the read, 0 otherwise

    // If pcb ran last, it runs again in case of tie (becomes head)
    // If the queue is being initialized, pcb goes after the head in case of tie
    if (rq.head == NULL || (!init && pcb->job_score <= rq.head->job_score)) {
        pcb->next = rq.head;
        rq.head   = pcb;
        if (rq.tail == NULL) rq.tail = pcb;
        rq.size++;
        return;
    }
    struct SCRIPT_PCB *prev = rq.head;
    while (prev->next != NULL && prev->next->job_score <= pcb->job_score)
        prev = prev->next;
    // Insert the program in the ReadyQueue after prev
    pcb->next  = prev->next;
    prev->next = pcb;
    if (pcb->next == NULL) rq.tail = pcb;
    rq.size++;
}

struct SCRIPT_PCB *rq_dequeue(void) {
    if (rq.size == 0 || rq.head == NULL) return NULL;
    struct SCRIPT_PCB *proc = rq.head;
    rq.head = proc->next;
    rq.size--;
    if (rq.head == NULL) rq.tail = NULL;
    proc->next = NULL;
    return proc;
}

int rq_get_head_indx() { 
    return rq.head ? rq.head->script_idx : -1; 
}
int rq_get_tail_indx() { 
    return rq.tail ? rq.tail->script_idx : -1; 
}


static char *fetch_line(struct SCRIPT_PCB *pcb, int line_idx) {
    int logical_page = line_idx / PAGE_SIZE;
    int frame        = pcb->page_table[logical_page];
    if (frame == -1) return NULL;
    frame_store[frame].lru_time = lru_clock++;
    return frame_store[frame].lines[line_idx % PAGE_SIZE];
}

// Variable memory
void mem_init(void) {
    printf("Frame Store Size = %d; Variable Store Size = %d\n",
           FRAME_STORE_SIZE, MEM_SIZE);

    for (int i = 0; i < MEM_SIZE; i++) {
        shellmemory[i].var   = "none";
        shellmemory[i].value = "none";
    }

    for (int i = 0; i < MAX_FRAMES; i++) {
        for (int j = 0; j < PAGE_SIZE; j++)
            frame_store[i].lines[j][0] = '\0';
        frame_store[i].owner_script = -1;
        frame_store[i].logical_page = -1;
        frame_store[i].lru_time     = 0;
    }

    // Initialize the array for script pointers
    for (int i = 0; i < MAX_SCRIPTS; i++)
        scripts[i] = NULL;

    rq.head = NULL;
    rq.tail = NULL;
    rq.size = 0;
}

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
        if (strcmp(shellmemory[i].var, var_in) == 0)
            return strdup(shellmemory[i].value);
    }
    return "Variable does not exist";
}
