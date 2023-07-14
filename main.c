#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void printError() {
    fprintf(stderr, "shysh: cannot allocate memory |-ERROR-|");
}

int shysh_cd(char **args);
int shysh_help(char **args);
int shysh_exit(char **args);

char *builtin_str[] = { "cd","help","exit"};

int (*builtin_fun[])(char **) = {&shysh_cd, &shysh_help, &shysh_exit};

int shysh_num_builtins() {
    return sizeof(builtin_str)/sizeof(char*);
}

int shysh_cd(char **args) {
    if(args[1]==NULL){
        fprintf(stderr, "shysh: Excepted a path");
    }
    else{
        if(chdir(args[1])!=0) {
            perror("shysh");
        }
    }
    return 1;
}

int shysh_help(char **args) {
    int i;
    printf("This Shy's Mini Shell\n");
    printf("Type program names and arguments and hit enter\n");
    printf("The following are built in => \n");

    for(int i=0; i<shysh_num_builtins(); i++) {
        printf("%s\n",builtin_str[i]);
    }

    printf("Use man command for more info of command");
    return 1;
}

int shysh_exit(char **args) {
    return 0;
}

int shysh_lauch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid==0) { // inside child
        if(execvp(args[0], args)==-1){
            perror("shysh");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid<0){ // error
        perror("shysh");
    }
    else{ // parent
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status)); 
    } 
    return 1;   
}

int shysh_exec(char **args) {
    int i;
    if(args[0]==NULL){
        return 1;
    }

    for(int i=0; i<shysh_num_builtins(); i++) {
        if(strcmp(args[0],builtin_str[i])==0) {
            return (*builtin_fun[i])(args);
        }
    }

    return shysh_lauch(args);
}


#define MS 1024
char *shysh_read_line(void) {
    int bSize = MS;
    int position = 0;
    char *buffer = malloc(bSize * sizeof(char));
    int c;

    if(!buffer) {
        fprintf(stderr, "shysh: cannot allocate memory |-ERROR-|");
        exit(EXIT_FAILURE);
    }

    while(1) {
        c = getchar();

        if(c==EOF || c=='\n') {
            buffer[position] = '\0';
            return buffer;
        }
        else{
            buffer[position] = c;
        }
        position++;

        if(position>= bSize) {
            bSize+=MS;
            buffer = realloc(buffer,bSize);
            if(!buffer) {
                fprintf(stderr, "shysh: cannot allocate memory |-ERROR-|");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define SHYSH_TOK_BSIZE 64
#define SHYSH_TOK_DELIM " \t\r\n\a"
char **shysh_split_line(char *line) {
    int bSize = SHYSH_TOK_BSIZE, position = 0;
    char **tokens = malloc(bSize * sizeof(char*));
    char *token;

    if(!tokens) {
        printError();
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SHYSH_TOK_DELIM);
    while(token!=NULL) {
        tokens[position] = token;
        position++;

        if(position >= bSize) {
            bSize+=SHYSH_TOK_BSIZE;
            tokens = realloc(tokens, bSize * sizeof(char*));

            if(!token) {
                printError();
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SHYSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

void shysh_loop(void) {
    char *line;
    char **args;
    int status;

    do{
        printf("-> ");
        line = shysh_read_line();
        args = shysh_split_line(line);
        status =shysh_exec(args);

        free(line);
        free(args);
    }while(status);
}

int main(int argc, char **argv) {
    
    shysh_loop();

    return EXIT_SUCCESS;
}
