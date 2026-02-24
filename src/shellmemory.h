#define MEM_SIZE 1000
void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
void reset_scripts();
struct SCRIPT_PCB *make_script_pcb(FILE *p);
struct SCRIPT_PCB *rq_get_head();
struct SCRIPT_PCB *rq_get_tail();
void rq_enqueue(struct SCRIPT_PCB *script);
