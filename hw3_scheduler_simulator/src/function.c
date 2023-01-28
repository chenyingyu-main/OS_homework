#include <stdio.h>
#include <stdlib.h>
#include "../include/task.h"
#include "../include/resource.h"
#include "../include/function.h"

void test_exit()
{
    task_exit();
    while (1);
}

void test_sleep()
{
    task_sleep(20);
    task_exit();
    while (1);
}

void test_resource1()
{
    int resource_list[3] = {1, 3, 7};
    get_resources(3, resource_list);
    task_sleep(5);
    release_resources(3, resource_list);
    task_exit();
    while (1);
}

void test_resource2()
{
    int resource_list[2] = {0, 3};
    get_resources(2, resource_list);
    release_resources(2, resource_list);
    task_exit();
    while (1);
}

void idle()
{
    while (1);
}

void task1()
{
    int len = 12000;
    int *arr = (int *)malloc(len * sizeof(int));
    for (int i = 0; i < len; ++i)
        arr[i] = rand();
        
    for (int i = 0; i < len; ++i) {
        for (int j = i; j < len; ++j) {
            if (arr[i] > arr[j]) {
                arr[i] ^= arr[j];
                arr[j] ^= arr[i];
                arr[i] ^= arr[j];
            }
        }
    }

    free(arr);
    task_exit();
    while (1);
}

void task2()
{
    int size = 512;
    int *matrix[size], *result[size];
    for (int i = 0; i < size; ++i) {
        matrix[i] = (int *)malloc(size * sizeof(int));
        result[i] = (int *)malloc(size * sizeof(int));
    }

    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            matrix[i][j] = rand() % 100;
    
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            result[row][col] = 0;
            for (int i = 0; i < size; ++i) {
                result[row][col] += matrix[row][i] * matrix[i][col];
            }
        }
    }

    for (int i = 0; i < size; ++i) {
        free(matrix[i]);
        free(result[i]);
    }
    task_exit();
    while (1);
}

void task3()
{
    int find_target = 65409;

    int len = 10000000;
    int *list = (int *)malloc(len * sizeof(int));
    for (int i = 0; i < len; ++i)
        list[i] = rand() % find_target;
        
    
    for (int i = 0; i < len; ++i)
        if (list[i] == find_target)
            break;

    free(list);
    task_exit();
    while (1);
}

void task4()
{
    int resource_list[3] = {0, 1, 2};
    get_resources(3, resource_list);
    task_sleep(70);     // using resources
    release_resources(3, resource_list);
    task_exit();
    while (1);
}

void task5()
{
    int resource_list_1[2] = {1, 4};
    get_resources(2, resource_list_1);
    task_sleep(20);     // using resources

    int resource_list_2[1] = {5};
    get_resources(1, resource_list_2);
    task_sleep(40);     // using resources

    int resource_list_all[3] = {1, 4, 5};
    release_resources(3, resource_list_all);
    task_exit();
    while (1);
}

void task6()
{
    int resource_list[2] = {2, 4};
    get_resources(2, resource_list);
    task_sleep(60);     // using resources
    release_resources(2, resource_list);
    task_exit();
    while (1);
}

void task7()
{
    int resource_list[3] = {1, 3, 6};
    get_resources(3, resource_list);
    task_sleep(80);     // using resources
    release_resources(3, resource_list);
    task_exit();
    while (1);
}

void task8()
{
    int resource_list[3] = {0, 4, 7};
    get_resources(3, resource_list);
    task_sleep(40);     // using resources
    release_resources(3, resource_list);
    task_exit();
    while (1);
}

void task9()
{
    int resource_list_1[1] = {5};
    get_resources(1, resource_list_1);
    task_sleep(80);     // using resources

    int resource_list_2[2] = {4, 6};
    get_resources(2, resource_list_2);
    task_sleep(40);     // using resources

    int resource_list_all[3] = {4, 5, 6};
    release_resources(3, resource_list_all);
    task_exit();
    while (1);
}
