//
// Usage:
//          client [IP_address_of_server [port_of_server]]
//      where IP_address_of_server is either IP number of server
//      or a symbolic Internet name, default is "localhost";
//      port_of_server is a port number, default is 1234.
//
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define TEXT_ENTER_MESSAGE "Enter a message: "
#define TEXT_WAITING_FOR_MESSAGE "Waiting for a message."
#define TEXT_SEPARATOR "--------------------------------------------"
const std::string CLOSE_CONNECTION_PROTOCOL = "Goodbye.";

#define TEXT_CLOSING_CONNECTION "Closing connection."
#define TEXT_SERVER_CLOSED_CONNECTION "Server closed connection."

static void usage();

int main(int argc, char *argv[]) {
    if (argc > 1 && *(argv[1]) == '-') {
        usage();
        exit(1);
    }

    // Create a socket
    int s0 = socket(AF_INET, SOCK_STREAM, 0);
    if (s0 < 0) {
        perror("Cannot create a socket"); exit(1);
    }

    // Fill in the address of server
    struct sockaddr_in peeraddr;
    int peeraddr_len;
    memset(&peeraddr, 0, sizeof(peeraddr));
    char* peerHost = (char *) "localhost";
    if (argc > 1)
        peerHost = argv[1];

    // Resolve the server address (convert from symbolic name to IP number)
    struct hostent *host = gethostbyname(peerHost);
    if (host == NULL) {
        perror("Cannot define host address"); exit(1);
    }
    peeraddr.sin_family = AF_INET;
    short peerPort = 1234;
    if (argc >= 3)
        peerPort = (short) atoi(argv[2]);
    peeraddr.sin_port = htons(peerPort);

    // Print a resolved address of server (the first IP of the host)
    printf(
        "peer addr = %d.%d.%d.%d, port %d\n",
        host->h_addr_list[0][0] & 0xff,
        host->h_addr_list[0][1] & 0xff,
        host->h_addr_list[0][2] & 0xff,
        host->h_addr_list[0][3] & 0xff,
        (int) peerPort
    );

    // Write resolved IP address of a server to the address structure
    memmove(&(peeraddr.sin_addr.s_addr), host->h_addr_list[0], 4);

    // Connect to a remote server
    int res = connect(s0, (struct sockaddr*) &peeraddr, sizeof(peeraddr));
    if (res < 0) {
        perror("Cannot connect"); exit(1);
    }
    std::cout << "Connected. Starting session." << std::endl << TEXT_SEPARATOR << std::endl;

    char buffer[1024];
    std::string input;
    // Main chat loop
    while (true)
    {
        std::cout << TEXT_WAITING_FOR_MESSAGE << std::endl;
        res = recv(s0, buffer, 1023, 0);
        if (res < 0) {
            perror("Read error");
            exit(1);
        }
        buffer[res] = 0;
        printf("Received message (%d bytes): %s\n", res, buffer);
        if (strcmp(buffer, CLOSE_CONNECTION_PROTOCOL.c_str()) == 0)
        {
            close(s0);          // Close the data socket
            std::cout << std::endl << TEXT_SERVER_CLOSED_CONNECTION << std::endl << TEXT_SEPARATOR << std::endl;
            break;
        }

        std::cout << TEXT_ENTER_MESSAGE;
        std::getline(std::cin, input);
        if (input == CLOSE_CONNECTION_PROTOCOL)
        {
            send(s0, CLOSE_CONNECTION_PROTOCOL.c_str(), CLOSE_CONNECTION_PROTOCOL.length(), 0);
            std::cout << std::endl << TEXT_CLOSING_CONNECTION << std::endl << TEXT_SEPARATOR << std::endl;
            close(s0);          // Close the data socket
            break;
        }
        send(s0, input.c_str(), input.length(), 0);
    }

    close(s0);
    return 0;
}

static void usage() {
    std::cout <<
        "A simple Internet client application.\n"
        "Usage:\n"
        "         client [IP_address_of_server [port_of_server]]\n"
        "     where IP_address_of_server is either IP number of server\n"
        "     or a symbolic Internet name, default is \"localhost\";\n"
        "     port_of_server is a port number, default is 1234.\n"
        "The client connects to a server which address is given in a\n"
        "command line, receives a message from a server, sends the message\n"
        "\"Thanks! Bye-bye...\", and terminates.\n"
    ;
}