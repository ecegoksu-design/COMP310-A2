# A3: Ece Göksu & Arda Kaan Yıldırım 
The Unix shell is extended with demand-paging virtual memory system. Supports scheduling policies. 

# Building
cd src
make clean
make mysh framesize=<X> varmemsize=<Y>

# Running 
Interactive mode:
./mysh

# Memory Architecture
1. Backing store - heap copy of all script lines
char (*backing_store)[MAX_USER_INPUT];  

- reads script file into heap memory

2. Frame Store - Phyical RAM
struct FrameEntry frame_store[MAX_FRAMES];  

- holds owner_script, logical_page, lru_time

3. Page Table
int *page_table;  

- frame index where page lives

# Shell Commands
set VAR VALUE
print VAR
echo VALUE
source FILE
my_ls
my_mkdir DIR
my_touch FILE
my_cd DIR
run CMD [ARGS]
help
quit

# Scheduling Policies
FCFS
SJF
RR
RR30
AGING

# Test All
bash src/test_all.sh

# Files
Makefile 
- framesize and varmemsize passed from CL to gcc

shellmemory.c
-  The following line prints the new message: printf("Frame Store Size = %d; Variable Store Size = %d\n", FRAME_STORE_SIZE, MEM_SIZE);

shellmemory.h
- Data structures
struct FrameEntry 
    char lines[PAGE_SIZE][MAX_USER_INPUT]; 
    int  owner_script;   
    int  logical_page;   
    int  lru_time;  

shellmemory.c
Loads inital page. allocate_script() read lines in backing_store
-  for (int pg = 0; pg < num_pages && loaded < pages_to_load; pg++) {
    int frame = find_free_frame();
    if (frame == -1) break;
    load_page_to_frame(pcb, pg, frame);
    loaded++;    
};

ensure_page_in_store -> free frame path vs. LRU eviction path

# Runtime
int logical_page = line_idx / PAGE_SIZE;  // logical page
int frame = pcb->page_table //isinRAM?[logical_page];  
load_page_to_frame(pcb, logical_page, frame_idx);
pcb->page_table[1] = frame_idx;  // update map
frame_store[frame].lines[line_idx % PAGE_SIZE] // get actual line