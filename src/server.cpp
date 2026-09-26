#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
    // First Create a TCP Socket
    // IPv4, TCP
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    return 0;
}
