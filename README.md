#A2: Arda Kaan Yildirim & Ece Göksu

Key Decisions for 1.2.1:
- Data Structure to store the lines of a running program:<br>
	We decided to store the lines with a 2D array of size _MEM\_SIZE_ with each entry of size _MAX\_USER\_INPUT_.

- Data Structure of PCBs and Ready Queue:<br>
	We stored the PCB in a struct called _SOURCE_PCB_. <br>
	Additionally, as suggested, we added a `struct SOURCE_PCB* next` field to implement the ready queue as a linked list. 
