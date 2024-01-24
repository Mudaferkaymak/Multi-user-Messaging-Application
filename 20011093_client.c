// Client side C/C++ program to demonstrate Socket programming
#include <arpa/inet.h>
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 2048
#define PORT 8080


// Function to handle receiving messages in a separate thread
void *receive_messages(void *arg) {
    int client_fd = *((int *)arg);
    char buffer[BUFFER_SIZE];

    while (1) {
        ssize_t valread = read(client_fd, buffer, sizeof(buffer));

        // Print the received message
        printf("Server says: %s\n", buffer);
    }

    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
    int status, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to the server.\n");

    // Read login message from the server
    read(client_fd, buffer, sizeof(buffer));
    printf("%s\n", buffer);

    // Log in with a username
    printf("Enter your username: ");
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = '\0';  // Remove newline character
    send(client_fd, message, strlen(message), 0);

    // Read the client ID from the server
    read(client_fd, buffer, sizeof(buffer));
    printf("%s\n", buffer);

    // Create a thread for receiving messages
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, (void *)&client_fd) != 0) {
        perror("pthread_create");
        return -1;
    }

    while (1) {
    printf("Enter your choice (1: Send, 3: Exit): ");
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = '\0';  // Remove newline character

    char command[2 * BUFFER_SIZE];  // Increase buffer size
    if (strcmp(message, "3") == 0) {
        snprintf(command, sizeof(command), "/exit");
        send(client_fd, command, strlen(command), 0);
        break;
    } else if (strcmp(message, "1") == 0) {
        // Send private message
        int target_client_id;
        printf("Enter the recipient's client ID: ");
        scanf("%d", &target_client_id);
        getchar();  

        printf("Enter your private message: ");
        fgets(message, sizeof(message), stdin);

        // Send the private message command to the server
        snprintf(command, sizeof(command), "/private %d %s", target_client_id, message);
        send(client_fd, command, strlen(command), 0);
    } else {
        printf("Invalid choice. Try again.\n");
    }
}

    // Close the connection
    close(client_fd);

    // Wait for the receive thread to finish
    pthread_join(receive_thread, NULL);

    return 0;
}