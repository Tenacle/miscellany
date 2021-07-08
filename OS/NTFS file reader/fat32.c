#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fat32.h"
//#include <assert.h>
//#include <stdlib.h>

int main(void)
{
	printf("%s", "Let's read this drive\n");
	fat32BS * fat32_struct = (fat32BS *) malloc(sizeof(fat32BS));
	printf("Size of fat32_struct: %d", (int) sizeof(*fat32_struct));

	int filedesc = open("a4image", O_WRONLY);

}
