#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shellmemory.h"
#include "shell.h"
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

int MAX_ARGS_SIZE = 3;

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}

// Available Functions
int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int badcommandFileDoesNotExist();
int echo(char *input);
int my_ls(); 
int my_mkdir(char *dirname); 
int my_touch(char *filename);
int my_cd(char *dirname);
int run(char* commands[], int args_size);

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand();
    }

    for (int i = 0; i < args_size; i++) {
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) {
        if (args_size != 1) return badcommand();
        return help();

    } else if (strcmp(command_args[0], "quit") == 0) {
        if (args_size != 1) return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        if (args_size != 3) return badcommand();
        return set(command_args[1], command_args[2]);

    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2) return badcommand();
        return print(command_args[1]);

    } else if (strcmp(command_args[0], "source") == 0) {
        if (args_size != 2) return badcommand();
        return source(command_args[1]);

    } else if (strcmp(command_args[0], "echo") == 0) {
        if (args_size != 2) return badcommand();
        return echo(command_args[1]);

    } else if (strcmp(command_args[0], "my_ls") == 0) {
        if (args_size != 1) return badcommand();
        return my_ls();

    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
        if (args_size != 2) return badcommand();
        return my_mkdir(command_args[1]);

    } else if (strcmp(command_args[0], "my_touch") == 0) {
        if (args_size != 2) return badcommand();
        return my_touch(command_args[1]);

    } else if (strcmp(command_args[0], "my_cd") == 0) {
        if (args_size != 2) return badcommand();
        return my_cd(command_args[1]);
    } else if (strcmp(command_args[0], "run") == 0) {
        if (args_size < 2) return badcommand();
        return run(command_args, args_size);
    }

    return badcommand();
}

int help() {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n ";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value) {
    // Challenge: allow setting VAR to the rest of the input line,
    // possibly including spaces.

    // Hint: Since "value" might contain multiple tokens, you'll need to loop
    // through them, concatenate each token to the buffer, and handle spacing
    // appropriately. Investigate how `strcat` works and how you can use it
    // effectively here.

    mem_set_value(var, value);
    return 0;
}


int print(char *var) {
    printf("%s\n", mem_get_value(var));
    return 0;
}

int source(char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");      // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    fgets(line, MAX_USER_INPUT - 1, p);
    while (1) {
        errCode = parseInput(line);     // which calls interpreter()
        memset(line, 0, sizeof(line));

        if (feof(p)) {
            break;
        }
        fgets(line, MAX_USER_INPUT - 1, p);
    }

    fclose(p);

    return errCode;
}

int echo(char *input){
    if (input[0] == '$'){ 
        // the user is trying to print a variable
        char var[100];
        size_t len = strlen(input);
        int i;
        for (i = 0; i+1 < len; i++)
            var[i] = input[i + 1]; // Get the variable's name
        var[i] = '\0'; // null-terminate

        char *value = mem_get_value(var);
        if (strcmp(value, "Variable does not exist") == 0) {
            printf("\n");
        } else {
            printf("%s\n", value);
        }
        return 0;
    } 
    else {
        printf("%s\n", input);
        return 0;
    }
    
}

int my_ls(){ 
    struct dirent* dirp;
    DIR *dir = opendir(".");
    char *names[256];
    int count = 0;
    int i, j;

    if (dir == NULL) {
        return 0;
    }

    while ((dirp = readdir(dir))) {
        names[count++] = strdup(dirp->d_name);
    }
    closedir(dir);

    // Sort directory entries
    for (i = 0; i < count - 1; i++) {
        for (j = 0; j < count - i - 1; j++) {
            if (strcmp(names[j], names[j + 1]) > 0) {
                char *tmp = names[j];
                names[j] = names[j + 1];
                names[j + 1] = tmp;
            }
        }
    }

    for (i = 0; i < count; i++) {
        printf("%s\n", names[i]);
        free(names[i]);
    }

    return 0;
}

int my_mkdir(char *dirname){ 
    char name[100];
    int i;

    if (dirname[0] == '$') {
        char *value = mem_get_value(dirname + 1);

        if (strcmp(value, "Variable does not exist") == 0) {
            printf("Bad command: my_mkdir\n");
            return 0;
        }

        for (i = 0; value[i]; i++) {
            if (!isalnum(value[i])) {
                printf("Bad command: my_mkdir\n");
                return 0;
            }
        }

        strcpy(name, value);
        free(value);
    }
    else {
        for (i = 0; dirname[i]; i++) {
            if (!isalnum(dirname[i])) {
                printf("Bad command: my_mkdir\n");
                return 0;
            }
        }

        strcpy(name, dirname);
    }

    // Attempt to create directory
    if (mkdir(name, 0755) != 0) {
        if (errno != EEXIST) {
            printf("Bad command: my_mkdir\n");
        }
        return 0;
    }

    return 0;
}

int my_touch(char *filename){
    // Need to check if filename already exists
    struct dirent* dirp;
    DIR *dir = opendir("."); // Open the directory
    while ((dirp=readdir(dir))) // if dirp is null, there's no more content to read
    {
        if (strcmp(filename, dirp->d_name)==0){ // File already exists, return
            closedir(dir); // close the handle (pointer)
            return 0;
        }
    }
    closedir(dir); // close the handle (pointer)
    
    // File permissions: rwxr-xr-x
	int fd = creat(filename, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    if (fd < 0){
        // Failure... no specification
        return -1;
    } 
    return 0;
}

int my_cd(char *dirname){
    /* Changes current directory to directory dirname, inside the current directory. If
    dirname does not exist inside the current directory, my_cd displays “Bad command: my_cd” and stays
    inside the current directory. dirname should be an alphanumeric string. */
    
    struct dirent* dirp;
    DIR *dir = opendir("."); // Open the directory
    while ((dirp=readdir(dir))) // if dirp is null, there's no more content to read
    {
        if (strcmp(dirname, dirp->d_name)==0){ // directory found, cd
            chdir(dirname);
            closedir(dir);
            return 0;
        }
    }
    // File not found
    closedir(dir);
    printf("Bad command: my_cd\n");    
    return 0;    
}

int run(char* commands[], int args_size){
    pid_t pid = fork();
    
    if(pid < 0){
    // Error
        return 2; 
    } else if (pid == 0){
        // exec
        char* temp[100];
        short i;
        for(i=0; i<args_size-1; i++)
            temp[i] = commands[i+1]; // copy every word in the input except for the first (run)
            
        temp[i] = NULL; // null-terminate the input array
        execvp(temp[0], temp);
        exit(1);
    } else {
        // Parent
        wait(0);
    }
    
    return 0;
}
