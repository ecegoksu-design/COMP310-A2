#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"

int parseInput(char ui[]);

// Start of everything
int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Shell version 1.5 created Dec 2025\n");

    char prompt = '$';  				// Shell prompt
    char userInput[MAX_USER_INPUT];		// user's input stored here
    int errorCode = 0;					// zero means no error, default

    // Detect interactive vs batch mode
    int interactive = isatty(STDIN_FILENO);

    //init user input
    for (int i = 0; i < MAX_USER_INPUT; i++) {
        userInput[i] = '\0';
    }
    
    //init shell memory
    mem_init();
    while (1) {
        if (interactive) {
            printf("%c ", prompt);
        }
        if (fgets(userInput, MAX_USER_INPUT - 1, stdin) == NULL) {
            break;
        }
        errorCode = parseInput(userInput);
        if (errorCode == -1) exit(99); 
        memset(userInput, 0, sizeof(userInput));
    }
    return 0;
}

int wordEnding(char c) {
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ';
}

int parseInput(char input[]) {
    char *command;
    char *rest = input;
    int errorCode = 0;

    while ((command = strtok_r(rest, ";", &rest))) {

        char temp[200], *words[100];
        int index = 0, w = 0, wordlength;
        while (command[index] == ' ') index++;
        while (command[index] != '\0' && command[index] != '\n') {
            for (wordlength = 0; !wordEnding(command[index]); index++, wordlength++) {
                temp[wordlength] = command[index];
            }
            temp[wordlength] = '\0';
            if (wordlength > 0) {
                words[w++] = strdup(temp);
            }
            if (command[index] == '\0') break;
            index++;
        }
        if (w > 0) {
            errorCode = interpreter(words, w);
        }
        for (int i = 0; i < w; i++) {
            free(words[i]);
        }
    }
    return errorCode;
}
