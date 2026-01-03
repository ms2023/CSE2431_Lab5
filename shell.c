#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define NUM_LINES 10
#define HISTORY_FILE "singh1964.history"

typedef struct {
    char command[NUM_LINES][MAX_LINE];
    int head;
    int count;
    int total;
} Commands;

Commands cBuf;

void init_Commands(){
    cBuf.head = 0;
    cBuf.count = 0;
    cBuf.total = 0;
}

void add_Command(char *c){
    strncpy(cBuf.command[cBuf.head], c, MAX_LINE);
    cBuf.head = (cBuf.head + 1) % NUM_LINES;
    if(cBuf.count < NUM_LINES){
        cBuf.count++;
    }
    cBuf.total++;
}

void load_history(){
    int fd = open(HISTORY_FILE, O_RDONLY);
    if(fd < 0){
        return;
    }
    
    char buffer[MAX_LINE];
    int bytes_read;
    
    bytes_read = read(fd, &cBuf.total, sizeof(int));
    if(bytes_read != sizeof(int)){
        close(fd);
        return;
    }
    
    int saved_total = cBuf.total;
    cBuf.total = 0;
    
    while(1){
        int cmd_len;
        bytes_read = read(fd, &cmd_len, sizeof(int));
        if(bytes_read != sizeof(int)){
            break;
        }
        
        bytes_read = read(fd, buffer, cmd_len);
        if(bytes_read != cmd_len){
            break;
        }
        buffer[cmd_len] = '\0';
        
        add_Command(buffer);
    }
    
    cBuf.total = saved_total;
    close(fd);
}

void save_history(){
    int fd = creat(HISTORY_FILE, 0644);
    if(fd < 0){
        return;
    }
    
    write(fd, &cBuf.total, sizeof(int));
    
    int start = (cBuf.head - cBuf.count + NUM_LINES) % NUM_LINES;
    for(int i = 0; i < cBuf.count; i++){
        int index = (start + i) % NUM_LINES;
        int len = strlen(cBuf.command[index]);
        write(fd, &len, sizeof(int));
        write(fd, cBuf.command[index], len);
    }
    
    close(fd);
}

void handle_SIGINT(int signo){
    write(STDOUT_FILENO, "\nCommand History (most recent last):\n", strlen("\nCommand History (most recent last):\n"));
    int start = (cBuf.head - cBuf.count + NUM_LINES) % NUM_LINES;
    int startNum = cBuf.total - cBuf.count + 1;
    
    for(int i = 0; i < cBuf.count; i++){
        int index = (start + i) % NUM_LINES;
        int cmdNum = startNum + i;
        
        char numStr[20];
        sprintf(numStr, "%d ", cmdNum);
        write(STDOUT_FILENO, numStr, strlen(numStr));
        
        write(STDOUT_FILENO, cBuf.command[index], strlen(cBuf.command[index]));
        write(STDOUT_FILENO, "\n", 1);
    }
}

/**
 * setup() reads in the next command line, separating it into distinct tokens
 * using whitespace as delimiters. setup() sets the args parameter as a
 * null-terminated string.
 */

void setup(char inputBuffer[], char *args[], int *background, char ogInput[])
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */

    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);

    start = -1;
    if (length == 0){
        save_history();
        exit(0);            /* ^d was entered, end of user command stream */
    }
    if (length < 0){
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
    }
    strncpy(ogInput, inputBuffer, length);
    ogInput[length] = '\0';
    /* examine every character in the inputBuffer */
    for (i = 0; i < length; i++) {
        switch (inputBuffer[i]){
        case ' ':
        case '\t' :               /* argument separators */
            if(start != -1){
                args[ct] = &inputBuffer[start];    /* set up pointer */
                ct++;
            }
            inputBuffer[i] = '\0'; /* add a null char; make a C string */
            start = -1;
            break;

        case '\n':                 /* should be the final char examined */
            if (start != -1){
                args[ct] = &inputBuffer[start];
                ct++;
            }
            inputBuffer[i] = '\0';
            args[ct] = NULL; /* no more arguments to this command */
            break;

        case '&':
            *background = 1;
            inputBuffer[i] = '\0';
            break;

        default :             /* some other character */
            if (start == -1)
                start = i;
        }
    }
    args[ct] = NULL; /* just in case the input line was > 80 */
}

int r_command(char *args[], char *retrievedCmd, char tempBuffer[MAX_LINE]){
    if(args[0] != NULL && strcmp(args[0], "r") == 0 && args[1] == NULL){
        if(cBuf.count == 0){
            write(STDOUT_FILENO, "There are no previous commands.\n", strlen("There are no previous commands.\n"));
            return 0;
        }
        int idx = (cBuf.head - 1 + NUM_LINES) % NUM_LINES;
        strcpy(retrievedCmd, cBuf.command[idx]);
        strcpy(tempBuffer, cBuf.command[idx]);
        
        write(STDOUT_FILENO, retrievedCmd, strlen(retrievedCmd));
        write(STDOUT_FILENO, "\n", 1);
        
        int count = 0;
        char *token = strtok(tempBuffer, " \t\n");
        while(token != NULL && count < MAX_LINE/2) {
            args[count++] = token;
            token = strtok(NULL, " \t\n");
        }
        args[count] = NULL;
        
        return 1;
    }
    else if(args[0] != NULL && strcmp(args[0], "r") == 0 && args[1] != NULL){
        char c = args[1][0];
        int matchCheck = 0;
        for(int i = 1; i <= cBuf.count; i++){
            int idx = (cBuf.head - i + NUM_LINES) % NUM_LINES;
            if(cBuf.command[idx][0] == c){
                strcpy(retrievedCmd, cBuf.command[idx]);
                strcpy(tempBuffer, cBuf.command[idx]);
                matchCheck = 1;
                
                write(STDOUT_FILENO, retrievedCmd, strlen(retrievedCmd));
                write(STDOUT_FILENO, "\n", 1);
                
                int count = 0;
                char *token = strtok(tempBuffer, " \t\n");
                while(token != NULL && count < MAX_LINE/2) {
                    args[count++] = token;
                    token = strtok(NULL, " \t\n");
                }
                args[count] = NULL;
                
                break;
            }
        }

        if(!matchCheck){
            write(STDOUT_FILENO, "There is no matching command.\n", strlen("There is no matching command.\n"));
            return 0;
        }
        return 1;
    }
    return 0;
}

int main(void)
{
    char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
    int background;             /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2+1];   /* 80-character command line has 40 args max */

    init_Commands();
    load_history();
    
    struct sigaction handler;
    handler.sa_handler = handle_SIGINT;
    sigemptyset(&handler.sa_mask);
    handler.sa_flags = SA_RESTART;
    sigaction(SIGINT, &handler, NULL);
    while (1){            /* Program terminates normally inside setup */
        background = 0;
        printf("COMMAND->");
        fflush(0);
        char ogInput[MAX_LINE];
        char retrievedCmd[MAX_LINE];
        char tempBuffer[MAX_LINE];
        setup(inputBuffer, args, &background, ogInput);       /* get next command */
        int is_r_command = 0;
        if(args[0] != NULL && strcmp(args[0], "r") == 0) {
            is_r_command = r_command(args, retrievedCmd, tempBuffer);
            if(!is_r_command){
                continue;
            }
            int len = strlen(retrievedCmd);
            if(len > 0 && retrievedCmd[len-1] == '\n'){
                retrievedCmd[len-1] = '\0';
            }
            add_Command(retrievedCmd);
        }
        else {
            int len = strlen(ogInput);
            if(len > 0 && ogInput[len-1] == '\n'){
                ogInput[len-1] = '\0';
            }
            add_Command(ogInput);
        }
        
        if(args[0] == NULL){
            continue;
        }
    /* the steps are:
     (1) fork a child process using fork()
     (2) the child process will invoke execvp()
     (3) if background == 0, the parent will wait,
         otherwise returns to the setup() function. */
        pid_t c = fork();
        pid_t p;
        int x;
        if(c < 0){ // Checks for invalid value
            printf("ERROR: Fork failed");
            save_history();
            exit(-1);//add an error message
        }else if(c == 0){ //Checks for child value and executes command
            const char *cmd = args[0];
            signal(SIGINT, SIG_DFL);
            execvp(cmd , args);    
        }else{
            if(background == 0){// checks the background to set up the parent wait system
                p = waitpid(c, &x, 0);
            }else{
                save_history();
                exit(0);
            }
        }
    }
}