#pragma once

#include "request.h"
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#define INVALID_SOCKET -1
#define BACKLOG 1000
using Socket = int;

class HTTP_Server
{
  private:
    Socket tcp_socket;

    void handle_client(Socket client_socket);
    void send_response(Socket client_socket);

    std::string read_request(Socket client_socket);

  public:
    HTTP_Server();
    ~HTTP_Server();

    void close_socket(int socket);
    void start(int port);
};
