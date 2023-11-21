// Nicholas Lai 251096192

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

/**
 * Struct that represents an Account 
*/
struct Account
{
    char name[10]; // account name 
    int balance; // account balance 
    pthread_mutex_t lock; // mutex lock 
};

/**
 * Struct that represents a Client 
*/
struct Client
{
    char name[10]; // client name 
    char transactions[255]; // transactions that need to be completed 
};

struct Account accounts[100]; // struct array to hold the account structs 
struct Client clients[100]; // struct array to hold the client structs 
int numAccounts = 0; // Number of Accounts
int numClients = 0;  // Number of Clients

/**
 * transactionHandler Deposits or Withdraws amount based on given information 
*/
void *transactionHandler(void *clientArg)
{
    struct Client *client = clientArg; // Client struct with client name and transaction info 
    char *token; // token for each chunk of strings 
    token = strtok(client->transactions, " ");
    int accountIndex; // index of account in the accounts struct array 

    while (token != NULL)
    {
        if (strcmp(token, "deposit") == 0) // deposits money 
        { 
            char *accountName = strtok(NULL, " "); // account name for transaction
            for (int i = 0; i < numAccounts; i++)
            {
                if (strcmp(accounts[i].name, accountName) == 0) // find matching account index of account name 
                {
                    accountIndex = i; // stores account index
                }
            }
            pthread_mutex_lock(&accounts[accountIndex].lock); // lock critical section 
            char *amount = strtok(NULL, " "); // stores amount of transaction 
            accounts[accountIndex].balance += atoi(amount); // adds amount to account balance 
        }
        else if (strcmp(token, "withdraw") == 0) // withdraws money
        {
            char *accountName = strtok(NULL, " "); // account name for transaction
            for (int i = 0; i < numAccounts; i++)
            {
                if (strcmp(accounts[i].name, accountName) == 0) // find matching account index of account name 
                {
                    accountIndex = i; // stores account index 
                }
            }
            pthread_mutex_lock(&accounts[accountIndex].lock); // lock critical section 
            char *amount = strtok(NULL, " "); // stores amount of transaction 
            if ((accounts[accountIndex].balance - atoi(amount)) < 0) // ignore transaction if balance goes into the negative after withdrawal
            {
            }
            else
            {
                accounts[accountIndex].balance -= atoi(amount); // adds amount from account balance 
            }
        }
        pthread_mutex_unlock(&accounts[accountIndex].lock); // unlock critical section
        token = strtok(NULL, " ");                          // go to next chunk of transactions
    }
    return 0;
}

int main()
{
    char inFile[] = "assignment_5_input.txt"; // Input File Name
    FILE *file = fopen(inFile, "r");          // Open File

    if (file == NULL) // throw error if file cannot open
    {
        printf("Error: Could not open input file.\n");
        exit(1);
    }

    char line[255]; // char array to store read line from input file
    void *ret;      // return variable for thread joins

    while (fgets(line, sizeof(line), file) != NULL)
    {

        if (line[0] == 'A') // count number of accounts
        {
            numAccounts++;
        }
        else if (line[0] == 'C') //count number of clients
        {
            numClients++;
        }
    }

    printf("No. of Accounts: %d\n", numAccounts); // print number of accounts
    printf("No. of Clients: %d\n", numClients); // print number of clients 
    fseek(file, 0, SEEK_SET); // go back to beginning of file 

    for (int i = 0; i < numAccounts; i++)
    {
        fgets(line, sizeof(line), file);
        char *token = strtok(line, " "); // gets first token (chunk) of line

        strncpy(accounts[i].name, token, sizeof(accounts[i].name)); // store account name 

        for (int j = 0; j < 2; j++)
        {
            token = strtok(NULL, " ");
            if (j == 1)
            {
                accounts[i].balance = atoi(token); // store initial account balance 
            }
        }

        pthread_mutex_init(&accounts[i].lock, NULL); // intialize mutex lock
    }

    for (int i = 0; i < numClients; i++)
    {
        fgets(line, sizeof(line), file);
        char *token = strtok(line, " "); // gets first token (chunk) of line
        strncpy(clients[i].name, token, sizeof(clients[i].name)); // store client name 
        token = strtok(NULL, "\n"); // retrieve transaction information
        strncpy(clients[i].transactions, token, sizeof(clients[i].transactions)); // store transaction information
    }
    fclose(file); // close file

    pthread_t threads[numClients]; // creates an array of threads to store client threads 
    for (int i = 0; i < numClients; i++)
    {
        if (pthread_create(&threads[i], NULL, transactionHandler, (void *)&clients[i]) != 0) // create a thread for each client
        {
            printf("Error while creating thread %d\n", i);
            exit(1);
        }
    }
    for (int i = 0; i < numClients; i++)
    {
        if (pthread_join(threads[i], &ret) != 0) // join threads 
        {
            perror("pthread_join() error on thread");
            exit(3);
        }
    }
    for (int i = 0; i < numAccounts; i++)
    {
        printf("%s balance %d\n", accounts[i].name, accounts[i].balance); // print final balances
    }

    for (int i = 0; i < numAccounts; ++i)
    {
        pthread_mutex_destroy(&accounts[i].lock); // destroy mutex 
    }

    return 0;
}
