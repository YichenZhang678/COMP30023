#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_
#define MAX_LENGTH 8

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

    int service_time;
    // current frames number
    int frames_num;
    // for linked list
    node_t* next;
};


typedef struct {
    node_t* head;  // address of head
    node_t* foot;  // address of foot
} list_t;


list_t* make_empty_list();
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


void delete_node(list_t* list, node_t* node);
node_t* get_head(list_t* list);

#endif