#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main(void)
{
    void *main_addr = &main;
    void *stack_ptr = &main_addr;
    void *heap_ptr = malloc(1);

    printf("Main is at %p\n", main_addr);
    printf("Stack is at %p\n", stack_ptr);
    printf("Heap is at %p\n", heap_ptr);

    assert( main_addr < heap_ptr );
    assert( heap_ptr < stack_ptr );

    getchar();

    return EXIT_SUCCESS;
}
