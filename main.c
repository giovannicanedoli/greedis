#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <semaphore.h>
#include <asm-generic/socket.h>
#include <signal.h>

#include "database.h"

#ifdef DEBUG
#endif

#define PORT 7379

Database* db;
sem_t semaphore;

void exit_with_error(const char * msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void add_duration(char* key, int deadline){
    int hash = hashFunction(db, key);

    Node* head = db->arr[hash];
    while (head != NULL) {
        
        if (!strcmp(head->key,key)) {
            time_t start_time;
            time(&start_time);

            head -> deadline = deadline;
            head -> start_time = start_time;
            break;
        }
        head = head->next;
    }
    return;
}

char* check_for_presence_and_time(char* key){
    //printf("DEBUG, ENTERING check FUNCTION\n");
    if(sem_wait(&semaphore) == -1)exit_with_error("ERROR IN SEM_WAIT"); 
    
    char* value = search(db, key);

    if(sem_post(&semaphore) == -1)exit_with_error("ERROR IN SEM_POST");
    
    return value;

}

char* set_for_key_and_value(char* key,char* value){
    //printf("DEBUG, ENTERING set FUNCTION\n");
    if(sem_wait(&semaphore) == -1)exit_with_error("ERROR IN SEM_WAIT");
    insert(db, key, value);
    if(sem_post(&semaphore) == -1)exit_with_error("ERROR IN SEM_POST");
    char* response = malloc(5);
    response = "+OK\r\n\0";
    return response;
}

char* analize(const char* command){

    char copy[strlen(command) + 1];
    strcpy(copy, command);
    
    char* supp; //support string used for finding substring

    char* last; //last occurrence of character
    char* key;   //key to insert or to verify
    char* value; //value to insert or to verify
    char* TOKEN = "\r\n";   //token used in strtok
    char* response = NULL; //pointer to response of check_for_presence_and_time or set_for_key_and_value
    
    
    
    int size; //size of key
    int iteration; //used to count the iteration of the strtok:
    
    printf("%s\n", copy);
    supp = strstr(copy, "GET");
    if(supp != NULL){
        
        supp = strtok(copy, TOKEN);
        iteration = 0;
        while(supp != NULL){
            
            if(iteration == 3){
                char* supp_pointer = supp;
                supp_pointer++;
                size = atoi(supp_pointer);
                key = malloc(size + 1);
            }
            if(iteration == 4){
                strcpy(key, supp);
                printf("value of key: %s\n", key);
                response = check_for_presence_and_time(key);
            }

            printf("value: %s\n", supp);
            ++iteration;
            supp = strtok(NULL, TOKEN);
        }

        free(key);
        return response;

    }

    supp = strstr(copy, "SET\r\n");

    if(supp != NULL){
        //$3 key $5 value $2 EX $2 10
        last = strchr(supp, '$');   //first occurrence of 
        
        supp = strtok(last, TOKEN);
        iteration = 0;
        while(supp != NULL){
            
            if(iteration == 0){
                char* supp_pointer = supp;
                supp_pointer++;
                size = atoi(supp_pointer);
                key = malloc(size + 1); //I create a buffer for the key
            }
            
            if(iteration == 1){
                strcpy(key, supp);
            }

            if(iteration == 2){
                char* supp_pointer = supp;
                supp_pointer++;
                size = atoi(supp_pointer);
                value = malloc(size + 1); //I create a buffer for the value
            }
            if(iteration == 3){
                strcpy(value, supp);
                printf("value of key: %s value of value: %s\n", key, value);
                response = set_for_key_and_value(key, value);   
                
            }
            
            if(iteration == 7){
                int deadline = atoi(supp);
                add_duration(key, deadline);
            }

            printf("value: %s\n", supp);
            ++iteration;
            supp = strtok(NULL, TOKEN);
        }

        
        return response;

    }
    
    response = malloc(5);
    response = "+OK\r\n\0";
    return response;

}

void connection_handler(int client_socket){
    int ret;
    char buffer[1024];
    memset(buffer, 0, 1024);
    char* response;

    while(1){

            ret = read(client_socket, buffer, 1024);

            //printf("[SERVER] received...\n");

            //printf("buffer:\n%s\n", buffer);

            response = analize(buffer);

            //cleaning buffer used    
            memset(buffer, 0, 1024);

            send(client_socket, response, strlen(response), 0);

            //printf("[SERVER] %s message sent\n", response);    
    }
    return;
}

// https://redis.io/docs/reference/protocol-spec/

int main(void) {

    int ret;
    db = malloc(sizeof(Database));
    initializeDatabase(db);
    sem_init(&semaphore, 1, 1);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)exit_with_error("CANNOT CREATE SERVER SOCKET");
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    int opt = 1;

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,sizeof(opt)) < 0)
    exit_with_error("CANNOT LINK SOCKET SO PORT");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0)exit_with_error("BIND FAILED!");

    if (listen(server_socket, 3) < 0)exit_with_error("LISTEN FAILED!");

    printf("Server listening...\n");

    int client_socket;

    while(1){
        client_socket = accept(server_socket, (struct sockaddr *)&address, &addrlen);
        if(client_socket < 0)exit_with_error("ACCEPT FAILED!");
        printf("CONNECTION ESTABLISHED!\n");
        
        pid_t pid = fork();
        if(pid < 0){
            exit_with_error("ERROR IN FORK");
        }

        if(pid == 0){
            ret = close(server_socket);
            if(ret == -1)exit_with_error("ERROR IN CLOSING SERVER SOCKET");
            connection_handler(client_socket);
            close(client_socket);
            if(ret == -1)exit_with_error("ERROR IN CLOSING CLIENT SOCKET");
            _exit(0);
        }

        memset(&address, 0, addrlen);
        ret = close(client_socket);
        if(ret == -1)exit_with_error("ERROR IN CLOSING CLIENT SOCKET");
    }

    return 0;

    // closing the connected socket

    // Open Socket and receive connections

    // Keep a key, value store (you are free to use any data structure you want)

    // Create a process for each connection to serve set and get requested
}

