### Bytes Counter

DESCRIPTION: This program can count bytes (0/1) from file. It can be use to other programs or to simple check file validation.  

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

#### Example:
Terminal:
```$> a.exe -a -b -c test.txt```

Output:

```
Counter 0: 1, Counter 1: 1, file: test.txt.
test.txt
Number of zeros:6 , ones: 10, all:16  in test.txt
```