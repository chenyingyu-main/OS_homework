#ifndef RESOURCE_H
#define RESOURCE_H
#include<stdbool.h>

void get_resources(int, int *);
void release_resources(int, int *);

extern bool resource_taken[8];

#endif
