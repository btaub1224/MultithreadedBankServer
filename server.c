#include "interface.h"
#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define PORT 4012
#define success 1
#define failure 0

BankAccountPtr accounts[20];
int account_num = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void print_and_send(char *message, int asocket)
{
    printf(message);
    send(asocket, message, strlen(message), 0);
}

int open_account(char *attempted_account, int asocket)
{   //opens account for client-given account name
    //mutex locked to prevent 2 clients from open the same account at the same time
    pthread_mutex_lock(&mutex);
    for(int i = 0; i < account_num; i++)
    {
        if(strcmp(accounts[i]->account_name, attempted_account) == 0)
        {
            print_and_send("This account name is already in use, please pick another one. \n", asocket);
            pthread_mutex_unlock(&mutex);
            return failure;
        }
    }
    BankAccountPtr new = malloc(sizeof(BankAccountPtr));
    new->account_name = strdup(attempted_account);
    new->balance = 0;
    new->in_session = false;
    accounts[account_num] = new;
    printf("New Account: %s\n", accounts[account_num]->account_name);
    account_num++;
    print_and_send("Account successfully created. \n", asocket);
    pthread_mutex_unlock(&mutex);
    return success;
}

int start(char *attempted_account, int asocket)
{   //starts session of client-given account name, provided the account has already been opened
    //this function is mutex locked to make sure 2 clients don't start a session of the same accoutn at the same time
    pthread_mutex_lock(&mutex);
   for(int i = 0; i < account_num; i++)
    {
        if(strcmp(accounts[i]->account_name, attempted_account) == 0)
        {
            if(accounts[i]->in_session)
            {
                print_and_send("User is already logged in.\n", asocket);
                pthread_mutex_unlock(&mutex);
                return failure;
            }
            print_and_send("Login successful. \n", asocket);
            accounts[i]->the_socket = asocket;
            accounts[i]->in_session = true;
            pthread_mutex_unlock(&mutex);
            return success;
        }
    }
    print_and_send("That account name is not in the system. \n", asocket);
    pthread_mutex_unlock(&mutex);
    return failure;
}

int deposit(char *attempted_account, float deposited_amount)
{   //deposits client-given amount to account
    for(int i = 0; i < account_num; i++)
    {
        if(strcmp(accounts[i]->account_name, attempted_account) == 0)
        {
             if(accounts[i]->in_session == true)
             {
                 accounts[i]->balance += deposited_amount;
                 print_and_send("Deposit successful. \n", accounts[i]->the_socket);
                 return success;
             }
             else
             {
                 print_and_send("Account has not logged in. \n", accounts[i]->the_socket);
                 return failure;
             }
        }
    }
    return failure;
}

int withdraw(char *attempted_account, float withdrawl_amount)
{   //withdraws the client-given amount from the account, provided the account has enough money in it
    for(int i = 0; i < account_num; i++)
    {
        if(strcmp(accounts[i]->account_name, attempted_account) == 0)
        {
             if(accounts[i]->in_session == true)
             {
                 if(accounts[i]->balance >= withdrawl_amount)
                 {
                     accounts[i]->balance -= withdrawl_amount;
                     print_and_send("Withdrawl successful. \n", accounts[i]->the_socket);
                     return success;
                 }
                 else
                 {
                     print_and_send("Insufficient funds in this account. \n", accounts[i]->the_socket);
                     return failure;
                 }
             }
             else
             {
                 print_and_send("Account has not logged in. \n", accounts[i]->the_socket);
                 return failure;
             }
        }
    }
    return failure;
}

int balance(char *attempted_account)
{   //prints the balances and sends it to the client 
    for(int i = 0; i < account_num; i++)
    {
        if(strcmp(accounts[i]->account_name, attempted_account) == 0)
        {
             if(accounts[i]->in_session == true)
             {
                 char buffer[50]; 
                 sprintf(buffer, "$%.2f \n", accounts[i]->balance);
                 print_and_send(buffer, accounts[i]->the_socket);
                 return success;
             }
             else
             {
                 print_and_send("Account has not logged in. \n", accounts[i]->the_socket);
                 return failure;
             }
        }
    }
    return failure;
}

int finish(char *attempted_account)
{   //switches insession on logged in account to false and resets the socket connected to the account
    for(int i = 0; i < account_num; i++)
    {
        if(strcmp(accounts[i]->account_name, attempted_account) == 0)
        {
             if(accounts[i]->in_session == true)
             {
                 accounts[i]->in_session = false;
                 print_and_send("Logout successful. \n", accounts[i]->the_socket);
                 accounts[i]->the_socket = 0;
                 return success;
             }
             else
             {
                 printf("Account has not logged in. \n");
                 return failure;
             }
        }
    }
    return failure;
}

void commands(char command[], int asocket)
{
    //splits the command into tokens and then put is into a char array that can be passed to the bank functions
    char *split[3];
    size_t n = 0;
    char command_copy[200];
    strcpy(command_copy, command);
    for(char *token = strtok(command_copy, " \n"); token; token = strtok(NULL, " "))
    {
        if(n >= 3)
        {
            break;
        }
        split[n++] = token;
    }
    //checks what the command is and calls the appropiate bank function
    if(strcmp(split[0], "open") == 0)
    {
        if(strlen(split[1]) > 100)
        {
            printf("Account name is greater than 100 characters.\n");
        }
        else
        {
            open_account(split[1], asocket);
        }
    }
    else if(strcmp(split[0], "start") == 0)
    {
        start(split[1], asocket);
    }
    else if(strcmp(split[0], "deposit") == 0)
    {
        for(int i = 0; i < account_num; i++)
        {   //determines what account to do the bank function on based on which socket is connected to said account and 
            //what socket is trying to call the bank function 
            if(accounts[i]->the_socket - asocket == 0)
            {
                deposit(accounts[i]->account_name, atof(split[1]));
            }
        }
        
    }
    else if(strcmp(split[0], "withdraw") == 0)
    {
        for(int i = 0; i < account_num; i++)
        {
            if(accounts[i]->the_socket - asocket == 0)
            {
                withdraw(accounts[i]->account_name, atof(split[1]));
            }
        }
        
    }
    else if(strcmp(split[0], "balance") == 0)
    {
        for(int i = 0; i < account_num; i++)
        {
            if(accounts[i]->the_socket - asocket == 0)
            {
                balance(accounts[i]->account_name);    
            }
        }
    }       
    else if(strcmp(split[0], "finish") == 0)
    {
        for(int i = 0; i < account_num; i++)
        {
            if(accounts[i]->the_socket - asocket == 0)
            {
                finish(accounts[i]->account_name);
            }
        }
    }            
    else
    {
        print_and_send("Invalid command \n", asocket);
    }

}
void* thread_helper(void* asocket)
{
    int user_socket = *(int *) asocket;
    char server_message[256] = "You have reached the server";
    //send the message
    send(user_socket, server_message, sizeof(server_message), 0);
    printf("Sent message\n");

    while(1)
    {
        //waits for command from client and if they don't exit pass the command to the command helper function
        char sent_command[200];
        recv(user_socket, &sent_command, sizeof(sent_command), 0);
        printf("Received message: %s \n", sent_command);
         if(strcmp(sent_command, "exit\n") == 0)
        {
             pthread_exit(0);
        }
        commands(sent_command, user_socket);
        sleep(2);
    }

}

int main()
{
    //create the server socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    //define the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(4012);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //bind the socket to our specified IP and port
    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    listen(server_socket, 20);
    printf("Server Start!\n");

    while(1)
    {
        //accepts a client connection, creates a thread for it and passes it to the thread helper function
        int client_socket; 
        client_socket = accept(server_socket, NULL, NULL);
        pthread_t tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tid, &attr, thread_helper, &client_socket);
        
    
    }
    

    return 0;
}