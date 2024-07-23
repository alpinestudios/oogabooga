#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <stdio.h>

void hello_world()
{
    printf("Hello, World!\n");
}

extern int add(int a, int b)
{
    return a + b;
}

#endif // EXAMPLE_H