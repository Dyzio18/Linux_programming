#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#define sec_to_nsec 1000000000
#define MAX_PATHS 15
#define BUF 128 
#define TOT 10 
char line[TOT][BUF];
int pathsCounter = 0;
char* paths[MAX_PATHS];

// File descriptor to self-pipe
static int pfd_A[2];
static int pfd_B[2];

// Temporary data to write 
char signA, signB; // Actual sign to read
int numA, numB = 0; // Actual line to read

// Signals variables
int sig_A, sig_B = 0;
int sig_A_counter, sig_B_counter = 0; // Signal counter;
int letter_A, letter_B = 0; // Actual letter to write
int continueAwrite, continueBwrite = 0;

// Output fifo file
int fifoFile;

struct timespec req, rem;
int ALIVE = 0;
int PAUSE_DISPLAY, TEXT_DISPLAY = 0;
int FIFO_ENABLE = 0;
int total = 0; // Line counter with rhythm in config file

// Timer
timer_t timer_A, timer_B;
struct itimerspec timer_A_val, timer_B_val, resetTime;

/**
 * Function handle alarm signals and write to output FIFO
 * @param signum
 * @param siginfo
 * @param ptrVoid
 */
void alarm_handler(int signum, siginfo_t *siginfo, void *ptrVoid) {
    char sign;
    char currLetter;

    // Response on USR1 
    if( signum == SIGALRM ) {
        currLetter = line[numA % total][letter_A++];

        if(currLetter != 0){
            if(currLetter == '*'){
                sign = signA;
                if(write(fifoFile, &sign, 1) == -1){
                    FIFO_ENABLE=0;
                    continueAwrite = 1;
                    perror("write");
                };            }
            if(currLetter == '.' && PAUSE_DISPLAY){
                sign = '-';
                if(write(fifoFile, &sign, 1) == -1){
                    FIFO_ENABLE=0;
                    continueAwrite = 1;
                    perror("write");
                };
            }
        }
        else {
            sig_A--; // Finish line
            letter_A=0;
            timer_settime(timer_A,0,&resetTime,NULL);
        }
    }

    // Response on USR2
    if( signum == SIGVTALRM ) {
        currLetter = line[numB % total][letter_B++];

        if(currLetter != 0){
            if(currLetter == '*'){
                sign = signB;
                if(write(fifoFile, &sign, 1) == -1){
                    FIFO_ENABLE=0;
                    continueBwrite = 1;
                    perror("write");
                };
            }
            if(currLetter == '.'  && PAUSE_DISPLAY){
                sign = '_';
                if(write(fifoFile, &sign, 1) == -1){
                    FIFO_ENABLE=0;
                    continueBwrite = 1;
                    perror("write");
                };
            }
        }
        else {
            sig_B--; // Finish line
            letter_B=0;
            timer_settime(timer_B,0,&resetTime,NULL);
        }
    }
}

/**
 * Function handle signal SIGUSR1 and SIGUSR2,
 * after catch signal handler send data to self-pipe on selected channel (self-pipe A or self-pipe B)
 * @param signum
 * @param siginfo
 * @param ptrVoid
 */
void usr_handler(int signum, siginfo_t *siginfo, void *ptrVoid)
{
    ALIVE = 1;
    int savedErrno;
    savedErrno = errno;
    int usrSign = ((sig_A_counter + sig_B_counter) % 25) + 65;
    int data[2]; // Data frame to send

    if(signum == SIGUSR1) {
        data[0] = usrSign;
        data[1] = sig_A_counter + sig_B_counter;
        sig_A_counter++;
        if (write(pfd_A[1], &data, sizeof(data)) == -1 && errno != EAGAIN)
            perror("write");
    }

    if (signum == SIGUSR2) {
        data[0] = usrSign;
        data[1] = sig_A_counter + sig_B_counter;
        sig_B_counter++;
        if (write(pfd_B[1], &data, sizeof(data)) == -1 && errno != EAGAIN)
            perror("write");
    }
    errno = savedErrno;
}


/**
 * Function takes milliseconds and executes nanosleep
 * @return <int> 0 - success, -1 - failure (errno set to indicate the error)
 */
int nsleep(double time)
{
    long miliseconds = (long)time;

    if(miliseconds > 999) {
        req.tv_sec = (int)(miliseconds / 1000);
        req.tv_nsec = (miliseconds - ((long)req.tv_sec * 1000)) * 1000000;
    }
    else {
        req.tv_sec = 0;
        req.tv_nsec = miliseconds * 1000000;
    }
    return nanosleep(&req , &rem);
}


/**
 * Handle when pipe broken and try search new file
 * After that send signal to continue write
 * @param signum
 * @param siginfo
 * @param ptrVoid
 */
void pipe_handler(int signum, siginfo_t *siginfo, void *ptrVoid)
{
    if(!FIFO_ENABLE){
        for(int j = 0; j < pathsCounter && !FIFO_ENABLE; j++){
            fifoFile = open(paths[j], O_WRONLY);
            nsleep(100);
            if(fifoFile > 0)
                FIFO_ENABLE=1;
        }
    }
    if(continueAwrite)
        raise(SIGALRM);

    if(continueAwrite)
        raise(SIGVTALRM);
}
/**
 * Create handler for timers
 */
void alarmSignals(){
    struct sigaction sa;
    memset( &sa, '\0', sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = alarm_handler;

    if( (sigaction(SIGALRM, &sa, NULL)) == -1 || (sigaction(SIGVTALRM, &sa, NULL))) {
        perror("sigaction");
    }
}
/**
 * Create handler for users signals SIGUSR1 and SIGUSR2
 */
void usrSignals()
{
    struct sigaction usr;
    memset( &usr, '\0', sizeof(usr));
    usr.sa_flags = SA_SIGINFO;
    usr.sa_sigaction = usr_handler;

    if( (sigaction(SIGUSR1, &usr, NULL)) == -1 || (sigaction(SIGUSR2, &usr, NULL))) {
        perror("sigaction");
    }
}

/**
 * Display help when run with wrong options or -h/--help
 */
void helpDisplay(){
    printf("\n\nRhythm generator - run program with this option:\n -d, --delay <float>\tset your rhythm delay in milisecond\n\t\t\t(default is 1000 milisecond = 1 sec.)\n -f,--file <path> <path> ...\t set paths to your fifo files.\n -h, --help\t\t display help.\n -p, --pause\t\t send pause to fifo \n -t,--text\t\t print config file.\n example:\n\t./rhythm.a -d 500 --file fifo1 fifo2 fifo3\n\n\n");
}

// *************************************************************
// *************************************************************
//                           MAIN
// *************************************************************
// *************************************************************
int main(int argc, char **argv) {

    double delayTime = 1;

    // *************************************************************
    //                      READ PARAMETERS (getopd_long)
    // *************************************************************
    /* Get parameters to specify the initial conditions */
    int next_option;
    int option_index = 0;
    char* const short_options = "thpd:f:";
    /* Struct to getopt_long {long_name <string>,required_argument(1)/no_argument(0), NULL, short_name <char> )*/
    static struct option long_options[] = {
        {"text", no_argument, NULL, 't'},
        {"help", no_argument, NULL, 'h'},
        {"pause", no_argument, NULL, 'p'},
        {"delay", required_argument, NULL, 'd' },
        {"file", required_argument, NULL, 'f' },
        {NULL, 0, NULL, 0 }
    };

    do {
        next_option = getopt_long(argc, argv, short_options, long_options, &option_index);
        switch (next_option) {
            case 't':
                TEXT_DISPLAY = 1;
                break;
            case 'p':
                PAUSE_DISPLAY = 1;
                break;
            case 'h':
                helpDisplay();
                return 0;
            case 'd':
                delayTime = strtod(optarg,0);
                printf("\nDelay: %lf\n", delayTime);
                break;
            case 'f':
                printf("\nPaths: ");
                optind--;
                for(int i = 0 ;optind < argc && *argv[optind] != '-' && i < MAX_PATHS; i++, optind++){
                    paths[i] = argv[optind];
                    printf(" %s, ",argv[optind]);
                    pathsCounter++;
                }
                printf("\n");
                break;
            case '?':
                helpDisplay();
                return 0;
            default:
                //printf("ERROR: getopt returned character code 0%o ??\n", next_option);
                break;
        }

        /*if (optind < argc) {
            printf("non-option ARGV-elements: ");
            while (optind < argc)
                printf("%s ", argv[optind++]);
            printf("\n");
        }*/
    } while(next_option != -1);

    // *************************************************************
    //                      SELF-PIPE
    // *************************************************************

    fd_set readfds_A,readfds_B;
    int  flags_A, flags_B;

    /**
     * Create self-pipe (from The Linux Programming Interface )
     */
    if ((pipe(pfd_A) == -1) || (pipe(pfd_B) == -1))
        perror("pipe");

    FD_SET(pfd_A[0], &readfds_A); /* Add read end of pipe to 'readfds_A' */
    flags_A = fcntl(pfd_A[0], F_GETFL);

    FD_SET(pfd_B[0], &readfds_B);
    flags_B = fcntl(pfd_B[0], F_GETFL);

    if ((flags_A == -1) || (flags_B == -1))
        perror("fcntl-F_GETFL");

    flags_A |= O_NONBLOCK; /* Make read end nonblocking */
    flags_B |= O_NONBLOCK;

    if ((fcntl(pfd_A[0], F_SETFL, flags_A) == -1) || (fcntl(pfd_B[0], F_SETFL, flags_B) == -1))
        perror("fcntl-F_SETFL");

    flags_A = fcntl(pfd_A[1], F_GETFL);
    flags_B = fcntl(pfd_B[1], F_GETFL);

    if ((flags_A == -1) || (flags_B == -1))
        perror("fcntl-F_GETFL");

    flags_A |= O_NONBLOCK; /* Make write end nonblocking */
    flags_B |= O_NONBLOCK;

    if ((fcntl(pfd_A[1], F_SETFL, flags_A) == -1) || (fcntl(pfd_B[1], F_SETFL, flags_B) == -1))
        perror("fcntl-F_SETFL");

    // *************************************************************
    //                      SIGNAL HANDLERS
    // *************************************************************

    alarmSignals();
    usrSignals();

    struct sigaction sigPipe;
    memset( &sigPipe, '\0', sizeof(sigPipe));
    sigPipe.sa_flags = SA_SIGINFO;
    sigPipe.sa_sigaction = pipe_handler;
    if( (sigaction(SIGPIPE, &sigPipe, NULL)) == -1 ) {
        perror("sigaction");
    }

    // *************************************************************
    //                      TIMERS
    // *************************************************************

    // A >>>
    struct sigevent timer_A_evp;
    timer_A_evp.sigev_notify = SIGEV_SIGNAL;
    timer_A_evp.sigev_signo = SIGALRM;
    timer_A_evp.sigev_value.sival_ptr = &timer_A;
    if(timer_create(CLOCK_MONOTONIC, &timer_A_evp, &timer_A) < 0){
        perror("timer_create");
    }

    timer_A_val.it_value.tv_sec = (time_t) delayTime;
    timer_A_val.it_value.tv_nsec = (delayTime - (time_t) delayTime) * sec_to_nsec;
    timer_A_val.it_interval.tv_sec = timer_A_val.it_value.tv_sec;
    timer_A_val.it_interval.tv_nsec = timer_A_val.it_value.tv_nsec;

    // B >>>
    struct sigevent timer_B_evp;
    timer_B_evp.sigev_notify = SIGEV_SIGNAL;
    timer_B_evp.sigev_signo = SIGVTALRM;
    timer_B_evp.sigev_value.sival_ptr = &timer_B;
    if(timer_create(CLOCK_MONOTONIC, &timer_B_evp, &timer_B) < 0){
        perror("timer_create");
    }

    timer_B_val.it_value.tv_sec = (time_t) delayTime;
    timer_B_val.it_value.tv_nsec = (delayTime - (time_t) delayTime) * sec_to_nsec;
    timer_B_val.it_interval.tv_sec = timer_B_val.it_value.tv_sec;
    timer_B_val.it_interval.tv_nsec = timer_B_val.it_value.tv_nsec;

    resetTime.it_interval.tv_nsec = resetTime.it_interval.tv_sec = 0;
    resetTime.it_value.tv_sec = resetTime.it_value.tv_nsec = 0;


    // *************************************************************
    //                   READ CONFIG FILE
    // *************************************************************

    FILE *plist = NULL;
    char tmpLine[BUF];
    plist = fopen("./rhythm.config", "r");
    if (plist < 0) {
        perror("open error");
        return -1;
    };

    while(fgets(tmpLine, BUF, plist)) {
        if(tmpLine[0]!='#'){
            strncpy(line[total++], tmpLine, sizeof(tmpLine));
            line[total][strlen(line[total]) - 1] = '\0';
        }
        if(TEXT_DISPLAY)
            printf("%s",tmpLine);
    }

    // *************************************************************
    //                      SWEET MAGIC
    // *************************************************************
    pause();
    while (ALIVE) { /* Consume bytes from pipe */

        if(!FIFO_ENABLE){
            for(int j = 0; j < pathsCounter && !FIFO_ENABLE; j++){
                nsleep(100);
                fifoFile = open(paths[j], O_WRONLY);
                if(fifoFile > 0)
                    FIFO_ENABLE=1;
            }
        }

        if (FD_ISSET(pfd_A[0], &readfds_A) || FD_ISSET(pfd_B[0], &readfds_B) ) {

            // Check data in self-pipe
            while ((sig_A == 0) || (sig_B == 0)) {
                int data[2]; // Frame with data do recieve

                if (sig_B == 0) {
                    if (read(pfd_B[0], &data, sizeof(data)) > 0) {  // If data in self-pipe
                        sig_B++;
                        signB = (char)data[0];
                        numB = data[1];
                        timer_settime(timer_B, 0, &timer_B_val, NULL);
                        break;
                    }
                }

                if (sig_A == 0) {
                    if (read(pfd_A[0], &data, sizeof(data)) > 0) {  // If  data in self-pipe
                        sig_A++;
                        signA = (char)data[0];
                        numA = data[1];
                        timer_settime(timer_A, 0, &timer_A_val, NULL);
                        break;
                    }
                }
                //nsleep(delayTime);
            }
        }
        pause();
    }

    exit(EXIT_SUCCESS);
}

