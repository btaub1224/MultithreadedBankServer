#include <stdbool.h>
typedef int boolean;
#define true 1
#define false 0


typedef struct BankAccount
{
    char *account_name;
    float balance;
    int the_socket;
    boolean in_session; 
} BankAccount;
typedef struct BankAccount* BankAccountPtr;