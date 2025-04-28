#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include"linked_list.h"
#include"process_utils.h"

#define COMMAND_LENGTH 7
int main(int argc, char* argv[]) {
    // The arguments can be passed in any order but you can assume that all the
    // arguments will be passed correctly
    // and each argument will be passed exactly once.

    // create 3 variable we need
    char* filename;
    char* method;
    int quantum, time = 0, i;
    // first check if the input is correct 7 for argv, argc counts the amound of
    // argv therefore check if argc is 7 or not
    if (argc != COMMAND_LENGTH) {
        // if not COMMAND_LENGTH
        printf("Wrong Input\n");
        return 0;
    }
    for (i = 0; i < argc - 1; i++) {
        // when filename
        if (strcmp(argv[i], "-f") == 0) {
            filename = argv[i + 1];
        }
        // when memory
        if (strcmp(argv[i], "-m") == 0) {
            method = argv[i + 1];
        }
        // when quantum
        if (strcmp(argv[i], "-q") == 0) {
            // atoi is used to change string to int which learnt in foa
            quantum = atoi(argv[i + 1]);
        }
    }

    // create a queue
    list_t* input_queue = make_empty_list();
    // read the input
    read_input(input_queue, filename);

    if (strcmp(method, "infinite") == 0) task_one(input_queue, time, quantum);
    else if (strcmp(method, "first-fit") == 0) task_two(input_queue, time, quantum);
    else if (strcmp(method, "paged") == 0) task_three(input_queue, time, quantum);
    else if (strcmp(method, "virtual") == 0) task_four(input_queue, time, quantum);

    return 0;
}
