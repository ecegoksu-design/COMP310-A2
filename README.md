#A2: Arda Kaan Yildirim & Ece Göksu

Key Decisions for 1.2.1:
- Data Structure to store the lines of a running program:
	We decided to store the lines with a 2D array of size _MEM\_SIZE_ with each entry of size _MAX\USER\INPUT_.

- Data Structure of PCB's and Ready Queue:
	We stored the PCB in a struct called SOURCE_PCB.
	Additionally, as suggested, we added a `struct SOURCE_PCB* next` field to implement the ready queue as a sort of linked-list. 
