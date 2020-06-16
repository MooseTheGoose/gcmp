#include <stdlib.h>
#include <time.h>
#include <stdio.h>

int main()
{
    srand(time(0));

    for(int i = 0; i < (1 << 23); i++)
    {
        printf("%c", rand());
    }
    return 0;
}