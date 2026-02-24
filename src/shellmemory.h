#define MEM_SIZE 1000
void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
void reset_scripts();
int make_script_pcb(FILE *p);
int get_pcb_length(int pcb_indx);
char *get_script_line(int index, int i);
int rq_get_head_indx();
int rq_get_tail_indx();
