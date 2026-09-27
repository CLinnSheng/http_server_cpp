#include "request.h"
#include "util/parsers.h"
#include <arpa/inet.h>
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <server.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <thread>

// Initiliaze TCP Socket
HTTP_Server::HTTP_Server()
{
    // IPv4, TCP
    this->tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (this->tcp_socket == INVALID_SOCKET)
    {
        perror("Fail to initiliaze TCP Socket: socket()");
        return;
    }

    int enabled = 1;

    // Allow reuse of the same socket when restart
    setsockopt(this->tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));

    printf("TCP Socket Created Succesfully\n");
}

// Close the socket
HTTP_Server::~HTTP_Server()
{
    if (this->tcp_socket != -1)
    {
        close_socket(this->tcp_socket);
    }
}

void HTTP_Server::close_socket(int socket)
{
    if (socket == this->tcp_socket)
    {
        this->tcp_socket = -1;
    }

    close(socket);
}

// Starting the server
void HTTP_Server::start(int port)
{
    // Accept Connections from any network interfaces
    sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(port), .sin_addr = INADDR_ANY};

    // Bind the socket
    if (bind(this->tcp_socket, (const struct sockaddr *)&addr, sizeof(addr)) == INVALID_SOCKET)
    {
        perror("Fail to bind TCP Socket: bind()\n");
        close_socket(this->tcp_socket);
        return;
    }
    printf("TCP Socket bind succesfully\n");

    // Listen to the socket
    if (listen(this->tcp_socket, BACKLOG) == INVALID_SOCKET)
    {
        perror("Fail to listen to TCP Socket: listen()\n");
        close_socket(this->tcp_socket);
        return;
    }
    printf("Server is listening on port %d\n", port);

    // Accepting Client Connections
    printf("Waiting for Connection\n");
    while (true)
    {
        sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);

        int client_socket = accept(tcp_socket, (struct sockaddr *)&client_addr, &client_addr_size);

        // Extract IP & Port
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("Get a connection from %s\n", client_ip);

        // Accept and Handle Client Request
        // Spawn one detached thread per connection
        // The caller thread dont have to handle it
        std::thread([this, client_socket]() {
            handle_client(client_socket);
            close_socket(client_socket);
        }).detach();
    }

    // Cleaning up and close the TCP socket
    close_socket(this->tcp_socket);
}

void HTTP_Server::handle_client(Socket client_socket)
{
    while (true)
    {
        std::string raw_req = read_request(client_socket);
        Request req = util::parse_request(raw_req);

        send_response(client_socket);
    }

    printf("Connection closed gracefully\n");
}

void HTTP_Server::send_response(Socket client_socket)
{
    std::stringstream response;

    // Response Status Line & Each header is seperate by a CRLN or '\r\n'
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: 46\r\n"; // Length of the HTML content
    response << "\r\n";
    response << "<html><body><h1>Hello, World!</h1></body></html>";

    // Send the HTTP response to the client
    int bytes_sent = send(client_socket, response.str().c_str(), response.str().length(), 0);
    if (bytes_sent == INVALID_SOCKET)
    {
        perror("Fail to send response: send()\n");
    }
    else
    {
        printf("Sent %d to client\n", bytes_sent);
    }
};

std::string HTTP_Server::read_request(Socket client_socket)
{
    char input_buffer[1024];

    std::string request;

    // Leave room for the null terminator
    size_t bytes_received = recv(client_socket, input_buffer, sizeof(input_buffer) - 1, 0);

    // -1 -> Connection closed on error
    // 0 -> Connection closed by client
    if (bytes_received <= 0)
    {
        perror("Fail to read the bytes sent: recv()");
    }

    input_buffer[bytes_received] = '\0';
    request += input_buffer;

    // Check whether is it the end of header
    // if (request.find("\r\n\r\n") != std::string::npos)
    // {
    //     break;
    // }

    printf("Message Received: \n%s\n", input_buffer);

    return request;
}
