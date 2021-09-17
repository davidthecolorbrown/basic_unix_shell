/* basic unix shell program with C */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

// max line buffer
#define MAX_BUFFER 1024
// max # args                        
#define MAX_ARGS 64
// token separators                          
#define SEPARATORS " \t\n"                     

int main (int argc, char ** argv) {

    // line buffer
    char buf[MAX_BUFFER];
    // pointers to arg strings                      
    char *args[MAX_ARGS];
    // working pointer thru args                     
    char **arg;
    // shell prompt                               
    char *prompt = "#";                      

    // initialize an array of ptrs to strings
    char *temp[64];

    //ctr to keep track of number of commands put into commandsList
    int ctr = 0;

    // global variable to track process groups of child processes
    int process_grp = 0;

    /* keep reading input until "quit" command or eof of redirected input */
    while (!feof(stdin)) {

        /* get command line from input */
        // write prompt
        fputs(prompt, stdout);               
        // read a line of max size MAX_BUFFER from stdin, put in buf array 
        if (fgets(buf, MAX_BUFFER, stdin)) { 

            // add buffer to temp string array
            temp[ctr] = (char *) malloc(strlen(buf) + 1);
            strcpy(temp[ctr], buf);

            /* tokenize the input into args array */
            for (int i = 0; i < 64; i++) {
                args[i] = NULL;
            }
            // point arg ptr to first element in array of argument strings (args)
            arg = args;                   
            // tokenize input by pulling input from line buffer buf, returning token string with
            *arg++ = strtok(buf, SEPARATORS);   

            // make all the empty argument space in args NULL
            while ((*arg++ = strtok(NULL, SEPARATORS))); 
            // if there's anything there
            if (args[0]) {      

                /* check for internal/external command */

                // "list process" command
                if (!strcmp(args[0], "ps")) { 
                    system("ps");  // local process list
                    //system("ps - A"); // all processes
                    continue;
                }

                // "list specific process" command
                if (!strcmp(args[0], "s_ps")) { 
                    system("ps -A | grep infiniteLoop");
                    continue;
                }

                // "list jobs" command
                if (!strcmp(args[0], "jobs")) { 
                    system("jobs");
                    continue;
                }

                // "clear" command
                if (!strcmp(args[0], "clear")) { 
                    system("clear");
                    continue;
                }

                // WHEREAMI command
                if (!strcmp(args[0], "whereami")) {
                    // current working directory
                    char cwd[MAX_BUFFER];        

                    if (getcwd(cwd, sizeof(cwd)) != NULL) {
                        printf("Current working dir: %s\n", cwd);
                    } else {
                        printf("Error running whereami command.\n");
                    }
                }

                // CHANGEDIR command
                if (!strcmp(args[0],"changedir"))
                {
                    // variable to hold current directory
                    char cwd[MAX_BUFFER];
                    char new_cwd[MAX_BUFFER+1]; // buffer for new_cwd large enough for NULL char

                    // success => directory exists == true
                    if (chdir(args[1]) == 0) {
                        chdir(args[1]);
                        strncpy(new_cwd, cwd, sizeof(new_cwd)); // copy cwd to new_cwd
                        printf("Directory has been changed to: %s\n", getcwd(cwd, sizeof(cwd)));
                    }

                    // failure/error
                    else {
                        printf("Error, directory does not exist.\n");
                    }
                }

                // LASTCOMMANDS [-C]
                if (!strcmp(args[0],"lastcommands"))
                {

                    // check if clear flag present
                    if (args[1] == NULL) {
                        for (int i = 0; i <= ctr; i++) {
                            printf("COMMAND[%d] %s.\n", i, temp[i]);
                        }

                    }

                    else {
                        // check for [-c] flag
                        if (!strcmp(args[1], "[-c]")) {
                            printf("CLEARED LIST OF RECENT COMMANDS.\n");
                            free(*temp); // free space
                            char *temp[64]; // re-intiailize
                            temp[0] = (char *)(malloc(strlen(buf) + 1));  // get space for current command
                            strcpy(temp[0], buf); // copy current command to array of strings
                            ctr=0; // reset ctr
                        }
                    }

                }

                // RUN PROGRAM [PARAMETERS]
                if (!strcmp(args[0],"run"))
                {

                    // progress through args[] to get total number of arguments from buf, store in argsSize
                    int argsSize = 0;
                    //int *status; // status for child process
                    for (int a = 0; a < MAX_ARGS; a++) {
                        if (args[a] == NULL) { // break if no more args from buf
                            break;
                        }
                        else {
                            //printf("%s PARAMETERS.\n", args[a]);
                            argsSize++;
                        }
                    }

                    // check for absolute path
                    if (args[1][0] == '/') {

                        // run program with parameters
                        // fork() processes
                        int rc = fork();
                        if (rc < 0) {	//fork failure
                            fprintf(stderr, "fork failed\n");
                            exit(1);
                        }
                        else if (rc == 0) {

                            char *myargs[argsSize]; // new array to pass to new process
                            for (int b = 1; b < MAX_ARGS; b++) {
                                if (args[b] == NULL) {
                                    myargs[b-1] = NULL;
                                    break;
                                }

                                else {
                                    myargs[b-1] = args[b];
                                    //printf("Passed args, myargs[b]: %s \n", myargs[b-1]);

                                }
                            }

                            // change directory to given path
                            chdir(myargs[0]);

                            // print error if directory/path do not exist

                            // execute given process using absolute path
                            execv(myargs[0], myargs);  // runs program with path
                            printf("Could not execute process %s, exec().\n", myargs[0]);
                            exit(1); // terminate child process if child process fails to exec myargs[0]
                        }
                        else {
                            // parent goes down this path (original process)
                            waitpid(rc, NULL, 0);
                        }

                    }

                    // RELATIVE PATH
                    else {

                        // run program with parameters
                        // fork() processes
                        int rc = fork();
                        if (rc < 0) {	//fork failure
                            fprintf(stderr, "fork failed\n");
                            exit(1);
                        }
                        else if (rc == 0) {

                            char *myargs[argsSize]; // new array to pass to new process
                            for (int b = 1; b < MAX_ARGS; b++) {
                                if (args[b] == NULL) {
                                    myargs[b-1] = NULL;
                                    break;
                                }

                                else {
                                    myargs[b-1] = args[b];
                                }
                            }

                            //execvp(myargs[0], myargs);
                            execv(myargs[0], myargs);
                            printf("Could not execute process %s, exec().\n", myargs[0]);
                            exit(1);
                        }

                        else {
                            // parent goes down this path (original process)
                            waitpid(rc, NULL, 0);
                        }

                    }
                }

                // BACKGROUND PROGRAM [PARAMETERS]
                if (!strcmp(args[0],"background"))
                {

                    // progress through args[] to get total number of arguments from buf, store in argsSize
                    int argsSize = 0;
                    //int status; // status for child process
                    for (int a = 0; a < MAX_ARGS; a++) {
                        if (args[a] == NULL) { // break if no more args from buf
                            break;
                        }
                        else {
                            argsSize++;
                        }
                    }

                    // check for absolute path
                    if (args[1][0] == '/') {

                        // run program with parameters
                        // fork() processes
                        int rc = fork();
                        if (rc < 0) {	//fork failure
                            fprintf(stderr, "fork failed\n");
                            exit(1);
                        }

                        else if (rc == 0) {

                            printf("PID of background process: %d)\n", (int) getpid());

                            char *myargs[argsSize]; // new array to pass to new process
                            for (int b = 1; b < MAX_ARGS; b++) {
                                if (args[b] == NULL) {
                                    myargs[b-1] = NULL;
                                    break;
                                }

                                else {
                                    myargs[b-1] = args[b];
                                }
                            }

                            // change directory to given path
                            chdir(myargs[0]);

                            // print error if directory/path do not exist


                            // execute given process using absolute path
                            execv(myargs[0], myargs);  // runs program with path
                            printf("Could not execute process %s, exec().\n", myargs[0]);
                            exit(1); // terminate child process if child process fails to exec myargs[0]
                        }
                        else {
                            // parent goes down this path (original process)
                            waitpid(-1, NULL, WNOHANG | WUNTRACED);
                        }
                    }

                    // RELATIVE PATH
                    else {
                        // run program with parameters
                        // fork() processes
                        int rc = fork();
                        if (rc < 0) {	//fork failure
                            fprintf(stderr, "fork failed\n");
                            exit(1);
                        }
                        else if (rc == 0) {
                            // child (new process)
                            printf("PID of background process: %d)\n", (int) getpid());

                            char *myargs[argsSize]; // new array to pass to new process
                            for (int b = 1; b < MAX_ARGS; b++) {
                                if (args[b] == NULL) {
                                    myargs[b-1] = NULL;
                                    break;
                                }

                                else {
                                    myargs[b-1] = args[b];
                                }
                            }

                            execv(myargs[0], myargs);
                            printf("Could not execute process %s, exec().\n", myargs[0]);
                            exit(1);
                        }
                        else {
                            // parent goes down this path (original process)
                            waitpid(-1, NULL, WNOHANG | WUNTRACED);
                        }

                    }
                }

                // EXTERMINATE PID
                if (!strcmp(args[0],"exterminate"))
                {
                    //printf("EXTERMINATE COMMAND RUNNING.\n");
                    int pid = atoi(args[1]); // convert pid from string to int
                    int killed = kill(pid, SIGKILL); // call kill() to kill process ID
                    if (killed == 0) {
                        printf("PID %d WAS SUCCESSFULLY TERMINATED\n", pid);
                    }
                    else { // killed == -1
                        printf("PID %d WAS UNSUCCESSFULLY TERMINATED\n", pid);
                    }
                }

                // QUIT COMMAND
                if (!strcmp(args[0],"quit")) {
                    break;  // break out of 'while' loop
                }

                // REPEAT NUM PROCESS_PATH [PARAMETERS_LIST]
                if (!strcmp(args[0],"repeat"))
                {
                    //printf("REPEAT COMMAND RUNNING.\n");
                    int num = atoi(args[1]); // convert num string to int
                    int i;
                    int pids[num];

                    // progress through args[] to get total number of arguments from buf
                    int argsSize = 0;
                    for (int a = 0; a < MAX_ARGS; a++) {
                        if (args[a] == NULL) { // break if no more args from buf
                            break;
                        }
                        else {
                            argsSize++;
                        }
                    }

                    // loop to create num children
                    for (i=0; i<num; i++)
                    {
                        if ((pids[i] = fork()) == 0) // successful child creation
                        {
                            //setpgid(0,1); // set process group for children
                            setpgid(getpid(), 0);
                            process_grp = getpgid(getpid());

                            char *myargs[argsSize]; // new array to pass to new process
                            for (int b = 2; b < MAX_ARGS; b++) {
                                if (args[b] == NULL) {
                                    myargs[b-2] = NULL;
                                    break;
                                }

                                else {
                                    myargs[b-2] = args[b];
                                }
                            }

                            // change directory to given path
                            chdir(myargs[0]);

                            execv(myargs[0], myargs);
                            printf("Could not execute process %s, exec().\n", myargs[0]);
                            exit(1);
                        }
                        //exit(1); // exit, fork error
                    }

                    // Using waitpid() and printing exit status
                    // of children.
                    printf("PIDs:");
                    for (i=0; i<num; i++)
                    {
                        waitpid(-1, NULL, WNOHANG | WUNTRACED);
                        printf(" %d ", pids[i]);
                    }
                    printf("\n");
                }

                // EXTERMINATEALL
                if (!strcmp(args[0],"exterminateall"))
                {
                    //int pid = atoi(args[1]); // convert pid from string to int
                    //int killed = killpg(0, SIGKILL); // call kill() to kill group processes
                    int killed = killpg(process_grp, SIGKILL); // call kill() to kill group processes
                    if (killed == 0) {
                        //printf("PID %d WAS SUCCESSFULLY TERMINATED\n", pid);
                        printf("ALL PROCESSES WERE SUCCESSFULLY TERMINATED\n");
                    }
                    else { // killed == -1
                        printf("PROCESSES WERE UNSUCCESSFULLY TERMINATED\n");
                        //printf("PID %d WAS UNSUCCESSFULLY TERMINATED\n", pid);
                    }
                }

                ctr++; // iterate counter tracking number of commands
            }
        }
    }

    // clear the memory allocated
    free(*temp);
    return 0;
}