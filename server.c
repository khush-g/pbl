#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <limits.h>

#define MAX_BLOCKS 10
#define BUFFER_SIZE 4096
#define MAX_PROCESSES 20
#define MAX_PROCESS_NAME 20

typedef struct {
    int size;               // Total size of the block
    int requested_size;     // Actual size requested by the process
    int used;
    char process[MAX_PROCESS_NAME];
    time_t allocation_time;
} MemoryBlock;

typedef struct {
    char name[MAX_PROCESS_NAME];
    int size;
    int allocated_block;
    time_t start_time;
} ProcessInfo;

MemoryBlock memory[MAX_BLOCKS] = {
    {100, 0, 0, "", 0}, {150, 0, 0, "", 0}, {200, 0, 0, "", 0},
    {250, 0, 0, "", 0}, {300, 0, 0, "", 0}, {180, 0, 0, "", 0},
    {120, 0, 0, "", 0}, {220, 0, 0, "", 0}, {170, 0, 0, "", 0}, {190, 0, 0, "", 0}
};

ProcessInfo processes[MAX_PROCESSES] = {0};
int process_count = 0;

void initialize_memory() {
   //srand((unsigned int)time(NULL));
    for (int i = 0; i < MAX_BLOCKS; i++) {
        memory[i].size = 100 + (rand() % 201);
        memory[i].requested_size = 0;
        memory[i].used = 0;
        strcpy(memory[i].process, "");
        memory[i].allocation_time = 0;
    }
}

void reset_memory() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        memory[i].used = 0;
        memory[i].requested_size = 0;
        strcpy(memory[i].process, "");
        memory[i].allocation_time = 0;
    }
    process_count = 0;
}

int first_fit(int size, const char *process) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!memory[i].used && memory[i].size >= size) {
            memory[i].used = 1;
            memory[i].requested_size = size;
            strncpy(memory[i].process, process, MAX_PROCESS_NAME - 1);
            memory[i].process[MAX_PROCESS_NAME - 1] = '\0';
            memory[i].allocation_time = time(NULL);
            
            if (process_count < MAX_PROCESSES) {
                strncpy(processes[process_count].name, process, MAX_PROCESS_NAME - 1);
                processes[process_count].name[MAX_PROCESS_NAME - 1] = '\0';
                processes[process_count].size = size;
                processes[process_count].allocated_block = i;
                processes[process_count].start_time = time(NULL);
                process_count++;
            }
            return i;
        }
    }
    return -1;
}

int best_fit(int size, const char *process) {
    int best_index = -1;
    int smallest_diff = INT_MAX;
    
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!memory[i].used && memory[i].size >= size) {
            int current_diff = memory[i].size - size;
            if (current_diff < smallest_diff) {
                smallest_diff = current_diff;
                best_index = i;
            }
        }
    }
    
    if (best_index != -1) {
        memory[best_index].used = 1;
        memory[best_index].requested_size = size;
        strncpy(memory[best_index].process, process, MAX_PROCESS_NAME - 1);
        memory[best_index].process[MAX_PROCESS_NAME - 1] = '\0';
        memory[best_index].allocation_time = time(NULL);
        
        if (process_count < MAX_PROCESSES) {
            strncpy(processes[process_count].name, process, MAX_PROCESS_NAME - 1);
            processes[process_count].name[MAX_PROCESS_NAME - 1] = '\0';
            processes[process_count].size = size;
            processes[process_count].allocated_block = best_index;
            processes[process_count].start_time = time(NULL);
            process_count++;
        }
    }
    return best_index;
}

int worst_fit(int size, const char *process) {
    int worst_index = -1;
    int largest_size = -1;
    
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!memory[i].used && memory[i].size >= size) {
            if (memory[i].size > largest_size) {
                largest_size = memory[i].size;
                worst_index = i;
            }
        }
    }
    
    if (worst_index != -1) {
        memory[worst_index].used = 1;
        memory[worst_index].requested_size = size;
        strncpy(memory[worst_index].process, process, MAX_PROCESS_NAME - 1);
        memory[worst_index].process[MAX_PROCESS_NAME - 1] = '\0';
        memory[worst_index].allocation_time = time(NULL);
        
        if (process_count < MAX_PROCESSES) {
            strncpy(processes[process_count].name, process, MAX_PROCESS_NAME - 1);
            processes[process_count].name[MAX_PROCESS_NAME - 1] = '\0';
            processes[process_count].size = size;
            processes[process_count].allocated_block = worst_index;
            processes[process_count].start_time = time(NULL);
            process_count++;
        }
    }
    return worst_index;
}
int deallocate_process(const char *process) {
    int found = 0;

    // Deallocate all memory blocks associated with the process
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (memory[i].used && strcmp(memory[i].process, process) == 0) {
            memory[i].used = 0;
            memory[i].requested_size = 0;
            strcpy(memory[i].process, "");
            memory[i].allocation_time = 0;
            found = 1;
        }
    }

    // If any memory block was freed, remove all matching entries from process list
    if (found) {
        for (int j = 0; j < process_count; ) {
            if (strcmp(processes[j].name, process) == 0) {
                // Shift the array left to remove the entry
                for (int k = j; k < process_count - 1; k++) {
                    processes[k] = processes[k + 1];
                }
                process_count--;
                // Do not increment j, because we need to check the new value at j
            } else {
                j++;
            }
        }
    }

    return found;
}



void send_response(SOCKET client, const char* status, const char* content_type, const char* body) {
    char header[BUFFER_SIZE];
    int body_length = (int)strlen(body);
    
    snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, content_type, body_length);
    
    send(client, header, (int)strlen(header), 0);
    send(client, body, body_length, 0);
}

void check_fragmentation(char* response) {
    int total_memory = 0;
    int used_memory = 0;
    int external_frag = 0;
    int internal_frag = 0;
    int largest_free_block = 0;
    int total_free = 0;
    
    for (int i = 0; i < MAX_BLOCKS; i++) {
        total_memory += memory[i].size;
        
        if (memory[i].used) {
            used_memory += memory[i].size;
            internal_frag += memory[i].size - memory[i].requested_size;
        } else {
            total_free += memory[i].size;
            if (memory[i].size > largest_free_block) {
                largest_free_block = memory[i].size;
            }
        }
    }
    
    external_frag = total_free - largest_free_block;
    
    double frag_percentage = total_memory > 0 ? 
        (double)(external_frag + internal_frag) / total_memory * 100 : 0;
    
    snprintf(response, BUFFER_SIZE, 
        "{\"total_memory\":%d, \"used_memory\":%d, \"free_memory\":%d, "
        "\"external_frag\":%d, \"internal_frag\":%d, \"largest_free_block\":%d, "
        "\"fragmentation_percentage\":%.2f}",
        total_memory, used_memory, total_free,
        external_frag, internal_frag, largest_free_block, frag_percentage);
}

void show_memory_status(char* response) {
    char buffer[BUFFER_SIZE] = "{\"blocks\":[";
    char temp[512];
    
    for (int i = 0; i < MAX_BLOCKS; i++) {
        time_t now = time(NULL);
        double hours_allocated = memory[i].allocation_time > 0 ? 
            difftime(now, memory[i].allocation_time) / 3600.0 : 0;
        
        snprintf(temp, sizeof(temp), 
            "%s{\"id\":%d,\"size\":%d,\"used\":%d,\"process\":\"%s\","
            "\"allocation_time\":%ld,\"hours_allocated\":%.2f}",
            i > 0 ? "," : "", i, memory[i].size, memory[i].used, 
            memory[i].process, memory[i].allocation_time, hours_allocated);
        
        strncat(buffer, temp, BUFFER_SIZE - strlen(buffer) - 1);
    }
    
    strncat(buffer, "]}", BUFFER_SIZE - strlen(buffer) - 1);
    strncpy(response, buffer, BUFFER_SIZE);
}

void show_processes(char* response) {
    char buffer[BUFFER_SIZE] = "{\"processes\":[";
    char temp[512];
    
    for (int i = 0; i < process_count; i++) {
        time_t now = time(NULL);
        double hours_running = difftime(now, processes[i].start_time) / 3600.0;
        
        snprintf(temp, sizeof(temp), 
            "%s{\"name\":\"%s\",\"size\":%d,\"block\":%d,\"start_time\":%ld,"
            "\"hours_running\":%.2f}",
            i > 0 ? "," : "", processes[i].name, processes[i].size, 
            processes[i].allocated_block, processes[i].start_time, hours_running);
        
        strncat(buffer, temp, BUFFER_SIZE - strlen(buffer) - 1);
    }
    
    strncat(buffer, "]}", BUFFER_SIZE - strlen(buffer) - 1);
    strncpy(response, buffer, BUFFER_SIZE);
}

void compact_memory() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!memory[i].used) {
            for (int j = i + 1; j < MAX_BLOCKS; j++) {
                if (memory[j].used) {
                    MemoryBlock temp = memory[i];
                    memory[i] = memory[j];
                    memory[j] = temp;
                    break;
                }
            }
        }
    }
}

void handle_client(SOCKET client) {
    char buffer[BUFFER_SIZE];
    int bytes_read = recv(client, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read <= 0) {
        closesocket(client);
        return;
    }
    
    buffer[bytes_read] = '\0';
    
    char method[16], path[256];
    sscanf(buffer, "%15s %255s", method, path);
    
    if (strcmp(path, "/reset") == 0) {
        reset_memory();
        send_response(client, "200 OK", "application/json", "{\"status\":\"memory_reset\"}");
    } 
    else if (strstr(path, "/alloc?")) {
        int size;
        char strategy[20], process[MAX_PROCESS_NAME];
        
        if (sscanf(strstr(path, "alloc?") + 6, "size=%d&strategy=%19[^&]&process=%49s", 
                  &size, strategy, process) != 3) {
            send_response(client, "400 Bad Request", "application/json", 
                         "{\"error\":\"Missing or invalid parameters\"}");
            closesocket(client);
            return;
        }
        
        int result = -1;
        if (strcmp(strategy, "first-fit") == 0) {
            result = first_fit(size, process);
        } else if (strcmp(strategy, "best-fit") == 0) {
            result = best_fit(size, process);
        } else if (strcmp(strategy, "worst-fit") == 0) {
            result = worst_fit(size, process);
        } else {
            send_response(client, "400 Bad Request", "application/json", 
                         "{\"error\":\"Invalid strategy\"}");
            closesocket(client);
            return;
        }
        
        if (result != -1) {
            char response[256];
            snprintf(response, sizeof(response), 
                    "{\"status\":\"allocated\",\"block\":%d,\"size\":%d}", result, memory[result].size);
            send_response(client, "200 OK", "application/json", response);
        } else {
            send_response(client, "200 OK", "application/json", "{\"status\":\"failed\",\"reason\":\"no_suitable_block\"}");
        }
    } 
    else if (strstr(path, "/dealloc?")) {
        char process[MAX_PROCESS_NAME];
        if (sscanf(strstr(path, "dealloc?") + 8, "process=%49s", process) != 1) {
            send_response(client, "400 Bad Request", "application/json", 
                         "{\"error\":\"Missing process parameter\"}");
            closesocket(client);
            return;
        }
        
        int removed = deallocate_process(process);
        if (removed) {
            send_response(client, "200 OK", "application/json", "{\"status\":\"deallocated\"}");
        } else {
            send_response(client, "200 OK", "application/json", "{\"status\":\"not_found\"}");
        }
    } 
    else if (strcmp(path, "/memory") == 0) {
        char response[BUFFER_SIZE];
        show_memory_status(response);
        send_response(client, "200 OK", "application/json", response);
    } 
    else if (strcmp(path, "/fragmentation") == 0) {
        char response[BUFFER_SIZE];
        check_fragmentation(response);
        send_response(client, "200 OK", "application/json", response);
    } 
    else if (strcmp(path, "/processes") == 0) {
        char response[BUFFER_SIZE];
        show_processes(response);
        send_response(client, "200 OK", "application/json", response);
    } 
    else if (strcmp(path, "/compact") == 0) {
        compact_memory();
        send_response(client, "200 OK", "application/json", "{\"status\":\"compacted\"}");
    } 
    else {
        send_response(client, "404 Not Found", "application/json", "{\"error\":\"endpoint_not_found\"}");
    }
    
    closesocket(client);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        fprintf(stderr, "Setsockopt failed with error: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed with error: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed with error: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Memory Management Server running on port 8080...\n");
    initialize_memory();

    while (1) {
        SOCKET client = accept(server_fd, NULL, NULL);
        if (client == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed with error: %d\n", WSAGetLastError());
            continue;
        }
        
        handle_client(client);
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}