/**
 *  Author: patryk.nizio@gmail.com; github.com/Dyzio18;
*   Date: 26.10.2017r.
*   PROGRAM: Bytes_Counter
*   DESCRIPTION: This program can count bits zeros and ones [0/1] from file
*       it can be use to other programs or to simple check file validation.  
*   HOW TO USE: run program from console with flags. (Read README to know more)
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

int main(int argc, char * * argv) {

    /** FLAGS:
    -a : set to count bytes "0"
    -b : set to count bytes "1"
    -c <FILE_NAME>: file name, need to execute program
    */
    int A_FLAG = 0;
    int B_FLAG = 0;
    char * FILE_NAME = NULL;
    int index;
    int c;

    /**  (OPTIONAL) opterr = 1; if 0 set to no display errors
    *   Gets parameters from standard input
    */

    while ((c = getopt(argc, argv, "abc:d:012")) != -1)
        switch (c) {
            case 'a':
                A_FLAG = 1;     // Count bytes "0"
                break;
            case 'b':
                B_FLAG = 1;     // Count bytes "1"
                break;
            case 'c':
                FILE_NAME = optarg;
                break;
            case '?':
                if (optopt == 'c'){
                    fprintf(stderr, "Option -%c Needs argument, (example: test.txt)\n", optopt);
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

    printf("\nCount Zeros? %d\nCount Ones? %d\nFILE: %s\n", A_FLAG, B_FLAG, FILE_NAME);
    for (index = optind; index < argc; index++)
        printf("No flag: %s\n", argv[index]);

    /**
     * Create pointer to file, open and catch error
     */
    FILE * fp; // File pointer
    fp = fopen(FILE_NAME, "rb"); // Open file from input and set to read
    if (fp == NULL) {
        printf("Cannot open %s\n", FILE_NAME);
        return 1;
    }

    // Init counters
    int count_zeros = 0;
    int count_ones = 0;

    /**
     * Count length of file in bytes (8 bits)
     */
    long int file_size = 0;
    fseek(fp, 0, SEEK_END); // Seek to end of file
    file_size = ftell(fp);  // Get current file pointer
    fseek(fp, 0, SEEK_SET); // Seek back to beginning of file
    //[OPTIONAL] long int file_size_bits = sizeof(char) * file_size; // Get size of file in bits (not bytes)

    /**
     * Counter loop.
     * First loop gets chars from file
     * and second loop count bytes in this character
     */
    int value;
    while (( value = fgetc(fp )) != EOF)
        for (size_t i = 0; i < CHAR_BIT * sizeof(char); ++i, value >>= 1)
            ((value & 1) == 0) ? ++count_zeros : ++count_ones;

    printf("\n*RESULT*\nFILE_SIZE (bytes) :%ld\nFILE_NAME:%s\n", file_size, FILE_NAME);
    if(A_FLAG) printf("Zeros: %d\n",count_zeros);
    if(B_FLAG) printf("Ones: %d\n",count_ones);
    fclose(fp); // Close file
    return 0;
}