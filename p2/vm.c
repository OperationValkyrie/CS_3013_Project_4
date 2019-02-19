/**
 * Jonathan Chang
 * CS 3013
 * Project 4
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

int main(int argc, char **argv) {
    unsigned int input_len = 20;
    verbose = 0;
    showMemory = 0;
    showResgisters = 0;
    initalize();
    while(1) {
        // Get user input
        printf("%s ", "Instruction?");
        char *input;
        input = malloc(input_len + 1);
        getline(&input, &input_len, stdin);
        trim(input);

        // Options for inputing files
        if(verbose) {
            printf("%s\n", input);
        }
        if(strcmp(input, "c") == 0) {
            clearMemory();
            initalize();
            continue;
        } else if(strcmp(input, "e") == 0) {
            exit(0);
        } else if(strcmp(input, "m") == 0) {
            showMemory = !showMemory;
            continue;
        } else if(strcmp(input, "r") == 0) {
            showResgisters = !showResgisters;
            continue;
        } else if(strcmp(input, "v") == 0) {
            verbose = !verbose;
            continue; 
        } else if(strcmp(input, "") == 0) {
            continue;
        }

        // Read in options from instruction
        char options[4][10];
        char *option;
        int i = 0;
        option = strtok(input, ",");
        while(option != NULL) {
            if(i > 4) {
                printf("%s\n", "ERROR: Must contain 4 arguments.");
                printHelp();
                continue;
            }
            strcpy(options[i++], option);
            option = strtok(NULL, ",");
        }
        
        // Validate input
        if(i != 4) {
            printf("%s\n", "ERROR: Must contain 4 arguments.");
            printHelp();
            continue;
        }
        if(!validInput(options)) {
            printHelp();
            continue;
        }

        // Choose options
        char *instruction = options[1];
        if(strcmp(instruction, "map") == 0) {
            mapProcess(options);
        } else if(strcmp(instruction, "store") == 0) {
            storeValue(options);
        } else if(strcmp(instruction, "load") == 0) {
            loadValue(options);
        }
        if(showMemory) {
            printMemory();
        }
        if(showResgisters) {
            printRegisters();
        }
        free(input);
    }
    return 0;
}

/**
 * Maps a process to the given options
 * @param option The options
 */
void mapProcess(char options[4][10]) {
    int process_id = atoi(options[0]);
    int page_table_num = atoi(options[2]) / PAGESIZE;
    // If the phyiscal frame has been mapped to another process
    if(freePages[page_table_num] != -1 && process_id != reverseResgisters[page_table_num]) {
        swapOut(process_id, page_table_num, 0);
    }
    // If only changing permisisons
    if(process_id == reverseResgisters[page_table_num] && registers[process_id] != page_table_num) {
        memory[getHeaderPhysicalAddress(page_table_num, process_id, WRITEABLEBIT)] = atoi(options[3]);
        printf("%s %d %s %d %s %d\n", "Changed permissions of PID", process_id, "at physical frame", page_table_num, "to", atoi(options[3]));
        return;
    }

    // If page table not yet mapped
    int process_page_num;
    int open_page_num;
    if(registers[process_id] == -1) {
        registers[process_id] = page_table_num;
        reverseResgisters[page_table_num] = process_id;
        freePages[page_table_num] = 1;

        process_page_num = getNextPage(page_table_num);
        open_page_num = getOpenPage();

        printf("%s %d %s %d\n", "Put table page for PID", process_id, "into physical frame", page_table_num);
    } else {
        page_table_num = registers[process_id];
        process_page_num = getNextPage(page_table_num);
        open_page_num = atoi(options[2]) / PAGESIZE;
        // Not allowed to allocate page to page table space
        if(open_page_num == page_table_num) {
            printf("%s %d %s %d %s\n", "ERROR: Physical Frame", page_table_num,"already allocated to PID", reverseResgisters[page_table_num], "page table");
            return;
        }
    }

    // If no open slots, open one
    if(open_page_num == -1) {
        swapOut(process_id, getSwappableSpace(process_id), 0);
        open_page_num = getOpenPage();
    }
    
    if(process_page_num == -1) {
        printf("%s\n", "ERROR: Maximum of four pages per process");
        return;
    }
    memory[page_table_num * PAGESIZE + process_page_num * HEADERBITS] = open_page_num;
    memory[getHeaderPhysicalAddress(open_page_num, process_id, PRESENTBIT)] = 1;
    memory[getHeaderPhysicalAddress(open_page_num, process_id, WRITEABLEBIT)] = atoi(options[3]);
    memory[getHeaderPhysicalAddress(open_page_num, process_id, VALIDBIT)] = 1;
    reverseResgisters[open_page_num] = process_id;
    freePages[open_page_num] = 1;
    printf("%s %d%s %d\n", "Mapped virtual address 0 (page", process_page_num,") to physical frame", open_page_num);
    return;
}

/**
 * Stores the information in the options
 * @param options The options
 */
void storeValue(char options[4][10]) {
    int process_id = atoi(options[0]);
    int page_table_num = registers[process_id];
    // Check if PID mapped to physical frame
    if(page_table_num == -1) {
        printf("%s %d %s\n", "ERROR: PID", process_id, "not yet mapped to physical frame");
        return;
    }
    // Check if page table in memory
    if(page_table_num > 3) {
        int new_page_table_num = getSwappableSpace(process_id);
        swapOut(process_id, new_page_table_num, 1);
        swapIn(process_id, process_id + NUMPAGES);
        page_table_num = registers[process_id];
    }
    int virtual_address = atoi(options[2]);
    int process_page_num = virtual_address / PAGESIZE;
    int page_num = memory[page_table_num * PAGESIZE + process_page_num * HEADERBITS];
    // test if page is in memory
    if(page_num > NUMPAGES) {
        int new_page_num = getSwappableSpace(process_id);
        swapOut(process_id, new_page_num, 0);
        swapIn(process_id, page_num);
        memory[page_table_num * PAGESIZE + process_page_num * HEADERBITS] = new_page_num;
        page_num = new_page_num;
    }
    // Test if virtual address is valid
    if(memory[getHeaderPhysicalAddress(page_num, process_id, VALIDBIT)] != 1) {
        printf("%s %d %s %d\n", "ERROR: Virtual Address", virtual_address, "out of virtual memory addresses of PID", process_id);
        return;
    }
    // Check if permissions are writable
    if(memory[getHeaderPhysicalAddress(page_num, process_id, WRITEABLEBIT)] != 1) {
        printf("%s %d %s %d\n", "ERROR: PID", process_id,"only has read permisisons at physical frame", page_table_num);
        return;
    }
    int physical_address = page_num * PAGESIZE + virtual_address % PAGESIZE;
    int value = atoi(options[3]);
    memory[physical_address] = value;
    printf("%s %d %s %d %s %d%s\n", "Stored value", value, "at virtual address", atoi(options[2]), "(physical address", physical_address, ")");
}

/**
 * Loads the value given the information in the options
 * @param options The options
 */
void loadValue(char options[4][10]) {
    int process_id = atoi(options[0]);
    int page_table_num = registers[process_id];
    // Tests if the PID has been mapped 
    if(page_table_num == -1) {
        printf("%s %d %s\n", "ERROR: PID", process_id, "not yet mapped to physical frame");
        return;
    }
    // Test if the desired page table is in memory
    if(page_table_num > 3) {
        int new_page_table_num = getSwappableSpace(process_id);
        swapOut(process_id, new_page_table_num, 1);
        swapIn(process_id, process_id + NUMPAGES);
        page_table_num = registers[process_id];
    }
    int virtual_address = atoi(options[2]);
    int process_page_num = virtual_address / PAGESIZE;
    int page_num = memory[page_table_num * PAGESIZE + process_page_num * HEADERBITS];
    // Test if the desired page is in memory
    if(page_num > NUMPAGES) {
        int new_page_num = getSwappableSpace(process_id);
        swapOut(process_id, new_page_num, 0);
        swapIn(process_id, page_num);
        memory[page_table_num * PAGESIZE + process_page_num * HEADERBITS] = new_page_num;
        page_num = new_page_num;
    }
    // Test if virtual address in process
    if(memory[getHeaderPhysicalAddress(page_num, process_id, VALIDBIT)] != 1) {
        printf("%s %d %s %d\n", "ERROR: Virtual Address", virtual_address, "out of virtual memory addresses of PID", process_id);
        return;
    }
    int physical_address = page_num * PAGESIZE + virtual_address % PAGESIZE;
    int value = memory[physical_address];
    printf("%s %d %s %d %s %d%s\n", "The value", value, "is at virtual address", atoi(options[2]), "(physical address", physical_address, ")");
}

/**
 * Gets the next process page of a page table
 * @param page_table_num The page number of the page table
 * @returns The next page number (-1 if already 4 pages)
 */
int getNextPage(int page_table_num) {
    int i;
    int j = 0;
    for(i = 0; i < PAGESIZE; i += HEADERBITS) {
        if(memory[page_table_num * PAGESIZE + i + VALIDBIT] == 0) {
            return j;
        }
        j++;
    }
    return -1;
}

/**
 * Gets the physical memory address of the given page and header bit
 * @param page_num The page to get the header for
 * @param process_id The process id of the page
 * @param bit_num The header bit to get
 * @returns The physical memeory address of the header bit (-1 if page not initalized)
 */
int getHeaderPhysicalAddress(int page_num, int process_id, int bit_num) {
    int page_table_num = registers[process_id];
    if(page_table_num != -1 && page_table_num < 4) {
        int j;
        for(j = 0; j < PAGESIZE; j += HEADERBITS) {
            if(memory[j + page_table_num * PAGESIZE] == page_num) {
                return j + page_table_num * PAGESIZE + bit_num;
            }
        }
    }
    return -1;
}

/**
 * Gets the next open page
 * @returns THe next open page (-1 if no open pages)
 */
int getOpenPage() {
    int i;
    for(i = 0; i < NUMPAGES; i++) {
        if(freePages[i] == -1) {
            return i;
        }
    }
    return -1;
}

/**
 * Swaps out the given page by saving it to disk
 * @param protected_id The id of the current process (its page talbe may not be kicked)
 * @param page_num The page to kick
 * @param flag Whether or not this call is for a page table
 * @return Success or not (1 for success, -1 for failure)
 */
int swapOut(int protected_id, int page_num, int flag) {
    int result;
    int process_id = reverseResgisters[page_num];
    int page_table_num = registers[process_id];
    
    // If Page table of page not in put it in
    if(page_table_num > 3) {
        int new_table_page_num = getSwappableSpace(protected_id);
        result = swapOut(protected_id, new_table_page_num, 1);
        swapIn(process_id, process_id + NUMPAGES);
        page_table_num = registers[process_id];
        if(flag) {
            return 1;
        }
    }
    // Different swap space and register ids for page tables
    if(page_table_num == page_num) {
        int i = process_id + NUMPAGES;
        result = writeToDisk(i, page_num);
        if(result < 0) {
            return -1;
        }
        registers[process_id] = i;
        freePages[page_num] = -1;
        reverseResgisters[page_num] = -1;
        printf("%s %d %s %d\n", "Moved frame", page_num, "to swap slot", process_id + NUMPAGES);
    } else {
        int i;
        for(i = 8; i < NUMSWAPS; i++) {
            if(freeSwaps[i] == -1) {
                result = writeToDisk(i, page_num);
                break;
            }
        }
        if(result < 0) {
            return -1;
        }
        memory[getHeaderPhysicalAddress(page_num, process_id, PHYSICALBIT)] = i;
        memory[getHeaderPhysicalAddress(page_num, process_id, PRESENTBIT)] = 0;
        freePages[page_num] = -1;
        reverseResgisters[page_num] = -1;
        printf("%s %d %s %d\n", "Moved frame", page_num, "to swap slot", i);
    }
    clearPage(page_num);
    return 1;
}

/**
 * Swaps the given swap slot into the memory
 * @param process_id The process_id
 * @param sawp_num The swap slot ot swap in
 * @return 1 for success, -1 for failure
 */
int swapIn(int process_id, int swap_num) {
    int i;
    for(i = 0; i < NUMPAGES; i++) {
        if(freePages[i] == -1) {
            readFromDisk(i, swap_num);
            freePages[i] = 1;
            reverseResgisters[i] = process_id;
            // If swapping in a page table
            if(swap_num < 8) {
                registers[process_id] = i;
            }
            printf("%s %d %s %d\n", "Moved swap slot", swap_num, "to physical frame", i);
            return 1;
        }
    }
    return -1;
}

/**
 * Gets an avaible swap space
 * The only invalid target is the page table of the current process
 * @param process_id The process id
 * @return The page_num to be kicked
 */
int getSwappableSpace(int protected_id) {
    int i;
    if(nextSwap == NUMPAGES) {
        nextSwap = 0;
    }
    for(i = nextSwap; i < NUMPAGES; i++) {
        int page_process_id = reverseResgisters[i];
        int page_table_num = registers[page_process_id];
        if(!(page_process_id == protected_id && page_table_num == i)) {
            nextSwap = i + 1;
            return i;
        }
        if(i == NUMPAGES - 1) {
            i = -1;
        }
    }
    return -1;
}

/**
 * This was written before, Professor Shue specificed that disk IO was not necessary
 * Copies over the given page to the given swap slot
 * @param sawp_num The swap slot to copy to
 * @param page_num The page to read from
 * @return 1 for success, -1 for failure
 */
int writeToDisk(int swap_num, int page_num) {
    FILE *fp = NULL;
    char filename[20];
    char swap_str[5];
    strcpy(filename, "");
    strcpy(swap_str, "");
    snprintf(swap_str, 5, "%d", swap_num);

    strcat(filename, "./disk/");
    strcat(filename, swap_str);
    strcat(filename, ".txt");
    fp = fopen(filename, "w");
    if(fp == NULL) {
        printf("%s\n", "Unable to create disk file");
        return -1;
    }

    char data[PAGESIZE * 5];
    strcpy(data, "");
    int i;
    for(i = 0; i < PAGESIZE; i++) {
        char byte[5];
        snprintf(byte, 5, "%d", memory[page_num * PAGESIZE + i]);
        strcat(byte, " ");
        strcat(data, byte);
    }
    fprintf(fp, "%s\n", data);

    fclose(fp);
    freeSwaps[swap_num] = 1;
    return 1;
}

/**
 * This was written before, Professor Shue specificed that disk IO was not necessary
 * Reads in data from the swap space and puts it into the given page
 * @param page_num The page to copy to
 * @param swap_num The swap slot to copy from
 * @return 1 for successs, -1 for failure
 */
int readFromDisk(int page_num, int swap_num) {
    FILE *fp = NULL;
    char filename[20];
    char swap_str[5];
    strcpy(filename, "");
    strcpy(swap_str, "");
    snprintf(swap_str, 5, "%d", swap_num);

    strcat(filename, "./disk/");
    strcat(filename, swap_str);
    strcat(filename, ".txt");
    fp = fopen(filename, "r");
    if(fp == NULL) {
        printf("%s %d\n", "Unable to open disk file", swap_num);
        return -1;
    }

    int i = 0;
    char line[PAGESIZE * 5];
    char *byte;
    strcpy(line, "");
    fgets(line, sizeof(line), fp);
    byte = strtok(line, " ");
    while(byte != NULL && i < PAGESIZE) {
        memory[page_num * PAGESIZE + i++] = atoi(byte);
        byte = strtok(NULL, " ");
    }
    fclose(fp);
    freeSwaps[swap_num] = -1;
    return 1;
}

/**
 * Prints out all the memory
 */
void printMemory() {
    int i;
    for(i = 0; i < NUMPAGES; i++) {
        printPage(i);
    }
}

/**
 * Prints out the page
 * @param page_num The page to print out
 */
void printPage(int page_num) {
    int i;
    for(i = 0; i < PAGESIZE; i++) {
        printf("%3d ", memory[page_num * PAGESIZE + i]);
    }
    printf("\n");
}

/**
 * Prints out all of the registers
 */
void printRegisters() {
    int i;
    printf("%-10s:\n", "Registers");
    for(i = 0; i < NUMPAGES; i++) {
        printf("%d ", registers[i]);
    }
    printf("\n%-10s:\n", "Reverse");
    for(i = 0; i < NUMPAGES; i++) {
        printf("%d ", reverseResgisters[i]);
    }
    printf("\n");
}

/**
 * Clears out the memory
 */
void clearMemory() {
    int i;
    for(i = 0; i < NUMPAGES; i++) {
        clearPage(i);
    }
}

/**
 * Clears out a given page of data
 * @param page_num Teh page to clear
 */
void clearPage(int page_num) {
    int i;
    for(i = 0; i < PAGESIZE; i ++) {
        memory[page_num * PAGESIZE + i] = 0;
    }
}

/**
 * Returns whetehr the given input is valid
 * @param options The char array of the inputs
 * @returns Whether the input is valid (0 for no, 1 for valid)
 */
int validInput(char options[4][10]) {
    int process_id = atoi(options[0]);
    if(process_id < 0 || process_id > NUMPROCESSES - 1) {
        printf("%s\n", "Invalid Process ID");
        return 0;
    }

    char *instruction = options[1];
    if(!(strcmp(instruction, "map") == 0 || strcmp(instruction, "store") == 0 || strcmp(instruction, "load") == 0)) {
        printf("%s\n", "Invalid Instruction");
        return 0;
    }

    int virtual_address = atoi(options[2]);
    if(strcmp(instruction, "map") == 0 && (virtual_address < 0 || virtual_address > SIZE - 1)) {
        printf("%s\n", "Invalid Virtual Address for map");
        return 0;
    } else if(strcmp(instruction, "map") != 0 && (virtual_address < 0 || virtual_address > SIZE - 1)) {
        printf("%s %s\n", "Invalid Virtual Address for", instruction);
        return 0;
    }
    

    int value = atoi(options[3]);
    if(strcmp(instruction, "map") == 0 && (value != 0 && value != 1)) {
        printf("%s\n", "Invalid value for instruction map");
        return 0;
    }
    if(value < 0 || value > 255) {
        printf("%s %s\n", "Invalid value for", instruction);
        return 0;
    }
    return 1;
}

/**
 * Prints out the help information for the instructions
 */
void printHelp() {
    printf("\n%s\n", "Instructions require 4 arguments in the format:");
    printf("%s\n", "  process_id,instruction_type,virtual_address,value");
    printf("%s\n", "  process_id       : (int) [0,3] the process id number");
    printf("%s\n", "  instruction_type : (string) the instruction type (map, store, load)");
    printf("%s\n", "  virtual_address  : (int) [0,63] for map, [0,16] for store and load - the virtual address of the instruction");
    printf("%s\n", "  value            : (int) [0,1] for map, [0,255] for store and load - value depending on the instruction");
    printf("%s\n\n", "See the README.txt for more details");
}

/**
 * Inializes the values of resisters, reverseRegisters, and writeable
 */
void initalize() {
    int i;
    nextSwap = 0;
    for(i = 0; i < NUMPAGES; i++) {
        registers[i] = -1;
        reverseResgisters[i] = -1;
        freePages[i] = -1;
    }
    for(i = 0; i < NUMSWAPS; i++) {
        freeSwaps[i] = -1;
    }
    system("rm -rf ./disk/*");
}

/**
 * Removes the final new line character from the end of a string
 * @param string The strig to trim
 * @returns The modified string
 */
char* trim(char *string) {
    int str_len = strlen(string);
    char *end = string + str_len - 1;
    end = end;
    end[0] = '\0';
    return string;
}