#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    printf("Going to start calling malloc.\n");
    for (int i = 0; i < 100000; i++)
    {
        int *ptr = malloc(sizeof(int));
        (*ptr)++;
    }
    return EXIT_SUCCESS;
}
