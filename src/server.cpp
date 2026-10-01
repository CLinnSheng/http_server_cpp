#include "request.h"
#include "util/parsers.h"
#include <arpa/inet.h>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <server.h>
#include <sstream>
#include <string>
#include <string_view>
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
    std::string buffer;
    Request req;

    while (read_request(client_socket, buffer, req))
    {
        printf("%s %s %s\n", req.method.c_str(), req.path.c_str(), req.http_version.c_str());

        bool keep_alive = util::to_lower(req.headers["connection"]) == "keep-alive";
        send_response(client_socket, keep_alive);

        if (!keep_alive)
        {
            break;
        }
    }

    printf("Connection closed gracefully\n");
}

void HTTP_Server::send_response(Socket client_socket, bool keep_alive)
{
    std::ostringstream response;
    const std::string body = "<html><body><h1>Hello, World!</h1></body></html>";

    // Response Status Line & Each header is seperate by a CRLN or '\r\n'
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.size() << "\r\n "; // Length of the HTML content
    response << "Connection: " << (keep_alive ? "keep-alive" : "close") << "\r\n ";
    response << "\r\n";
    response << body;

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

bool HTTP_Server::read_request(Socket client_socket, std::string &buffer, Request &req)
{
    constexpr size_t MAX_HEADER = 8 * 1024;
    constexpr size_t MAX_BODY = 10 * 1024 * 1024;

    // Receive more bytes into the buffer. Returns false on close or error.
    auto recv_more = [&]() -> bool {
        char temp_buffer[4096];
        while (true)
        {
            ssize_t bytes_received = recv(client_socket, temp_buffer, sizeof(temp_buffer), 0);
            if (bytes_received > 0)
            {
                buffer.append(temp_buffer, bytes_received);
                return true;
            }
            if (bytes_received == 0)
            {
                return false; // client closed the connection normally
            }
            if (errno == EINTR)
            {
                continue; // interrupted by a signal, retry
            }
            perror("recv");
            return false;
        }
    };

    // Read until the end of the headers
    size_t header_end;
    while ((header_end = buffer.find("\r\n\r\n")) == std::string::npos)
    {
        if (buffer.size() > MAX_HEADER)
        {
            return false;
        }
        if (!recv_more())
        {
            return false;
        }
    }

    // Parse the headers, clearing the previous request first
    req = Request{};
    if (!util::parse_header(std::string_view(buffer).substr(0, header_end), req))
    {
        return false;
    }

    const size_t body_start = header_end + 4;

    // Work out the body length
    size_t content_length = 0;
    if (auto it = req.headers.find("content-length"); it != req.headers.end())
    {
        try
        {
            content_length = std::stoul(it->second);
        }
        catch (...)
        {
            return false; // invalid number, don't let it crash the server
        }

        if (content_length > MAX_BODY)
            return false;
    }

    // Read until the whole body has arrived
    while (buffer.size() < body_start + content_length)
    {
        if (!recv_more())
            return false;
    }

    req.body = buffer.substr(body_start, content_length);

    // Consume this request; leftover bytes belong to the next one
    buffer.erase(0, body_start + content_length);

    return true;
}
