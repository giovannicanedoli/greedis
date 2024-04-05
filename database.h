#pragma once

typedef struct node{
    char* key;
    char* value;
    int deadline;
    time_t start_time;
    struct node* next;
}Node;

typedef struct hashMap {
    int numOfElements;
    int capacity;
    Node** arr;
}Database;

void setNode(Node* node, char* key, char* value);
void initializeDatabase(Database* db);
int hashFunction(Database* db, const char* key);
void insert(Database* db, char* key, char* value);
char* search(Database* db, const char* key);
