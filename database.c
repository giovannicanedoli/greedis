#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


typedef struct node{
    char* key;
    char* value;
    time_t start_time;
    int deadline;
    struct node* next;
}Node;

typedef struct hashMap {
    int capacity;
    Node** arr;
}Database;

void setNode(Node* node, char* key, char* value){
    node->key = key;
    node->value = value;
    node->next = NULL;
    node -> deadline = -1;
    return;
};

void initializeDatabase(Database* db){
 
    db->capacity = 100;
 
    db->arr = malloc(sizeof(Node*)* db->capacity);
    return;
}

int hashFunction(Database* db, const char* key){
    int bucketIndex;
    int sum = 0, factor = 31;
    for (int i = 0; i < strlen(key); i++) {
        sum = ((sum % db->capacity)
               + (((int)key[i]) * factor) % db->capacity)
              % db->capacity;
        factor = ((factor % __INT16_MAX__) * (31 % __INT16_MAX__)) % __INT16_MAX__;
    }

    bucketIndex = sum;
    return bucketIndex;
}

void insert(Database* db, char* key, char* value){

    int hash = hashFunction(db, key);
    Node* newNode = malloc(sizeof(Node));
    setNode(newNode, key, value);
 
    if (db->arr[hash] == NULL) {
        db->arr[hash] = newNode;
    }

    else {

        newNode->next = db->arr[hash];
        db->arr[hash] = newNode;
    }
    return;
}

char* search(Database* db, const char* key){
    char* return_msg;
    time_t current_time;
    time(&current_time);

    int hash = hashFunction(db, key);
 
    Node* head = db->arr[hash];

    while (head != NULL) {
        
        if (!strcmp(head->key,key)) {
            
            
            return_msg = malloc(5);
            
            
            double elapsed_time = difftime(current_time, head -> start_time);
            
            if(elapsed_time > head -> deadline && head -> deadline != -1){
                printf("%f %d\n", elapsed_time, head -> deadline);
                return_msg = "$-1\r\n\0";

            }else{
                return_msg = "+OK\r\n\0";
            }
            

            return return_msg;

        }
        head = head->next;
    }
    
    return_msg = malloc(5);

    return_msg = "$-1\r\n\0";
    return return_msg;
}