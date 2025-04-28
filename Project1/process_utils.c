#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linked_list.h"
#include"process_utils.h"


double scale_average_time(double num){
    num *= SCALE_SIZE;
    double after_round = round(num);
    double scale_back = after_round/SCALE_SIZE;
    return scale_back;
}

void get_entrying_list(list_t* input_queue,
                       list_t* ready_queue,
                       int time,
                       int* remainingProcess) {    
    node_t* temp = input_queue->head;
    while (temp != NULL) {
        node_t* next = temp->next;
        if (temp->entrying_time <= time) {
            delete_node(input_queue, temp);
            temp->next = NULL;
            ready_queue = insert_at_foot(ready_queue, temp);
            (*remainingProcess) += 1;
        }
        temp = next;
    }
}

void task_one(list_t* queue, int time, int quantum) {

    // Statistics
    int turnaround_time = 0;
    int count = 0;
    double total_overhead_time = 0;
    double max_overhead_time = 0;

    list_t* runningQueue = make_empty_list();
    int reamining_process = 0;
    while (!is_empty_list(runningQueue) || !is_empty_list(queue)) {
        get_entrying_list(queue, runningQueue, time, &reamining_process);

        if (!is_empty_list(runningQueue)) {
            node_t* process_pop = get_head(runningQueue);

            if (process_pop->processState == READY_STATE) {
                process_pop->processState = RUNNING_STATE;
                printf("%d,RUNNING,process-name=%s,remaining-time=%d\n", time,
                       process_pop->process_name, process_pop->remaining_time);
            }

            time += quantum;
            process_pop->remaining_time -= quantum;

            // detect wehther process is finished
            if (process_pop->remaining_time <= 0) {
                process_pop->remaining_time = 0;
                reamining_process -= 1;
                process_pop->processState = FINISH_STATE;
                printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", time,
                       process_pop->process_name, reamining_process);

                turnaround_time += time - process_pop->entrying_time;
                double current_overhead = (double)(time - process_pop->entrying_time)/process_pop->service_time;
                total_overhead_time += current_overhead;
                if (current_overhead > max_overhead_time) max_overhead_time = current_overhead;
                count+=1;

                // free node
                free(process_pop);
            } else {
                // if there are other process, set state of current process to READY
                get_entrying_list(queue, runningQueue, time,
                                  &reamining_process);
                if (reamining_process != 1)
                    process_pop->processState = READY_STATE;

                runningQueue = insert_at_foot(runningQueue, process_pop);
            }
        } else {
            time += quantum;
            continue;
            ;
        }
    }

    double average_turnaround = ceil((double)turnaround_time/count);
    
    printf("Turnaround time %.0f\n", average_turnaround);
    printf("Time overhead %.2f %.2f\n", max_overhead_time, scale_average_time(total_overhead_time/count));
    printf("Makespan %d\n", time);

    // free lists
    free(runningQueue);
    free(queue);
}

void read_input(list_t* input_queue, char* filename) {
    // open the file in read mode
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("NO INPUT THERE");
        return;
    }
    int entrying_time, remaining_time, memory_space;
    char process_name[MAX_LENGTH + 1];
    while (fscanf(fp, "%d %s %d %d\n", &entrying_time, process_name,
                  &remaining_time, &memory_space) == 4) {
        node_t* new = create_node(process_name, entrying_time, remaining_time,
                                  memory_space);
        input_queue = insert_at_foot(input_queue, new);
    }
    fclose(fp);
}

// Task 2 first fit
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
void task_two(list_t* queue, int time, int quantum) {

    // Statistics
    int turnaround_time = 0;
    int count = 0;
    double total_overhead_time = 0;
    double max_overhead_time = 0;

    // initialize the memory array
    int memory[TOTAL_MEMORY];
    memset(memory, 0, sizeof(memory));

    // initialize running queue and variable
    list_t* runningQueue = make_empty_list();
    int remaining_process = 0;
    int curr_memory = 0;
    while (!is_empty_list(runningQueue) || !is_empty_list(queue)) {

        get_entrying_list(queue, runningQueue, time, &remaining_process);


        if (!is_empty_list(runningQueue)) {
            node_t* process_pop = get_head(runningQueue);

            if (process_pop->allocate_address == -1) {
                int start_address = memory_detect(
                    memory, process_pop->memory_space, curr_memory);
                if (start_address >= 0) {
                    allocate_memory(memory, process_pop, start_address);
                    curr_memory += process_pop->memory_space;
        
                } else {
                    runningQueue = insert_at_foot(runningQueue, process_pop);
                    continue;
                }

            } 
            if (process_pop->processState == READY_STATE) 
            {
                process_pop->processState = RUNNING_STATE;
                printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%.0f%%,"
                "allocated-at=%d\n",time, process_pop->process_name, process_pop->remaining_time,
                ((double)(curr_memory) / TOTAL_MEMORY) * 100,process_pop->allocate_address);
            }

            process_pop->remaining_time -= quantum;
            time += quantum;

            if (process_pop->remaining_time <= 0) {
                process_pop->remaining_time = 0;
                deallocate_memory(memory, process_pop, &curr_memory);
                remaining_process -= 1;

                get_entrying_list(queue, runningQueue, time, &remaining_process);
                process_pop->processState = FINISH_STATE;
                printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", time,
                process_pop->process_name, remaining_process);

                // record statistics
                turnaround_time += time - process_pop->entrying_time;
                double current_overhead = (double)(time - process_pop->entrying_time)/process_pop->service_time;
                total_overhead_time += current_overhead;
                if (current_overhead > max_overhead_time) max_overhead_time = current_overhead;
                count+=1;

                // free node
                free(process_pop);
            } else 
            {
                get_entrying_list(queue, runningQueue, time, &remaining_process);
                // if there are other process, set state of current process to READY
                if (remaining_process != 1) process_pop->processState = READY_STATE;
                runningQueue = insert_at_foot(runningQueue, process_pop);
             }

        } else {
            time += quantum;
            continue;
        }
    }

    // print statistics
    double average_turnaround = ceil((double)turnaround_time/count);
    printf("Turnaround time %.0f\n", average_turnaround);
    printf("Time overhead %.2f %.2f\n", max_overhead_time, scale_average_time(total_overhead_time/count));
    printf("Makespan %d\n", time);

    // free lists
    free(runningQueue);
    free(queue);
}

void allocate_memory(int* memory, node_t* node, int start_address) {
    node->allocate_address = start_address;
    for (int i = node->allocate_address;
         i < (node->memory_space) + start_address; i++) {
        memory[i] = 1;
    }
}

int memory_detect(int* memory, int memory_space, int curr_memory) {
    // memory not enough
    if (curr_memory + memory_space > TOTAL_MEMORY)
        return -1;

    // indicate current contiguous memory space
    int contiguous_memory = 0;
    // detection for contiguous memory address by loop
    for (int i = 0; i < TOTAL_MEMORY; i++) {
        if (memory[i] != 1) {
            contiguous_memory++;
        } else {
            contiguous_memory = 0;
        }
        // printf("current contiguous memory is %d\n", contiguous_memory);
        if (contiguous_memory >= memory_space)
            return i - contiguous_memory +
                   1;  // means there exist enough contiguous memory space to
                       // allocate
    }

    // although there are enough memory space in total, but not enough in
    // contiguous, hence return -1
    return -1;
}

void deallocate_memory(int* memory, node_t* node, int* curr_memory) {
    for (int i = node->allocate_address;
         i <= (node->allocate_address) + (node->memory_space); i++) {
        memory[i] = 0;
    }

    (*curr_memory) -= node->memory_space;
}


// Task 3 page fit
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
void task_three(list_t* queue, int time, int quantum) {

    // Statistics
    int turnaround_time = 0;
    int count = 0;
    double total_overhead_time = 0;
    double max_overhead_time = 0;

    int pages[TOTAL_MEMORY / PAGE_FRAME_SIZE];
    memset(pages, 0, sizeof(pages));

    list_t* runningQueue = make_empty_list();
    int remaining_process = 0;

    while (!is_empty_list(runningQueue) || !is_empty_list(queue)) {
        get_entrying_list(queue, runningQueue, time, &remaining_process);

        if (!is_empty_list(runningQueue)) {
            node_t* process_pop = get_head(runningQueue);

            // check wether farames are allocated
            if (process_pop->frames_num == 0) {
                // allocate farmes for current node
                allocate_page_fit(runningQueue, process_pop, time, pages);
            }

            if (process_pop->processState == READY_STATE) {
                process_pop->processState = RUNNING_STATE;
                double usage =
                    ceil(((double)(512 - count_free_pages(pages)) / 512) * 100);
                printf(
                    "%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%.0f%%,mem-frames=",
                    time, process_pop->process_name,
                    process_pop->remaining_time, usage);
                print_frames(process_pop);
            }

            time += quantum;
            process_pop->remaining_time -= quantum;

            if (process_pop->remaining_time <= 0) {
                process_pop->remaining_time = 0;
                remaining_process -= 1;
                process_pop->processState = FINISH_STATE;
                deallocate_pages(pages, process_pop, time);
                printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", time,
                       process_pop->process_name, remaining_process);
                
                turnaround_time += time - process_pop->entrying_time;
                double current_overhead = (double)(time - process_pop->entrying_time)/process_pop->service_time;
                total_overhead_time += current_overhead;
                if (current_overhead > max_overhead_time) max_overhead_time = current_overhead;
                count+=1;

                // free current node
                free(process_pop);
            } else {
                // if there are other process, set state of current process to READY
                if (remaining_process != 1)
                    process_pop->processState = READY_STATE;
                runningQueue = insert_at_foot(runningQueue, process_pop);
            }

        } else {
            time += quantum;
            continue;
        }
    }

    // print statistics
    double average_turnaround = ceil((double)turnaround_time/count);
    printf("Turnaround time %.0f\n", average_turnaround);
    printf("Time overhead %.2f %.2f\n", max_overhead_time,scale_average_time(total_overhead_time/count));
    printf("Makespan %d\n", time);

    // free list
    free(runningQueue);
    free(queue);
}

void allocate_page_fit(list_t* queue, node_t* node, int time, int* pages) {
    // frames needed for current node
    int frames_need = ceil(node->memory_space / (double)PAGE_FRAME_SIZE);

    // receive current free pages number
    int current_pages = count_free_pages(pages);

    // the frames needed larger than current free pages, so evict pages
    if (frames_need > current_pages)
        evict_page(queue, frames_need, pages, time);

    // after evict pages, next step is to allocate pages for current node
    int curr_frames = 0;

    // allocate pages for current node
    for (int i = 0; i < TOTAL_MEMORY / PAGE_FRAME_SIZE; i++) {
        if (curr_frames == frames_need)
            break;

        if (pages[i] == 0) {
            node->allocate_frames[curr_frames] = i;
            curr_frames++;
            pages[i] = 1;  // represent the i th page in memory is allocated
        }
    }
    node->frames_num = curr_frames;
}

void evict_page(list_t* queue, int frame_needed, int* pages, int time) {

    printf("%d,EVICTED,evicted-frames=[", time);
    // this loop will continue loop until we evict enough pages number for the
    // node which needs to be allocated

    node_t* pop = queue->head;

    while (frame_needed > count_free_pages(pages)) {
        // node_t* pop = get_head(queue);
        if (pop == NULL) break;
        // if the node just pop has been allocated, then we evict pages for this
        // node
        if (pop->allocate_frames != NULL) {
            
            for (int i = 0; i < pop->frames_num; i++) {
                int page_index = pop->allocate_frames[i];
                printf("%d", page_index);
                if (i != pop->frames_num - 1)
                    printf(",");
                pages[page_index] = 0;
            }
            // current process was evicted
            pop->frames_num = 0;
            pop = pop->next;
        } else {
            // if the node was not allocated
            pop = pop->next;
        }
    }
    printf("]\n");
}

void deallocate_pages(int* pages, node_t* node, int time) {
    printf("%d,EVICTED,evicted-frames=[", time);

    // deallocate a process that has finished
    for (int i = 0; i < node->frames_num; i++) {
        int page_index = node->allocate_frames[i];
        printf("%d", page_index);
        if (i != node->frames_num - 1)
            printf(",");
        pages[page_index] = 0;
    }
    printf("]\n");

    node->frames_num = 0;
}

int count_free_pages(int* pages) {
    int free_pages_num = 0;
    for (int i = 0; i < TOTAL_MEMORY / PAGE_FRAME_SIZE; i++) {
        if (pages[i] == 0)
            free_pages_num++;
    }
    return free_pages_num;
}

void print_frames(node_t* node) {
    // printf("current node is %s\n", node->process_name);
    // printf("node's frames num = %d", node->frames_num);
    printf("[");

    for (int i = 0; i < node->frames_num; i++) {
        printf("%d", node->allocate_frames[i]);
        if (i != node->frames_num - 1)
            printf(",");
    }

    printf("]\n");
}

// Task 4 virtual memory
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
void task_four(list_t* queue, int time, int quantum) {


    // Statistics
    int turnaround_time = 0;
    int count = 0;
    double total_overhead_time = 0;
    double max_overhead_time = 0;

    int pages[TOTAL_MEMORY / PAGE_FRAME_SIZE];
    memset(pages, 0, sizeof(pages));

    list_t* runningQueue = make_empty_list();
    int remaining_process = 0;
    // int curr_pages = 0;

    while (!is_empty_list(runningQueue) || !is_empty_list(queue)) {
        get_entrying_list(queue, runningQueue, time, &remaining_process);


        if (!is_empty_list(runningQueue)) {
            node_t* process_pop = get_head(runningQueue);

            // check wether farames are allocated
            if (process_pop->frames_num > 0) {
                // does not meet the minimum requirements for running
                if (process_pop->frames_num < 4) {
                    // if the current frames number does not meet the whole
                    // pages needed itself, allocate memory
                    if (process_pop->frames_num !=
                        ceil(process_pop->memory_space /
                             (double)PAGE_FRAME_SIZE)) {
                        allocate_virtual_memory(runningQueue, process_pop, time,
                                                pages);
                    }  // allocate memory
                }

            } else {  // frames num == 0, We should allocate memory for it
                allocate_virtual_memory(runningQueue, process_pop, time, pages);
            }

            if (process_pop->processState == READY_STATE) {
                process_pop->processState = RUNNING_STATE;

                double usage =
                    ceil(((double)(512 - count_free_pages(pages)) / 512) * 100);
                printf(
                    "%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%."
                    "0f%%,mem-frames=",
                    time, process_pop->process_name,
                    process_pop->remaining_time, usage);
                print_frames(process_pop);
            }

            time += quantum;
            process_pop->remaining_time -= quantum;

            if (process_pop->remaining_time <= 0) {
                process_pop->remaining_time = 0;
                remaining_process -= 1;
                process_pop->processState = FINISH_STATE;
                deallocate_pages(pages, process_pop, time);
                get_entrying_list(queue, runningQueue, time,
                                  &remaining_process);
                printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", time,
                       process_pop->process_name, remaining_process);
                
                turnaround_time += time - process_pop->entrying_time;
                double current_overhead = (double)(time - process_pop->entrying_time)/process_pop->service_time;
                total_overhead_time += current_overhead;
                if (current_overhead > max_overhead_time) max_overhead_time = current_overhead;
                count+=1;

                // free node
                free(process_pop);
            } else {
                // if there are other process, set state of current process to READY
                get_entrying_list(queue, runningQueue, time,
                                  &remaining_process);
                if (remaining_process != 1)
                    process_pop->processState = READY_STATE;
                runningQueue = insert_at_foot(runningQueue, process_pop);
            }

        } else {
            time += quantum;
            continue;
        }
    }

    // print statistics
    double average_turnaround = ceil((double)turnaround_time/count);
    printf("Turnaround time %.0f\n", average_turnaround);
    printf("Time overhead %.2f %.2f\n", max_overhead_time, scale_average_time(total_overhead_time/count));
    printf("Makespan %d\n", time);

    // free lists
    free(runningQueue);
    free(queue);
}

void allocate_virtual_memory(list_t* queue,
                             node_t* node,
                             int time,
                             int* pages) {
    // frames needed for current node
    int frames_need_toFill =
        ceil(node->memory_space / (double)PAGE_FRAME_SIZE) - node->frames_num;

    int current_pages = count_free_pages(pages);

    // if there are not enough pages for running
    // Then we evict
    if (node->frames_num + current_pages < 4)
        virtual_memory_evict_page(queue, node, pages, time);

    // after evicting, we sure current allocated frames + free pages >= 4
    // allocate as much pages as possible

    int allocated_count = 0;
    for (int i = 0; i < TOTAL_MEMORY / PAGE_FRAME_SIZE; i++) {
        if (allocated_count == frames_need_toFill)
            break;  // we have allocated all pages for current node

        if (pages[i] == 0) {
            node->allocate_frames[node->frames_num] = i;
            node->frames_num += 1;
            pages[i] = 1;
            allocated_count++;
        }
    }
}

void virtual_memory_evict_page(list_t* queue,
                              node_t* node,
                              int* pages,
                              int time) {
    printf("%d,EVICTED,evicted-frames=[", time);

    // remember free!!!void
    // list_t* un_allocated = make_empty_list();

    int current_free_pages = count_free_pages(pages);

    node_t* pop = queue->head;
    // Once we evict enough pages, break the loop
    while (node->frames_num + count_free_pages(pages) < 4) {
        if (pop == NULL) break;

        if (pop->frames_num != 0) {

            for (int i = 0; i < pop->frames_num; i++) {
                if (node->frames_num + current_free_pages >= 4)
                    break;

                int page_index = pop->allocate_frames[i];
                printf("%d", page_index);

                pages[page_index] = 0;
                current_free_pages++;
                if (node->frames_num + current_free_pages < 4)
                    printf(",");
                // indicate those frames are evict
                pop->allocate_frames[i] = -1;
            }

            update_frames(pop);
            pop = pop->next;
        } else {
            pop = pop->next;
        }
        
    }
    printf("]\n");
}

void update_frames(node_t* node) {
    // create a temp frames array for given node
    int* current_array = node->allocate_frames;

    int new_array[TOTAL_MEMORY / PAGE_FRAME_SIZE];

    int invalid_frames = 0;
    int valid_frames = 0;

    // copy valid page index in old array
    for (int i = 0; i < node->frames_num; i++) {
        if (current_array[i] == -1) {
            invalid_frames++;
        } else {
            new_array[valid_frames] = current_array[i];
            valid_frames++;
        }
    }

    // update frames number
    node->frames_num -= invalid_frames;

    // free old array

    // copy array
    memcpy(node->allocate_frames, new_array, sizeof(new_array));
}




