#include "server.h"

int main(int argc, char *argv[])
{
    HTTP_Server server;

    server.start(8080);

    return 0;
}
