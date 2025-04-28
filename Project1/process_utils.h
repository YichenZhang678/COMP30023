#ifndef _PROCESS_H_
#define _PROCESS_H_


#include"linked_list.h"

#define READY_STATE 0
#define RUNNING_STATE 1
#define FINISH_STATE 2
#define TOTAL_MEMORY 2048
#define PAGE_FRAME_SIZE 4
#define SCALE_SIZE 100
// General
void get_entrying_list(list_t* input_queue,
                       list_t* ready_queue,
                       int time,
                       int* remainingProcess);

void read_input(list_t* input_queue, char* filename);
double scale_average_time(double num);


// Task 1 round-robin
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
void task_one(list_t* queue, int time, int quantum);


// Task 2 first fit
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
void task_two(list_t* queue, int time, int quantum);

void allocate_memory(int* memory, node_t* node, int start_address);

int memory_detect(int* memory, int memory_space, int curr_memory);

void deallocate_memory(int* memory, node_t* node, int* curr_memory);

// Task 3 page fit
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
void task_three(list_t* queue, int time, int quantum);

void allocate_page_fit(list_t* queue, node_t* node, int time, int* pages);

void evict_page(list_t* queue, int frame_needed, int* pages, int time);

void deallocate_pages(int* pages, node_t* node, int time);

int count_free_pages(int* pages);

void print_frames(node_t* node);

// Task 4 virtual memory
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
void task_four(list_t* queue, int time, int quantum);

void allocate_virtual_memory(list_t* queue,
                             node_t* node,
                             int time,
                             int* memory);
void virtual_memory_evict_page(list_t* queue,
                              node_t* node,
                              int* pages,
                              int time);
void update_frames(node_t* node);


#endif