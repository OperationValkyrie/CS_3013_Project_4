#ifndef VM_H_
#define VM_H_

#define SIZE 64
#define PAGESIZE 16
#define NUMPAGES SIZE/PAGESIZE
#define NUMPROCESSES 4

#define HEADERBITS 4
#define PHYSICALBIT 0
#define PRESENTBIT 1
#define WRITEABLEBIT 2
#define VALIDBIT 3

unsigned char memory[SIZE];
 
int registers[NUMPROCESSES];            // Process ID to Page Table Num
int reverseResgisters[NUMPAGES];        // Page Num to PID        
int freePages[NUMPAGES];                // PAge Table Num to Permissions

void mapProcess(char options[4][10]);
void storeValue(char options[4][10]);
void loadValue(char options[4][10]);

int getHeaderPhysicalAddress(int page_num, int bit_num);
int getOpenPage();
int getNextPage(int page_table_num);

void printMemory();
void printPage(int page_num);

int validInput(char options[4][10]);
int verbose;
int showMemory;

void printHelp();
void initalize();

char* trim(char *string);
#endif