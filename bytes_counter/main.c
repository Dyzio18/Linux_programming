/** Author: patryk.nizio@gmail.com; github.com/Dyzio18;
*   Date: 26.10.2017r.
*   PROGRAM: Bytes_Counter
*   DESCRIPTION: This program can count bytes (0/1) from file
*       it can be use to other programs or to simple check file validation.  
*   HOW TO USE: run program from console with flags. (Read README to know more)
*/

#include <ctype.h>  
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>

int main(int argc, char * * argv) {

    /* FLAGS:
    -a : set to count bytes "0"
    -b : set to count bytes "1"
    -c <FILE_NAME>: file name, need to execute program
    */
    bool A_FLAG = false;
    bool B_FLAG = false;
    char * FILE_NAME = NULL;
    int index;
    int c;

    /* (OPTIONAL) opterr = 1; if 0 set to no display errors */
    // ========================================================================
    // Gets parametr from standard input

    while ((c = getopt(argc, argv, "abc:d:012")) != -1)
        switch (c) {
        case 'a':
            A_FLAG = true; // Count bytes "0"
            break;
        case 'b':
            B_FLAG = true; // Count bytes "1"
            break;
        case 'c':
            FILE_NAME = optarg;
            break;
        case '?':
            if (optopt == 'c'){
                fprintf(stderr, "Option -%c Needs parametr, (example: test.txt)\n", optopt);
                return 1; // End program when no file
            }
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option -%c'.\n", optopt);
            else
                fprintf(stderr, "Unknown option\\x%x'.\n", optopt);
            return 1;
        default:
            abort();
        }

    printf("Counter 0: %d, Counter 1: %d, file: %s.\n\n", A_FLAG, B_FLAG, FILE_NAME);

    for (index = optind; index < argc; index++)
        printf("No flag: %s\n", argv[index]);

    // ======================================================================== 

    FILE * fp; // File pointer
    fp = fopen(FILE_NAME, "rb"); // Open file from input and read in byte mode
    puts(FILE_NAME);

    if (fp == NULL) {
        printf("Cannot open %s\n", FILE_NAME);
        return 1;
    }

    // Init variables
    char sign;
    int count_0 = 0;
    int count_1 = 0;
    int count_all = 0;

    // Read signs from file
    while (( sign = fgetc(fp )) != EOF) {
        if( A_FLAG && sign == 0 ) count_0++;
        if( B_FLAG && sign == 1 ) count_1++;
        count_all++;
    }

    // Print result
    printf("Number of zeros:%d , ones: %d, all:%d  in %s\n", count_0, count_1, count_all, FILE_NAME);
    
    fclose(fp); // Close file 
    return 0;
}
