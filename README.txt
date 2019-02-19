Joanthan Chang
CS 3013
Project 4

Project:
This project simulates the managing of virtual memory using paging.
To do this the memory has been simulated as an array that houses both the 
pages and the page tables. Part 1 does not account for swapping and 
while the user is able to store and load data, the amount of usable memory is rather
small. Part 2 impliments swithcing various pages in and out which allows 
a user to make full advantage of the maximu mamoutn of pages for each process.

One major assumption is that page tables can only occupy 1 page and
therefore the maximum pages per process is 4. Furthermore, the user is 
barred from attempting to allocate a page to the page where the page table is.
Finally the swap slot numbers does not start at 0 because the swap number was used 
to determine whether a page was in memoery or not and certain spots were 
reserved for page tables.

Testing:
This program was tested using the various input files in each part.
Each of these files was specifically written to test the features of the 
part to ensure proper behaviour. The output of these files was then checked
against expected values.

Running instructions:
This program supports the regular instruction supplied in the pdf as
well as the following:

"c" - clears the memory and reintializes all variables
"v" - verbose mode, only used to repeat back the input, useful when piping in txt files
"m" - memory mode, will print out the memory after each instruction
"r" - register mode, will print out the registers after each instruction
"e" - exits the program