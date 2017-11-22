### Bits Counter

DESCRIPTION: 

This program can count bits (0/1) from file.
 It can be use to other programs or to simple check file validation.  

### How to compile?

#### For Linux:
Compile:  `g++ -Wall main.c -o a.out`
set as executable `chmod +x`  and run `./a.out [flags]`

#### For Windows: 
(save as .exe) Compile: `g++ -Wall main.c -o a.exe` and run `a.exe [flags]`

### Flags
- -a : set for check counter of zeros bytes
- -b : set for check counter of ones bytes
- -c [FILE_NAME] : set on what file execute program

#### Compile with execute:

```
dyzio@dyzio:~/Desktop/works/github_repo/bytes_counter$ g++ -Wall main.c  -o a.out
dyzio@dyzio:~/Desktop/works/github_repo/bytes_counter$ ./a.out -a -c test.txt 

Count Zeros? 1
Count Ones? 0
FILE: test.txt

*RESULT*
FILE_SIZE (bytes) :82
FILE_NAME:test.txt
Zeros: 388

```