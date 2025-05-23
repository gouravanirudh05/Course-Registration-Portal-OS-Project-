#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
// Function prototypes
void send_to_client(int s_fd,  char *message);
void recieve(int s_fd, char *buffer);

int main() {
    int s_fd;
    struct sockaddr_in server_addr;
    char buffer[1024];
    
    // Create socket
    if ((s_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    
    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(1);
    }
    
    // Connect to server
    if (connect(s_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }
    
    printf("Connected to server\n");
    
    // Main loop for communication
    while (1) {
        // Receive message from server
        memset(buffer, 0, 1024);
        if (read(s_fd, buffer, 1024 - 1) <= 0) {
            printf("Server disconnected\n");
            break;
        }
        
        // Print message from server
        printf("%s", buffer);
        
        // Check if server is waiting for input
        if (strstr(buffer, ": ") != NULL && buffer[strlen(buffer) - 1] != '\n') {
            // Get input from user
            fgets(buffer, 1024, stdin);
            
            // Send input to server
            if (write(s_fd, buffer, strlen(buffer)) <= 0) {
                perror("Failed to send message");
                break;
            }
        }
    }
    
    // Close socket
    close(s_fd);
    
    return 0;
}

// Send message to server
void send_to_client(int s_fd,  char *message) {
    if (write(s_fd, message, strlen(message)) == -1) {
        perror("Failed to send message");
    }
}

// Receive message from server
void recieve(int s_fd, char *buffer) {
    int content = read(s_fd, buffer, 1024 - 1);
    if (content <= 0) {
        if (content == 0) {
            // Server disconnected
            printf("Server disconnected\n");
        } else {
            perror("Failed to receive message");
        }
        buffer[0] = '\0';
        return;
    }
    
    // Null-terminate the buffer
    buffer[content] = '\0';
}
