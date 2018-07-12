#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
 
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>

// C/C++ program to print numbers
// from 1 to 10 using goto statement


// function to print numbers from 1 to 10
void printNumbers()
{
    int n = 1;
label:
    printf("%d\n", n);
    n++;
    if (n <= 10)
        goto label;
}

// Driver program to test above function
int main()
{
    printNumbers();
    return 0;
}