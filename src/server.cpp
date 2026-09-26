#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstdio>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX_CONNECTION 10000

int handle_client(int client_socket)
{
    char input_buffer[1024];
    size_t bytes_read = 0;

    bytes_read = read(client_socket, input_buffer, sizeof(input_buffer) - 1);

    if (bytes_read == -1)
    {
        perror("Fail to read the bytes sent: read()");
        return -1;
    }

    if (bytes_read == 0)
    {
        printf("Connection closed gracefully\n");
        return 0;
    }

    printf("Message Received: %s", input_buffer);

    return 1;
}

int main(int argc, char *argv[])
{
    // First Create a TCP Socket
    // IPv4, TCP
    // Return the file descriptor
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    int enabled = true;

    if (tcp_socket == -1)
    {
        perror("Fail creating TCP socket");
        return -1;
    }
    printf("TCP Socket Created Succesfully\n");

    setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));

    // Listen to all local IPv4 interfaces
    sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(8080), .sin_addr = INADDR_ANY};

    // Bind the TPC socket
    int bind_res = bind(tcp_socket, (const struct sockaddr *)&addr, sizeof(addr));

    if (bind_res == -1)
    {
        perror("Fail to bind TCP Socket: bind()\n");
        close(tcp_socket);
        return -1;
    }
    printf("TCP Socket bind Succesfully\n");

    // Listen on the TCP socket
    int list_res = listen(tcp_socket, MAX_CONNECTION);

    if (list_res == -1)
    {
        perror("Fail to listen to TCP Socket: listen()\n");
        close(tcp_socket);
        return -1;
    }
    printf("Successfully listen to the TCP Socket\n");

    printf("Waiting for Connection\n");
    while (true)
    {
        sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);

        int client_socket = accept(tcp_socket, (struct sockaddr *)&client_addr, &client_addr_size);

        // Extract IP & Port
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr, client_ip, sizeof(client_addr));
        printf("Get a connection from %s\n", client_ip);

        handle_client(client_socket);
    }

    return 0;
}
