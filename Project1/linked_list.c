#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include"linked_list.h"
#include"process_utils.h"

// from foa
list_t* insert_at_head(list_t* list, node_t* new_node) {
    assert(list != NULL && new_node != NULL);
    new_node->next = list->head;
    list->head = new_node;
    if (list->foot == NULL) {
        /* this is the first insertion into the list */
        list->foot = new_node;
    }
    return list;
}

// from foa
list_t* insert_at_foot(list_t* list, node_t* value) {
    assert(list != NULL && value != NULL);
    value->next = NULL;
    if (list->foot == NULL) {
        list->head = list->foot = value;
    } else {
        list->foot->next = value;
        list->foot = value;
    }
    return list;
}

// from foa
void free_list(list_t* list) {
    node_t *curr, *prev;
    assert(list != NULL);
    curr = list->head;
    while (curr) {
        prev = curr;
        curr = curr->next;
        free(prev);
    }
    free(list);
}

// from foa
list_t* get_tail(list_t* list) {
    node_t* oldhead;
    assert(list != NULL && list->head != NULL);
    oldhead = list->head;
    list->head = list->head->next;
    if (list->head == NULL) {
        /* the only list node just got deleted */
        list->foot = NULL;
    }
    free(oldhead);
    return list;
}

// from foa
int is_empty_list(list_t* list) {
    assert(list != NULL);
    return list->head == NULL;
}

// from foa
list_t* make_empty_list() {
    list_t* list;
    list = (list_t*)malloc(sizeof(*list));
    assert(list != NULL);
    list->head = list->foot = NULL;
    return list;
}

// from foa
node_t* get_head(list_t* list) {
    assert(list != NULL && list->head != NULL);

    node_t* prev_head = list->head;

    list->head = list->head->next;

    if (list->head == NULL)
        list->foot = NULL;

    prev_head->next = NULL;

    return prev_head;
}

node_t* create_node(char* process_name,
                    int entrying_time,
                    int remaining_time,
                    int memory_space) {
    // using malloc to create space for the process
    node_t* process = malloc(sizeof(node_t));
    // update the info of process
    process->entrying_time = entrying_time;
    process->remaining_time = remaining_time;
    process->memory_space = memory_space;
    strcpy(process->process_name, process_name);
    // change new state to ready
    process->processState = READY_STATE;
    process->allocate_address = -1;
    // process->allocate_frames = int [512];
    process->service_time = remaining_time;
    process->frames_num = 0;  // no pages was allocated
    process->next = NULL;
    return process;
}

void delete_node(list_t* list, node_t* node) {

    // when head is the node we delete
    if (list->head == node) {
        list->head = list->head->next;
        return;
    }
    // if not
    node_t* previous;
    node_t* current = list->head;
    while (current != NULL) {
        previous = current;
        current = current->next;
        if (current == node) {
            break;
        }
    }
    previous->next = current->next;
    // if the node we delete is the foot
    if (current == list->foot) {
        list->foot = previous;
    }
}

