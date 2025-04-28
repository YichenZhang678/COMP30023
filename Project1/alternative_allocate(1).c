#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LENGTH 8

// use number to define the state
#define READY_STATE 0
#define RUNNING_STATE 1
#define FINISH_STATE 2
#define TOTAL_MEMORY 2048
#define PAGE_FRAME_SIZE 4

// struct area

// applying round-robin scheduling and this is similar to the linked list
// using typedef to create the head and foot of the list
typedef struct node node_t;

struct node {
    // name of the process
    // +1 for null byte
    char process_name[MAX_LENGTH + 1];
    // the state of the process
    int processState;
    // the time of process entry the list
    int entrying_time;
    // the remaining  time of the process
    int remaining_time;
    // memory space which is the memory usage needed
    int memory_space;
    // start address for allocate in memory
    int allocate_address;

    // record pages numbers that allocated
    int allocate_frames[512];

    // current frames number
    int frames_num;
    // for linked list
    node_t* next;
};

typedef struct {
    node_t* head;  // address of head
    node_t* foot;  // address of foot
} list_t;

// function area
list_t* make_empty_list(void);
list_t* insert_at_head(list_t* list, node_t* new_node);
list_t* insert_at_foot(list_t* list, node_t* value);
list_t* get_tail(list_t* list);
node_t* get_head(list_t* list);
int is_empty_list(list_t* list);
void free_list(list_t* list);
node_t* create_node(char* process_name,
                    int entrying_time,
                    int remaining_time,
                    int memory_space);
void read_input(list_t* input_queue, char* filename);
void get_entrying_list(list_t* input_queue, list_t* ready_queue, int time, int* remainingProcess);
void delete_node(list_t* list, node_t* node);
node_t* get_head(list_t* list);

// from foa
list_t* make_empty_list(void) {
    list_t* list;
    list = (list_t*)malloc(sizeof(*list));
    assert(list != NULL);
    list->head = list->foot = NULL;
    return list;
}

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
node_t* get_head(list_t* list) {
    assert(list != NULL && list->head != NULL);
    
    node_t* prev_head = list->head;

    list->head = list->head->next;
    
    if(list->head == NULL) list->foot = NULL;

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
    process->frames_num = 0; // no pages was allocated
    process->next = NULL;
    return process;
}

void runningProcess(list_t* queue, node_t* currPointer, int* time, node_t* nextProcess, int quantum, int* remaining_process, node_t** nextPointer);
void task_one_new(list_t* queue, int time, int quantum);
void make_process_ready(list_t* queue);
// Task 1 functions
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
void task_one(list_t* queue, int time, int quantum);


// Task 2 functions
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
void task_two(list_t* queue, int time, int quantum);
void allocate_memory(int* memory, node_t* node, int start_address);
int memory_detect(int* memory, int memory_space, int curr_memory);
void allocate_get_entrying_list(list_t* input_queue, list_t* ready_queue, int time, int* remainingProcess, int* curr_memory, int* memory, int quantum);
void runProcess(list_t* queue, list_t* input_queue,node_t* node, int* curr_memory, int* time, int* memory, int* remaining_process, int quantum);
void deallocate_memory(int* memory, node_t* node, int* curr_memory);

// Task 3 functions
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
void task_three(list_t* queue, int time, int quantum);
void allocate_page_fit(list_t* queue, node_t* node, int time, int* pages);
void evict_page(list_t* queue, int frame_needed, int* pages, int time);
void deallocate_pages(int* pages, node_t* node, int time);
int count_free_pages(int* pages);
void print_frames(node_t* node);

// Task 4 functions
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
void task_four(list_t* queue, int time, int quantum);
void allocate_virtual_memory(list_t* queue, node_t* node, int time, int* memory);
int virtual_memory_evict_page(list_t* queue, node_t* node,int* pages, int time);
void update_frames(node_t* node);



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
    // if (argc != 7) {
    //     // if not 7
    //     printf("Wrong Input\n");
    //     return 0;
    // }
    // then for argc is 7
    // if (argc == 7) {
    // printf("argc has %d\n", argc);
    // for (i =0; i < argc; i++){
    // printf("argv = %s\n", argv[i]);}
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
    // }
    // create a queue
        list_t* input_queue = make_empty_list();
        // read the input
        read_input(input_queue, filename);

        task_three(input_queue, time, 3);

    return 0;
    }


void task_one(list_t* queue, int time, int quantum){
    list_t* runningQueue = make_empty_list();
    int reamining_process = 0;
    while(!is_empty_list(runningQueue) || !is_empty_list(queue)){
        get_entrying_list(queue, runningQueue, time, &reamining_process);

        if (!is_empty_list(runningQueue))
        {
            node_t* process_pop = get_head(runningQueue);

            if (process_pop->processState == READY_STATE)
            {
                process_pop->processState = RUNNING_STATE;
                printf("%d,RUNNING,process-name=%s,remaining-time=%d\n", time, process_pop->process_name, process_pop->remaining_time);
            }
            
            time += quantum;
            process_pop->remaining_time -= quantum;

                // detect wehther process is finished
            if (process_pop->remaining_time <= 0){
                process_pop->remaining_time = 0;
                reamining_process -= 1;
                process_pop->processState = FINISH_STATE;
                printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", time, process_pop->process_name, reamining_process);
            } else{
                // if there are other process, set state of current process to READY
                // printf("current reamaining process = %d\n", reamining_process);
                get_entrying_list(queue, runningQueue, time, &reamining_process);
                if (reamining_process != 1) process_pop->processState = READY_STATE;
                
                runningQueue = insert_at_foot(runningQueue, process_pop);
                
            }
        } else{
            time+=quantum;
            continue;;
        }
    }
}

void get_entrying_list(list_t* input_queue, list_t* ready_queue, int time, int* remainingProcess) {
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


// Task 2
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
void task_two(list_t* queue, int time, int quantum){
    // initialize the memory array
    int memory[TOTAL_MEMORY];
    memset(memory, 0, sizeof(memory));

    // initialize running queue and variable
    list_t* runningQueue = make_empty_list();
    int remaining_process = 0;
    int curr_memory = 0;
    while(!is_empty_list(runningQueue) || !is_empty_list(queue)){
        allocate_get_entrying_list(queue, runningQueue, time, &remaining_process, &curr_memory, memory, quantum);

        // printf("current usage = %d\n", usage_convert(curr_memory));

        if (!is_empty_list(runningQueue))
        {
            node_t* process_pop = get_head(runningQueue);

            if(process_pop->allocate_address == -1){
                int start_address = memory_detect(memory, process_pop->memory_space, curr_memory);
                if (start_address >= 0){
                    allocate_memory(memory, process_pop, start_address);
                    curr_memory += process_pop->memory_space;
                    // printf("curr process = %s, allocate_address = %d\n", process_pop->process_name, process_pop->allocate_address);
                    runProcess(runningQueue, queue, process_pop, &curr_memory, &time, memory, &remaining_process, quantum);
                } else {
                    // printf("here\n");
                    runningQueue = insert_at_foot(runningQueue, process_pop);
                    continue;
                }
                    
            } else{
                runProcess(runningQueue, queue, process_pop, &curr_memory, &time, memory, &remaining_process, quantum);
            }
        } else{
            time += quantum;
            continue;
        }
    }

    
}

void deallocate_memory(int* memory, node_t* node, int* curr_memory){
    for (int i = node->allocate_address; i <= (node->allocate_address) + (node->memory_space); i++){
        memory[i] = 0;
    }

    (*curr_memory) -= node->memory_space;
}

void runProcess(list_t* queue, list_t* input_queue ,node_t* node, int* curr_memory, int* time, int* memory, int* remaining_process, int quantum){
    if (node->processState == READY_STATE)
    {
        node->processState = RUNNING_STATE;
        printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%.0f%%,allocated-at=%d\n", 
        *time, node->process_name, node->remaining_time, ((double)(*curr_memory)/TOTAL_MEMORY)*100, node->allocate_address);
    }

    node->remaining_time -= quantum;
    (*time) += quantum;
    
    // detect wehther process is finished
    if (node->remaining_time <= 0){
        node->remaining_time = 0;
        deallocate_memory(memory, node, curr_memory);
        (*remaining_process) -= 1;
        node->processState = FINISH_STATE;
        printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", *time, node->process_name, *remaining_process);
    } else{
        // if there are other process, set state of current process to READY
        allocate_get_entrying_list(input_queue, queue, *time, remaining_process, curr_memory, memory, quantum);
        if ((*remaining_process) != 1) node->processState = READY_STATE;
        queue = insert_at_foot(queue, node);
    }
}

void allocate_get_entrying_list(list_t* input_queue, list_t* ready_queue, int time, int* remainingProcess, int* curr_memory, int* memory, int quantum) {
    node_t* temp = input_queue->head;
    while (temp != NULL) {
        node_t* next = temp->next;
        int start_address = memory_detect(memory, temp->memory_space, *curr_memory);
        // printf("start address for proces%s is %d\n", temp->process_name, start_address);
        if ((temp->entrying_time <= time)) {
            delete_node(input_queue, temp);
            if (start_address >= 0) {
                allocate_memory(memory, temp, start_address);
                (*curr_memory) += temp->memory_space;
            }
            // printf("curr memory = %d\n", *curr_memory);
            (*remainingProcess) += 1; // first time enqueue
            temp->next = NULL;

            // if(temp->entrying_time == time)
            // { 
            //     ready_queue = insert_at_head(ready_queue, temp);
            // }
            // else{
                ready_queue = insert_at_foot(ready_queue, temp);
            // }
        }
        temp = next;
    }
}

int memory_detect(int* memory, int memory_space, int curr_memory){

    // memory not enough
    if (curr_memory + memory_space > TOTAL_MEMORY) return -1;

    // indicate current contiguous memory space
    int contiguous_memory = 0;
    // detection for contiguous memory address by loop
    for (int i = 0; i < TOTAL_MEMORY; i++){
        if (memory[i] != 1){
            contiguous_memory++;
        }else{
            contiguous_memory = 0;
        }
        // printf("current contiguous memory is %d\n", contiguous_memory);
        if (contiguous_memory >= memory_space) return i - contiguous_memory + 1; // means there exist enough contiguous memory space to allocate
        
    }

    // although there are enough memory space in total, but not enough in contiguous, hence return -1
    return -1;
}

void allocate_memory(int* memory, node_t* node, int start_address){
    node->allocate_address = start_address;
    for (int i = node->allocate_address; i < (node->memory_space) + start_address; i++){
        memory[i] = 1;
    }
}

// Task 3
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//

void task_three(list_t* queue, int time, int quantum){

   int pages[TOTAL_MEMORY/PAGE_FRAME_SIZE];
   memset(pages, 0, sizeof(pages));

   list_t* runningQueue = make_empty_list();
    int remaining_process = 0;
    int curr_pages = 0;

    while(!is_empty_list(runningQueue) || !is_empty_list(queue)){
        get_entrying_list(queue, runningQueue, time, &remaining_process);

        // printf("current usage = %d\n", usage_convert(curr_memory));

        if (!is_empty_list(runningQueue))
        {   

            node_t* process_pop = get_head(runningQueue);

            

            // check wether farames are allocated
            if (process_pop->frames_num == 0){
                // allocate farmes for current node
                allocate_page_fit(runningQueue, process_pop, time, pages);
                
            }
            
            if (process_pop->processState == READY_STATE)
            {
                process_pop->processState = RUNNING_STATE;
                // printf("current blank pages = %d\n", count_free_pages(pages));
                double usage = ceil(((double)(512-count_free_pages(pages))/512)*100);
                printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%.0f%%,mem-frames=", 
                time, process_pop->process_name, process_pop->remaining_time, usage);
                print_frames(process_pop);
            }

            time += quantum;
            process_pop->remaining_time -= quantum;

            if (process_pop->remaining_time <= 0){
                process_pop->remaining_time = 0;
                remaining_process -= 1;
                process_pop->processState = FINISH_STATE;
                deallocate_pages(pages, process_pop, time);
                printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", time, process_pop->process_name, remaining_process);
            } else{
                // if there are other process, set state of current process to READY
                if (remaining_process != 1) process_pop->processState = READY_STATE;
                runningQueue = insert_at_foot(runningQueue, process_pop);
                // get_entrying_list(queue, runningQueue, time, &remaining_process);
            }
                
        } else{
            time += quantum;
            continue;
        }
    } 
}

int count_free_pages(int* pages){
    int free_pages_num = 0;
    for (int i = 0; i < TOTAL_MEMORY/PAGE_FRAME_SIZE; i++){
        if(pages[i] == 0) free_pages_num++;
    }
    return free_pages_num;
}

void allocate_page_fit(list_t* queue, node_t* node, int time, int* pages){

    // frames needed for current node
    int frames_need = ceil(node->memory_space/(double)PAGE_FRAME_SIZE);

    // receive current free pages number
    int current_pages = count_free_pages(pages);

    // the frames needed larger than current free pages, so evict pages 
    if (frames_need > current_pages) evict_page(queue, frames_need, pages, time);

    // after evict pages, next step is to allocate pages for current node
    int curr_frames = 0;

    // allocate pages for current node
    for (int i = 0; i < TOTAL_MEMORY/PAGE_FRAME_SIZE; i++){
        if (curr_frames == frames_need) break;

        if (pages[i] == 0){
            node->allocate_frames[curr_frames] = i;
            curr_frames++;
            pages[i] = 1; // represent the i th page in memory is allocated
        }
    }
    node->frames_num = curr_frames;

}

void deallocate_pages(int* pages, node_t* node, int time){
    
    printf("%d,EVICTED,evicted-frames=[", time);

    // deallocate a process that has finished
    for (int i = 0; i < node->frames_num; i++){
        int page_index = node->allocate_frames[i];
        printf("%d", page_index);
        if (i != node->frames_num - 1) printf(",");
        pages[page_index] = 0;
    }
    printf("]\n");

    node->frames_num = 0;

}

void evict_page(list_t* queue, int frame_needed, int* pages, int time){

    int num_page = 0;

    // this loop will continue loop until we evict enough pages number for the node which needs to be allocated
    while(frame_needed > count_free_pages(pages) ){
        node_t* pop = get_head(queue);
        
        // if the node just pop has been allocated, then we evict pages for this node
        if (pop->allocate_frames != NULL){
            printf("%d,EVICTED,evicted-frames=[", time);
            for (int i = 0; i < pop->frames_num; i++){
                int page_index = pop->allocate_frames[i];
                printf("%d", page_index);
                if (i != pop->frames_num - 1) printf(",");
                pages[page_index] = 0;
            }
            printf("]\n");
        }else{
            // if the node was not allocated
            continue;
        }

        // after evicting pages, update information and push it into queue
        pop->frames_num = 0; // current process was evicted
        queue = insert_at_foot(queue, pop);
    }
}

void print_frames(node_t* node){
    // printf("current node is %s\n", node->process_name);
    // printf("node's frames num = %d", node->frames_num);
    printf("[");

    for (int i = 0; i < node->frames_num; i++){
        printf("%d", node->allocate_frames[i]);
        if (i != node->frames_num - 1) printf(",");
    }

    printf("]\n");
}

// Task 4
// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
void task_four(list_t* queue, int time, int quantum){
    int pages[TOTAL_MEMORY/PAGE_FRAME_SIZE];
    memset(pages, 0, sizeof(pages));

    list_t* runningQueue = make_empty_list();
    int remaining_process = 0;
    int curr_pages = 0;

    while(!is_empty_list(runningQueue) || !is_empty_list(queue)){
        get_entrying_list(queue, runningQueue, time, &remaining_process);

        // printf("current usage = %d\n", usage_convert(curr_memory));

        if (!is_empty_list(runningQueue))
        {   

            node_t* process_pop = get_head(runningQueue);
            // printf("curr pop is %s\n", process_pop->process_name);

            // check wether farames are allocated
            if (process_pop->frames_num > 0)
            {   
                // does not meet the minimum requirements for running
                if (process_pop->frames_num < 4)
                {   
                    // if the current frames number does not meet the whole pages needed itself, allocate memory
                    if (process_pop->frames_num != ceil(process_pop->memory_space/(double)PAGE_FRAME_SIZE))
                    {
                        allocate_virtual_memory(runningQueue, process_pop, time, pages);
                    } // allocate memory
                } 

            } else{ // frames num == 0, We should allocate memory for it
                allocate_virtual_memory(runningQueue, process_pop, time, pages);
            }


            if (process_pop->processState == READY_STATE)
            {
                process_pop->processState = RUNNING_STATE;
                
                // printf("current blank pages = %d\n", count_free_pages(pages));
                double usage = ceil(((double)(512-count_free_pages(pages))/512)*100);
                printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%.0f%%,mem-frames=", 
                time, process_pop->process_name, process_pop->remaining_time, usage);
                print_frames(process_pop);
            }

            time += quantum;
            process_pop->remaining_time -= quantum;

            if (process_pop->remaining_time <= 0){
                process_pop->remaining_time = 0;
                remaining_process -= 1;
                process_pop->processState = FINISH_STATE;
                deallocate_pages(pages, process_pop, time);
                get_entrying_list(queue, runningQueue, time, &remaining_process);
                printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", time, process_pop->process_name, remaining_process);
            } else{
                // if there are other process, set state of current process to READY
                get_entrying_list(queue, runningQueue, time, &remaining_process);
                if (remaining_process != 1) process_pop->processState = READY_STATE;
                runningQueue = insert_at_foot(runningQueue, process_pop);
                // get_entrying_list(queue, runningQueue, time, &remaining_process);
            }
                
        } else{
            time += quantum;
            continue;
        }
    } 

}

void allocate_virtual_memory(list_t* queue, node_t* node, int time, int* pages){

    // frames needed for current node
    int frames_need_toFill = ceil(node->memory_space/(double)PAGE_FRAME_SIZE) - node->frames_num;

    int current_pages = count_free_pages(pages);

    // if there are not enough pages for running
    // Then we evict 
    if (node->frames_num + current_pages < 4) virtual_memory_evict_page(queue, node, pages, time);

    // after evicting, we sure current allocated frames + free pages >= 4
    // allocate as much pages as possible

    // printf("current node is %s\n", node->process_name);
    int allocated_count = 0;
    for (int i = 0; i<TOTAL_MEMORY/PAGE_FRAME_SIZE; i++){
        if (allocated_count == frames_need_toFill) break; // we have allocated all pages for current node

        if (pages[i] == 0) {
            node->allocate_frames[node->frames_num] = i;
            // printf("%d with index %d ", node->allocate_frames[node->frames_num], node->frames_num);
            node->frames_num+= 1;
            pages[i] = 1;
            allocated_count ++;
            
        }
    }
}

int virtual_memory_evict_page(list_t* queue, node_t* node, int* pages, int time){
    // printf("current node is %s\n", node->process_name);

    // remember free!!!
    list_t* un_allocated = make_empty_list();

    int current_free_pages = count_free_pages(pages);
    // Once we evict enough pages, break the loop
    while(node->frames_num + count_free_pages(pages) < 4) 
    {
        node_t* pop = get_head(queue);

        // printf("current pop evict is%s\n", pop->process_name);
        if (pop->frames_num != 0)
        {
            printf("%d,EVICTED,evicted-frames=[", time);
            for(int i = 0; i < pop->frames_num; i++)
            {
                if (node->frames_num + current_free_pages >= 4) break;
                
                int page_index = pop->allocate_frames[i];
                printf("%d", page_index);
                
                pages[page_index] = 0;
                current_free_pages++;
                if (node->frames_num + current_free_pages < 4) printf(",");
                // indicate those frames are evict
                pop->allocate_frames[i] = -1;
            }
            printf("]\n");

            update_frames(pop);
            insert_at_head(queue, pop);
        } else
        {   
            insert_at_foot(un_allocated, pop);
            continue;
        }
        
    }

    if (!is_empty_list(un_allocated) )
    {
        un_allocated->foot->next = queue->head;
        queue->head = un_allocated->head;
    }
    

}

void update_frames(node_t* node){

    // create a temp frames array for given node
    int* current_array = node->allocate_frames;

    int new_array[TOTAL_MEMORY/PAGE_FRAME_SIZE];

    int invalid_frames = 0;
    int valid_frames = 0;

    // copy valid page index in old array
    for (int i = 0; i < node->frames_num; i++)
    {
        if (current_array[i] == -1)
        {
            invalid_frames++;
        } else 
        {
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





void make_process_ready(list_t*queue){
    if (is_empty_list(queue)) return;
    node_t* temp = queue->head;
    while(temp!= NULL){
        temp->processState = READY_STATE;
        temp = temp->next;
    }
}









// void allocate_get_entrying_list(list_t* input_queue, list_t* ready_queue, int time, int* remainingProcess, int* curr_memory, int* memory, int quantum) {
//     node_t* temp = input_queue->head;
//     while (temp != NULL) {
//         node_t* next = temp->next;
//         int start_address = memory_detect(memory, temp->memory_space, *curr_memory);
//         // printf("start address for proces%s is %d\n", temp->process_name, start_address);
//         if ((temp->entrying_time <= time)) {
//             if (start_address >= 0)
//             {
//                 delete_node(input_queue, temp);
//                 allocate_memory(memory, temp, start_address);
//                 (*curr_memory) += temp->memory_space;

//                 if (temp->is_Waiting != 1) (*remainingProcess) += 1; // first time enqueue
//                 temp->next = NULL;

//                 if(temp->entrying_time == time)
//                 { 
//                 ready_queue = insert_at_head(ready_queue, temp);
//                 }
//                 else{ready_queue = insert_at_foot(ready_queue, temp);}
                
//             } 
//             else
//             {
//                 if (temp->is_Waiting != 1)
//                 {
//                     temp->is_Waiting = 1;
//                     (*remainingProcess) ++;
//                 }
//             }
//         }
//         temp = next;
//     }
// }


// ---------------------------------------------------------------------------------------------------------------------------------------------------------//
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

/////////////////////////////////////////////////////////////////////////////