#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define PORT 4012 

struct addrinfo request, *result;

int main(int argc, char **argv)
{
    char *hostname = argv[1]; 

    //create a socket
    int network_socket;
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    //specify an address for the socket
    struct sockaddr_in server_address;
    request.ai_family = AF_INET;

    //grab the ip address for the server
    getaddrinfo(hostname, "4012", &request, &result);

    //store the ip address data into a readable form so it can be used as a connection address
    struct addrinfo *p;
    char ip_addr[256];
    for(p = result; p != NULL; p = p->ai_next)
    {
        getnameinfo(p->ai_addr, p->ai_addrlen, ip_addr, sizeof(ip_addr), NULL, 0, NI_NUMERICHOST);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(4012);
    server_address.sin_addr.s_addr = inet_addr(ip_addr);

    int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    //check for error with connection
    if(connection_status == -1)
    {
        printf("There was an error making a connection to the remote socket \n\n");
        return 1;
    }
    //recieve data from the server
    char server_response[256];
    recv(network_socket, &server_response, sizeof(server_response), 0);

    //print out the server's response
    printf("The server sent the data: %s\n", server_response);

    freeaddrinfo(result);

    while(1)
    {
        //send messages/commands to server and recieve responses
        char command[200];
        char return_message[200];
        printf("Enter a command: \n");
        fgets(command, 200, stdin);
        send(network_socket, command, sizeof(command), 0);
        if(strcmp(command, "exit\n") == 0)
        {   
            //if "exit" is typed it closes the connection with the server
            close(network_socket);
            return 0;
        }
        recv(network_socket, return_message, sizeof(return_message), 0);
        printf("%s\n", return_message);
        memset(return_message, 0, sizeof(return_message)); //refreshes the return

        
    }
    return 0;
}

