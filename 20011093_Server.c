// Server side C/C++ program to demonstrate Socket programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <asm-generic/socket.h>

#define PORT 8080
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1500

typedef struct {
    int client_socket;
    int client_id;
    char profile[50];
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];
int client_count = 0;

void *handle_client(void *client_info_ptr) {
    ClientInfo *client_info = (ClientInfo *)client_info_ptr;
    int client_socket = client_info->client_socket;
    int client_id = client_info->client_id;
    char buffer[1024] = {0};

    printf("Client %d connected.\n", client_id);

    // send login message to client 
    send(client_socket, "Enter your username using /login Username", 50, 0);

    // get client's username
    ssize_t valread = read(client_socket, buffer, sizeof(buffer));
    if (valread <= 0) {
        perror("read");
        close(client_socket);
        clients[client_id].client_socket = -1;
        pthread_exit(NULL);
    }

    // get login format and parse it
    if (strncmp(buffer, "/login", 6) == 0) {
        if (sscanf(buffer + 7, "%s", client_info->profile) == 1) {
            // Send the client ID to the client
            snprintf(buffer, sizeof(buffer), "Your client ID is: %d", client_id);
            send(client_socket, buffer, strlen(buffer), 0);
        }
    } else {
        // if login format is wrong ask client to login correctly
        send(client_socket, "Invalid login format. Use /login Username", 50, 0);
        close(client_socket);
        clients[client_id].client_socket = -1;
        pthread_exit(NULL);
    }

    while (1) {
        read(client_socket, buffer, sizeof(buffer));

        
            int target_client_id;
            char private_message[1024];

            
            if (sscanf(buffer + 9, "%d %[^\n]", &target_client_id, private_message) == 2 &&
                target_client_id >= 0 && target_client_id < client_count) {
                // Send the private message only to the specified client
                char private_broadcast[BUFFER_SIZE];
                snprintf(private_broadcast, sizeof(private_broadcast), "%s -%s", private_message, clients[client_id].profile);
                send(clients[target_client_id].client_socket, private_broadcast, strlen(private_broadcast), 0);
            } else {
                // Invalid private message format
                send(client_socket, "Invalid private message format. Use /private ID Message", 70, 0);
            }
            
    }

    // Client disconnected, remove it from the list
    close(client_socket);
    clients[client_id].client_socket = -1;
    printf("Client %d (%s) disconnected.\n", client_id, client_info->profile);

    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    pthread_t thread_id;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Create a new thread for each client
        ClientInfo client_info;
        client_info.client_socket = new_socket;
        client_info.client_id = client_count;

        // Assign a default profile 
        snprintf(client_info.profile, sizeof(client_info.profile), "User%d", client_count);

        clients[client_count] = client_info;
        client_count++;

        if (pthread_create(&thread_id, NULL, handle_client, (void *)&client_info) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }

        // Detach the thread to allow its resources to be released when it finishes
        pthread_detach(thread_id);
    }

    
    // close(server_fd);
    return 0;
}