# MultithreadedBankServer
Test bank server written in C, using multithreading and capable of being used over network from multiple PCs

The implementation of my bank server/client is based on socket connections that are then passed
through threads that handle individual clients at the same time using basic bank functions such as opening
an account, depositing, view the balance etc. 
Critical points such as opening an account or withdrawing cash is locked so deadlocks do not occur, such as 2 identical accounts being made, or money being overdrawn from an account.
