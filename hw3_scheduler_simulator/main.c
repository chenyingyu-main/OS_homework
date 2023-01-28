#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "include/shell.h"
#include "include/command.h"
#include "include/builtin.h"

int main(int argc, char *argv[])
{
	history_count = 0;
	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	history[i] = (char *)malloc(BUF_SIZE * sizeof(char));

	ALGORITH = argv[1];
	shell();

	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	free(history[i]);

	return 0;
}
