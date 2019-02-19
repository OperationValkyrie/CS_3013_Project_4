#ifndef VM_H_
#define VM_H_

#define SIZE 64
#define PAGESIZE 16
#define NUMPAGES SIZE/PAGESIZE
#define NUMPROCESSES 4
#define NUMSWAPS NUMPROCESSES * 5

#define HEADERBITS 4
#define PHYSICALBIT 0
#define PRESENTBIT 1
#define WRITEABLEBIT 2
#define VALIDBIT 3

unsigned char memory[SIZE];
 
int registers[NUMPROCESSES];            // Process ID to Page Table Num
int reverseResgisters[NUMPAGES];        // Page Num to PID        
int freePages[NUMPAGES];                // PAge Table Num to Permissions
int freeSwaps[NUMSWAPS];

void mapProcess(char options[4][10]);
void storeValue(char options[4][10]);
void loadValue(char options[4][10]);

int getHeaderPhysicalAddress(int page_num, int process_id, int bit_num);
int getOpenPage();
int getNextPage(int page_table_num);

int nextSwap;
int swapOut(int protected_id, int page_num, int flag);
int swapIn(int process_id, int swap_num);
int getSwappableSpace(int protected_id);
int writeToDisk(int swap_num, int page_num);
int readFromDisk(int page_num, int swap_num);

void printMemory();
void printPage(int page_num);
void printRegisters();
void clearMemory();
void clearPage();

int validInput(char options[4][10]);
int verbose;
int showMemory;
int showResgisters;

void printHelp();
void initalize();

char* trim(char *string);
#endif